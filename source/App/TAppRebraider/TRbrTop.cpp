#include "TRbrTop.h"


/**
 * Default constructor for debraider class
 */
TRbrTop::TRbrTop() {
  TEncSlice& sliceEncoder = *getSliceEncoder();
  TEncCu&    cuEncoder    = *getCuEncoder();

  setDeltaQpRD(0);

  sliceEncoder.init(this);
  cuEncoder.init(this);
  cuEncoder.setSliceEncoder(&sliceEncoder);
}


/**
 * Transcode a NAL unit without decoding
 */
Void TRbrTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu) {
  outputNalu = inputNalu;
}


/**
 * Transcode a decoded VPS NAL unit
 */
Void TRbrTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComVPS& vps) {
  setVPS(&vps);
  xEncodeVPS(vps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded SPS NAL unit
 */
Void TRbrTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSPS& sps) {
  *getSpsMap()->allocatePS(sps.getSPSId()) = sps;
  xEncodeSPS(sps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded PPS NAL unit
 */
Void TRbrTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComPPS& pps) {
  TComPPS& encPps = *getPpsMap()->allocatePS(pps.getPPSId());
  encPps = pps;
  xEncodePPS(encPps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded slice NAL unit
 */
Void TRbrTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSlice& slice) {
  // Find or create an encoder picture for the current slice
  TComPic& encPic = xGetEncPicBySlice(slice);

  // Copy the decoded slice to encoder
  TComSlice& encSlice = xCopySliceToPic(slice, encPic);

  // Set the current encoder slice
  encPic.setCurrSliceIdx(encPic.getNumAllocatedSlice() - 1);

  // Set the slice scaling list
  xResetScalingList(encSlice);

  // Set the reference pic list
  if (!encSlice.getDependentSliceSegmentFlag()) {
    encSlice.setRefPicList(*getListPic(), true);
  }

  // Requantize the slice to achieve a target rate
  //xRequantizeSlice(encSlice);

  // Entropy code the requantized slice into the output nal unit bitstream
  xEncodeSlice(encSlice, outputNalu.m_Bitstream);
}


/**
 * Encode a VPS and write to bitstream
 */
Void TRbrTop::xEncodeVPS(const TComVPS& vps, TComOutputBitstream& bitstream) {
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodeVPS(&vps);
}


/**
 * Encode a SPS and write to bitstream
 */
Void TRbrTop::xEncodeSPS(const TComSPS& sps, TComOutputBitstream& bitstream) {
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodeSPS(&sps);

  // TODO: If multiple SPS nal units are encountered in the source bitstream,
  //   the slice encoder will be intialized again (possible activation
  //   issues?). Is this a problem?
  getSliceEncoder()->create(
    sps.getPicWidthInLumaSamples(),
    sps.getPicHeightInLumaSamples(),
    sps.getChromaFormatIdc(),
    sps.getMaxCUHeight(),
    sps.getMaxCUWidth(),
    sps.getMaxTotalCUDepth()
  );

  // TODO: Same issue here with the cu buffers being initialized multiple times
  //   and possibly causing problems
  xMakeCuBuffers(sps);

  // TODO: Same issue with creating loop filter
  getLoopFilter()->create(sps.getMaxTotalCUDepth());

  // TODO: And with prediction search
  getPredSearch()->initTempBuff(sps.getChromaFormatIdc());

  // TODO: And transquant
  getTrQuant()->init(sps.getMaxTrSize());
}


/**
 * Encode a PPS and write to bitstream
 */
Void TRbrTop::xEncodePPS(const TComPPS& pps, TComOutputBitstream& bitstream) {
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodePPS(&pps);

  // Initialize the sao filter for this pps
  TComSPS& sps = *getSpsMap()->getPS(pps.getSPSId());
  getSAO()->create(
    sps.getPicWidthInLumaSamples(),
    sps.getPicHeightInLumaSamples(),
    sps.getChromaFormatIdc(),
    sps.getMaxCUWidth(),
    sps.getMaxCUHeight(),
    sps.getMaxTotalCUDepth(),
    pps.getPpsRangeExtension().getLog2SaoOffsetScale(CHANNEL_TYPE_LUMA),
    pps.getPpsRangeExtension().getLog2SaoOffsetScale(CHANNEL_TYPE_CHROMA)
  );
}


/**
 * Local, static helper function to calculate the number of encoding substreams
 *   to allocate for encoding a given slice
 */
static inline Int numEncSubstreams(const TComSlice& slice) {
  const TComPPS& pps = *slice.getPPS();
  const Int numSubstreamCols = pps.getNumTileColumnsMinus1() + 1;
  const Int numSubstreamRows =
    pps.getEntropyCodingSyncEnabledFlag() ?
    slice.getPic()->getFrameHeightInCtus() :
    pps.getNumTileRowsMinus1() + 1;
  return numSubstreamRows * numSubstreamCols;
}


/**
 * Encode a slice and write to bitstream
 */
Void TRbrTop::xEncodeSlice(TComSlice& slice, TComOutputBitstream& bitstream) {
  // The provided slice must have an associated TComPic
  TComPic& pic = *slice.getPic();

  // Get encoder references
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  TEncSlice&   sliceEncoder   = *getSliceEncoder();

  // Prepare pic to encode slice
  pic.setTLayer(slice.getTLayer());

  // Allocate temporary encoding substreams
  std::vector<TComOutputBitstream> substreams(numEncSubstreams(slice));

  // Set current slice
  const UInt sliceIndex = slice.getSliceIdx();
  sliceEncoder.setSliceIdx(sliceIndex);
  pic.setCurrSliceIdx(sliceIndex);

  // Write slice head to output bitstream
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.resetEntropy(&slice);
  entropyEncoder.setBitstream(&bitstream);
  slice.setEncCABACTableIdx(sliceEncoder.getEncCABACTableIdx());
  entropyEncoder.encodeSliceHeader(&slice);

  // Encode slice to temporary substreams
  UInt numBinsCoded = 0;
  slice.setFinalized(true);
  slice.clearSubstreamSizes();
  sliceEncoder.encodeSlice(&pic, substreams.data(), numBinsCoded);

  // Write WPP entry points to output bitstream
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodeTilesWPPEntryPoint(&slice);

  // Concatenate substreams
  TComOutputBitstream concatenatedStream;
  const Int numZeroSubstreamsAtStartOfSlice = pic.getSubstreamForCtuAddr(slice.getSliceSegmentCurStartCtuTsAddr(), false, &slice);
  const Int numSubstreamsToCode             = slice.getNumberOfSubstreamSizes() + 1;
  for (UInt substreamIndex = 0 ; substreamIndex < numSubstreamsToCode; substreamIndex++) {
    TComOutputBitstream& substream =
      *(substreams.data() + substreamIndex + numZeroSubstreamsAtStartOfSlice);
    concatenatedStream.addSubstream(&substream);
    substream.clear();
  }

  // Append concatenated substream to the output bitstream
  bitstream.writeByteAlignment();
  if (concatenatedStream.getNumberOfWrittenBits() > 0) {
    bitstream.addSubstream(&concatenatedStream);
  }
  entropyEncoder.setBitstream(&bitstream);
  concatenatedStream.clear();

  // Apply filters, compress motion
  if (slice.getSliceCurEndCtuTsAddr() == pic.getNumberOfCtusInFrame()) {
    xFinishPicture(pic);
  }
}


/**
 * Apply filters and compress motion for a reconstructed picture
 */
Void TRbrTop::xFinishPicture(TComPic& pic) {
  // Get pps, sps
  const TComPPS& pps = pic.getPicSym()->getPPS();
  const TComSPS& sps = pic.getPicSym()->getSPS();

  // Apply deblocking filter
  TComLoopFilter& loopFilterProcessor = *getLoopFilter();
  Bool shouldDeblockOverTiles = pps.getLoopFilterAcrossTilesEnabledFlag();
  loopFilterProcessor.setCfg(shouldDeblockOverTiles);
  loopFilterProcessor.loopFilterPic(&pic);

  // Apply sao filter
  if (sps.getUseSAO()) {
    TComSampleAdaptiveOffset& saoProcessor = *getSAO();
    SAOBlkParam* saoBlockParam = pic.getPicSym()->getSAOBlkParam();
    saoProcessor.reconstructBlkSAOParams(&pic, saoBlockParam);
    saoProcessor.SAOProcess(&pic);
    saoProcessor.PCMLFDisableProcess(&pic);
  }

  // Compress motion
  pic.compressMotion();

  // Mark the picture as fully reconstructed
  //pic.setOutputMark(pic.getSlice(0)->getPicOutputFlag());
  pic.setReconMark(true);
}


/**
 * Resolve a TComSlice into its corresponding encoder TComPic
 */
TComPic& TRbrTop::xGetEncPicBySlice(const TComSlice& slice) {
  // Search for existing TComPic with the desired POC
  TComPic* pPic = xGetEncPicByPoc(slice.getPOC());
  if (pPic != nullptr) {
    return *pPic;
  }

  // Get an unused entry in the buffer for the new TComPic
  TComPic*& pPicEntry = xGetUnusedEntry();
  if (pPicEntry == nullptr) {
    pPicEntry = new TComPic;
  }

  // Set up the new TComPic
  xCopyDecPic(*slice.getPic(), *pPicEntry);

  // Return the new TComPic
  return *pPicEntry;
}


/**
 * Copy a TComSlice to a TComPic, returning a reference to the new TComSlice
 */
TComSlice& TRbrTop::xCopySliceToPic(const TComSlice& srcSlice, TComPic& dstPic) {
  // Create destination slice
  dstPic.allocateNewSlice();
  TComSlice& dstSlice = *dstPic.getSlice(dstPic.getNumAllocatedSlice() - 1);

  // Copy slice data to destination slice
  dstSlice = srcSlice;
  dstSlice.setPic(&dstPic);
  dstSlice.setRPS(srcSlice.getRPS());
  if (dstPic.getCurrSliceIdx() == 0) {
    dstSlice.applyReferencePictureSet(*getListPic(), dstSlice.getRPS());
  }

  // Use encoder picture references instead of decoder references
  for (Int iList = 0; iList < NUM_REF_PIC_LIST_01; iList++) {
    for (Int iPic = 0; iPic < MAX_NUM_REF; iPic++) {
      RefPicList iRefPicList = static_cast<RefPicList>(iList);
      const TComPic* decRefPic = srcSlice.getRefPic(iRefPicList, iPic);
      if (decRefPic != nullptr) {
        TComPic* encRefPic = xGetEncPicByPoc(decRefPic->getPOC());
        assert(encRefPic != nullptr); // ensure reference lists are synced
        dstSlice.setRefPic(encRefPic, iRefPicList, iPic);
      }
    }
  }

  // Copy TComDataCUs corresponding to the slice
  {
    const TComPic& srcPic         = *srcSlice.getPic();
    const UInt     startCtuTsAddr = srcSlice.getSliceSegmentCurStartCtuTsAddr();
    const UInt     endCtuTsAddr   = srcSlice.getSliceSegmentCurEndCtuTsAddr();
    for (UInt ctuTsAddr = startCtuTsAddr; ctuTsAddr < endCtuTsAddr; ctuTsAddr++) {
      const TComPicSym& srcPicSym = *srcPic.getPicSym();
      const UInt        ctuRsAddr = srcPicSym.getCtuTsToRsAddrMap(ctuTsAddr);
      const TComDataCU& srcCtu    = *srcPicSym.getCtu(ctuRsAddr);
            TComDataCU& dstCtu    = *dstPic.getPicSym()->getCtu(ctuRsAddr);
      dstCtu = srcCtu;
      dstCtu.setSlice(&dstSlice);
    }
  }

  return dstSlice;
}


/**
 * Get an unused entry from the picture buffer
 */
TComPic*& TRbrTop::xGetUnusedEntry() {
  TComList<TComPic*>& cpb = *getListPic();

  // Sort the buffer
  TComSlice::sortPicList(cpb);

  // Linearly search the buffer for an existing unused entry
  for (auto it = cpb.begin(); it != cpb.end(); it++) {
    TComPic*& pPic = *it;

    Bool isUnusedEntry = 
      pPic != nullptr &&
      (!pPic->getReconMark() || !pPic->getSlice(0)->isReferenced());

    if (isUnusedEntry) {
      pPic->setReconMark(false);
      pPic->getPicYuvRec()->setBorderExtension(false);
      return pPic;
    }
  }

  // All entries are used, so make a new entry at the end of the list
  cpb.push_back(nullptr);
  return cpb.back();
}


/**
 * Find an existing const TComPic by POC
 */
TComPic* TRbrTop::xGetEncPicByPoc(Int poc) {
  TComList<TComPic*>& cpb = *getListPic();

  for (auto it = cpb.begin(); it != cpb.end(); it++) {
    TComPic* pPic = *it;
    if (pPic != nullptr && pPic->getPOC() == poc) {
      return pPic;
    }
  }

  return nullptr;
}


/**
 * Set up memory buffers for all cu sizes
 */
Void TRbrTop::xMakeCuBuffers(const TComSPS& sps) {
  UInt maxDepth             = sps.getMaxTotalCUDepth();
  UInt maxWidth             = sps.getMaxCUWidth();
  UInt maxHeight            = sps.getMaxCUHeight();
  ChromaFormat chromaFormat = sps.getChromaFormatIdc();

  // TODO: The following resize operation may leak memory if the stored
  //   objects are not destructor-safe
  m_originalBuffer.resize(maxDepth);
  m_predictionBuffer.resize(maxDepth);
  m_residualBuffer.resize(maxDepth);
  m_reconstructionBuffer.resize(maxDepth);
  m_cuBuffer.resize(maxDepth);

  for (UInt d = 0; d < maxDepth; d++) {
    UInt uiNumPartitions = 1 << ((maxDepth - d) << 1);
    UInt uiWidth  = maxWidth  >> d;
    UInt uiHeight = maxHeight >> d;
    
    m_originalBuffer[d].create(uiWidth, uiHeight, chromaFormat);
    m_predictionBuffer[d].create(uiWidth, uiHeight, chromaFormat);
    m_residualBuffer[d].create(uiWidth, uiHeight, chromaFormat);
    m_reconstructionBuffer[d].create(uiWidth, uiHeight, chromaFormat);

    m_cuBuffer[d].create(
      chromaFormat,
      uiNumPartitions,
      uiWidth,
      uiHeight,
      true,
      maxWidth >> maxDepth
    );
  }
}


/**
 * Set up an encoder TComPic by copying relevant configuration from a
 *   corresponding decoded TComPic
 */
Void TRbrTop::xCopyDecPic(const TComPic& srcPic, TComPic& dstPic) {
  // Get the encoder TComPPS and TComSPS for this TComPic
  TComPPS& pps = *getPpsMap()->getPS(srcPic.getPicSym()->getPPS().getPPSId());
  TComSPS& sps = *getSpsMap()->getPS(srcPic.getPicSym()->getSPS().getSPSId());

  // Reinitialize the encoder TComPic
  dstPic.setPicYuvOrg(nullptr);
  dstPic.setPicYuvTrueOrg(nullptr);
  dstPic.create(sps, pps, false, true);

  // Clear slice buffer
  dstPic.clearSliceBuffer();

  // Set prediction and residual planes
  dstPic.setPicYuvPred(&getSliceEncoder()->getPicYuvPred());
  dstPic.setPicYuvResi(&getSliceEncoder()->getPicYuvResi());

  // Copy relevant TComPic data
  // TODO: m_SEIs
  dstPic.setTLayer(srcPic.getTLayer());
  dstPic.setUsedByCurr(srcPic.getUsedByCurr());
  dstPic.setIsLongTerm(srcPic.getIsLongTerm());
  dstPic.setReconMark(srcPic.getReconMark());
  dstPic.setOutputMark(srcPic.getOutputMark());
  dstPic.setCurrSliceIdx(srcPic.getCurrSliceIdx());
  dstPic.setCheckLTMSBPresent(srcPic.getCheckLTMSBPresent());
  dstPic.setTopField(srcPic.isTopField());
  dstPic.setField(srcPic.isField());

  // Source picture
  TComPicYuv* reconPlane = const_cast<TComPic&>(srcPic).getPicYuvRec();
  dstPic.setPicYuvOrg(reconPlane);
  dstPic.setPicYuvTrueOrg(reconPlane);

  // Copy relevant TComPicSym data
  {
    const TComPicSym& srcSym = *srcPic.getPicSym();
          TComPicSym& dstSym = *dstPic.getPicSym();

    // m_dpbPerCtuData
    dstSym.copyDPBPerCtuDataFrom(srcSym);

    // m_saoBlkParams
    dstSym.copySAOBlkParamsFrom(srcSym);

#if ADAPTIVE_QP_SELECTION
  // m_pParentARLBuffer
    dstSym.copyParentARLBufferFrom(srcSym);
#endif
  }

  // Make each TComDataCU in the slice aware of its position within the TComPic
  for (UInt ctuRsAddr = 0; ctuRsAddr < dstPic.getPicSym()->getNumberOfCtusInFrame(); ctuRsAddr++) {
    dstPic.getCtu(ctuRsAddr)->attachToPic(dstPic, ctuRsAddr);
  }
}


/**
 * Copies pixels corresponding to a given cu directly from one TComPicYuv to
 *   another
 */
Void TRbrTop::xCopyCuPixels(TComDataCU& cu, const TComPicYuv& src, TComPicYuv& dst) {
  // Uses the origBuff cu buffer to copy pixels
  TComYuv& origBuff = m_originalBuffer[cu.getDepth(0)];

  UInt ctuRasterAddress = cu.getCtuRsAddr();
  UInt cuZOrderAddress  = cu.getZorderIdxInCtu();

  // Obtain source cu pixel values by copying yuv data from source picture
  origBuff.copyFromPicYuv(&src, ctuRasterAddress, cuZOrderAddress);

  // Copy these pixels directly back into the reconstructed picture
  origBuff.copyToPicYuv(&dst, ctuRasterAddress, cuZOrderAddress);
}


/**
 * Resets the scaling list on the transform quantizer object for a given slice
 */
Void TRbrTop::xResetScalingList(TComSlice& slice) {
        TComTrQuant& transQuant = *getTrQuant();
  const TComSPS&     sps        = *slice.getSPS();
  const TComPPS&     pps        = *slice.getPPS();

  if (sps.getScalingListFlag()) {
    TComScalingList scalingList;
    if (pps.getScalingListPresentFlag()) {
      scalingList = pps.getScalingList();
    } else if (sps.getScalingListPresentFlag()) {
      scalingList = sps.getScalingList();
    } else {
      scalingList.setDefaultScalingList();
    }
    transQuant.setScalingListDec(scalingList);
    transQuant.setUseScalingList(true);
  } else {
    const Int maxLog2TrDynamicRange[MAX_NUM_CHANNEL_TYPE] = {
      sps.getMaxLog2TrDynamicRange(CHANNEL_TYPE_LUMA),
      sps.getMaxLog2TrDynamicRange(CHANNEL_TYPE_CHROMA)
    };
    transQuant.setFlatScalingList(maxLog2TrDynamicRange, sps.getBitDepths());
    transQuant.setUseScalingList(false);
  }
}


/**
 * Find an existing TComPic by POC
 */
TComPic* TRbrTop::getPicByPoc(Int poc) {
  return xGetEncPicByPoc(poc);
}
