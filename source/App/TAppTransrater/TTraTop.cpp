#include "TTraTop.h"


/**
 * Default constructor for transrater class
 */
TTraTop::TTraTop() {
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
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu) {
  outputNalu = inputNalu;
}


/**
 * Transcode a decoded VPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComVPS& vps) {
  setVPS(&vps);
  xEncodeVPS(vps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded SPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSPS& sps) {
  *getSpsMap()->allocatePS(sps.getSPSId()) = sps;
  xEncodeSPS(sps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded PPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComPPS& pps) {
  TComPPS& encPps = *getPpsMap()->allocatePS(pps.getPPSId());
  encPps = pps;
  encPps.setPicInitQPMinus26(encPps.getPicInitQPMinus26() + 4);
  xEncodePPS(encPps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded slice NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSlice& slice) {
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
  xRequantizeSlice(encSlice);

  // Entropy code the requantized slice into the output nal unit bitstream
  xEncodeSlice(encSlice, outputNalu.m_Bitstream);
}


/**
 * Encode a VPS and write to bitstream
 */
Void TTraTop::xEncodeVPS(const TComVPS& vps, TComOutputBitstream& bitstream) {
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodeVPS(&vps);
}


/**
 * Encode a SPS and write to bitstream
 */
Void TTraTop::xEncodeSPS(const TComSPS& sps, TComOutputBitstream& bitstream) {
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
Void TTraTop::xEncodePPS(const TComPPS& pps, TComOutputBitstream& bitstream) {
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
Void TTraTop::xEncodeSlice(TComSlice& slice, TComOutputBitstream& bitstream) {
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
Void TTraTop::xFinishPicture(TComPic& pic) {
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
TComPic& TTraTop::xGetEncPicBySlice(const TComSlice& slice) {
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
TComSlice& TTraTop::xCopySliceToPic(const TComSlice& srcSlice, TComPic& dstPic) {
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
TComPic*& TTraTop::xGetUnusedEntry() {
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
TComPic* TTraTop::xGetEncPicByPoc(Int poc) {
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
Void TTraTop::xMakeCuBuffers(const TComSPS& sps) {
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
Void TTraTop::xCopyDecPic(const TComPic& srcPic, TComPic& dstPic) {
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
 * Requantize a given slice by altering residual qp
 */
Void TTraTop::xRequantizeSlice(TComSlice& slice) {
  slice.setSliceQp(std::min(std::max(slice.getSliceQp() + 4, 0), 51));

  TComPicSym& picSym         = *slice.getPic()->getPicSym();
  UInt        startCtuTsAddr = slice.getSliceSegmentCurStartCtuTsAddr();
  UInt        endCtuTsAddr   = slice.getSliceSegmentCurEndCtuTsAddr();
  for (UInt ctuTsAddr = startCtuTsAddr; ctuTsAddr < endCtuTsAddr; ctuTsAddr++) {
    UInt        ctuRsAddr = picSym.getCtuTsToRsAddrMap(ctuTsAddr);
    TComDataCU& ctu       = *picSym.getCtu(ctuRsAddr);
    xRequantizeCtu(ctu);
  }
}


/**
 * Recursively compress a ctu by altering residual qp
 */
Void TTraTop::xRequantizeCtu(TComDataCU& ctu, UInt cuPartAddr, UInt cuDepth) {
        TComPic& pic = *ctu.getPic();
  const TComSPS& sps = *ctu.getSlice()->getSPS();

  UInt cuLeftBoundary = ctu.getCUPelX() +
    g_auiRasterToPelX[g_auiZscanToRaster[cuPartAddr]];
  UInt cuRightBoundary =
    cuLeftBoundary + (sps.getMaxCUWidth() >> cuDepth) - 1;
  UInt cuTopBoundary = ctu.getCUPelY() +
    g_auiRasterToPelY[g_auiZscanToRaster[cuPartAddr]];
  UInt cuBottomBoundary =
    cuTopBoundary + (sps.getMaxCUHeight() >> cuDepth) - 1;

  Bool isCuPartiallyOutsidePicture = (
    cuRightBoundary  >= sps.getPicWidthInLumaSamples() ||
    cuBottomBoundary >= sps.getPicHeightInLumaSamples()
  );

  Bool isCuExplicitlySplit = (
    cuDepth < ctu.getDepth(cuPartAddr) &&
    cuDepth < sps.getLog2DiffMaxMinCodingBlockSize()
  );

  if (isCuExplicitlySplit || isCuPartiallyOutsidePicture) {
    UInt childCuDepth       = cuDepth + 1;
    UInt numPartsPerChildCu = ctu.getTotalNumPart() >> (childCuDepth << 1);
    UInt childCuPartAddr    = cuPartAddr;

    for (UInt i = 0; i < 4; i++) {
      UInt childCuLeftBoundary = ctu.getCUPelX() +
        g_auiRasterToPelX[g_auiZscanToRaster[childCuPartAddr]];
      UInt childCuTopBoundary = ctu.getCUPelY() +
        g_auiRasterToPelY[g_auiZscanToRaster[childCuPartAddr]];

      Bool isChildCuPartiallyInsidePicture = (
        childCuLeftBoundary < sps.getPicWidthInLumaSamples() &&
        childCuTopBoundary  < sps.getPicHeightInLumaSamples()
      );

      if (isChildCuPartiallyInsidePicture) {
        xRequantizeCtu(ctu, childCuPartAddr, childCuDepth);
      }

      childCuPartAddr += numPartsPerChildCu;
    }

    return;
  }

  // Set up cu data structure for transcoding
  TComDataCU& cu = m_cuBuffer[cuDepth];
  cu.copySubCU(&ctu, cuPartAddr);

  // Get original, prediction, residual, and reconstruction yuv buffers
  TComYuv& origBuff = m_originalBuffer[cuDepth];
  TComYuv& predBuff = m_predictionBuffer[cuDepth];
  TComYuv& resiBuff = m_residualBuffer[cuDepth];
  TComYuv& recoBuff = m_reconstructionBuffer[cuDepth];

  // Adjust cu qp
  cu.setQPSubParts(std::min(std::max(cu.getQP(0) + 4, 0), 51), 0, cuDepth);

  // Requantize cu
  if (cu.isInter(0)) {
    xRequantizeInterCu(cu);
  } else if (cu.isIntra(0)) {
    xRequantizeIntraCu(cu);
  } else {
    assert(0);
  }
}


/**
 * Requantizes an inter-predicted cu
 */
Void TTraTop::xRequantizeInterCu(TComDataCU& cu) {
  TComPic& pic     = *cu.getPic();
  UInt     cuDepth = cu.getDepth(0);
  UInt     cuWidth = cu.getWidth(0);

  // Get original, prediction, residual, and reconstruction yuv buffers
  TComYuv& origBuff = m_originalBuffer[cuDepth];
  TComYuv& predBuff = m_predictionBuffer[cuDepth];
  TComYuv& resiBuff = m_residualBuffer[cuDepth];
  TComYuv& recoBuff = m_reconstructionBuffer[cuDepth];

  // Obtain original by copying yuv data from source picture
  origBuff.copyFromPicYuv(
    pic.getPicYuvOrg(),
    cu.getCtuRsAddr(),
    cu.getZorderIdxInCtu()
  );

  // Obtain prediction by applying motion vectors
  getPredSearch()->motionCompensation(&cu, &predBuff);

  // Obtain prediction error by computing (original - prediction)
  // Note: store prediction error in residual buffer
  resiBuff.subtract(&origBuff, &predBuff, 0, cuWidth);

  // Obtain real residual by transforming and quantizing prediction error
  TComTURecurse tu(&cu, 0, cuDepth);
  for (UInt c = 0; c < pic.getNumberValidComponents(); c++) {
    xRequantizeInterTb(tu, static_cast<ComponentID>(c));
  }

  // Degrade merge mode cus into skip mode cus
  xReevaluateSkipModeDecision(cu);

  // Obtain reconstructed signal by computing (prediction + residual)
  recoBuff.addClip(
    &predBuff,
    &resiBuff,
    0,
    cuWidth,
    cu.getSlice()->getSPS()->getBitDepths()
  );

  // Copy reconstructed signal back into picture
  recoBuff.copyToPicYuv(
    pic.getPicYuvRec(),
    cu.getCtuRsAddr(),
    cu.getZorderIdxInCtu()
  );
}


/**
 * Recursively requantizes an inter-predicted transform block
 */
Void TTraTop::xRequantizeInterTb(TComTURecurse& tu, ComponentID component) {
  TComTrQuant& transQuant = *getTrQuant();
  TComDataCU&  cu         = *tu.getCU();
  UChar        cuDepth    = cu.getDepth(0);
  TComYuv&     origBuff   = m_originalBuffer[cuDepth];
  TComYuv&     predBuff   = m_predictionBuffer[cuDepth];
  TComYuv&     resiBuff   = m_residualBuffer[cuDepth];

  // Recursion base case: transform block spans zero pixels
  if (!tu.ProcessComponentSection(component)) {
    return;
  }

  UInt tuPartIndex = tu.GetAbsPartIdxTU();
  UInt uiTrMode = tu.GetTransformDepthRel();

  Bool areDecodedCoefficientsAllZero =
    cu.getCbf(tuPartIndex, component, uiTrMode) == 0;

  // TODO: Commented this out because maybe we can't ever skip requant
  /*Bool isCrossComponentPredictionEnabled =
    cu.getSlice()
      ->getPPS()
      ->getPpsRangeExtension()
      .getCrossComponentPredictionEnabledFlag();

  Bool canSkipTransQuant =
    areDecodedCoefficientsAllZero &&
    (isLuma(component) || !isCrossComponentPredictionEnabled);

  if (canSkipTransQuant) {
    return;
  }*/

  // Traverse residual quadtree via recursion
  if (uiTrMode != cu.getTransformIdx(tuPartIndex)) {
    Bool hasNonzeroCoefficients = false;
    TComTURecurse tuChild(tu, false);

    do {
      xRequantizeInterTb(tuChild, component);

      hasNonzeroCoefficients =
        hasNonzeroCoefficients ||
        xHasNonzeroCoefficients(tuChild, component);
    } while (tuChild.nextSection(tu));

    if (hasNonzeroCoefficients) {
      xMarkTbCbfTrue(tu, component);
    }

    return;
  }

  const TComRectangle& tuRect        = tu.getRect(component);
  const UInt           tuWidth       = tuRect.width;
  const UInt           tuHeight      = tuRect.height;
  const Int            cuStride      = resiBuff.getStride(component);
  const UInt           tuPelOffset   = tuRect.x0 + cuStride * tuRect.y0;
        Pel*           pResidual     = resiBuff.getAddr(component) + tuPelOffset;

  // If the source encoding had no residual coefficients, then the recoded block
  //   also should have no residual coefficients
  if (areDecodedCoefficientsAllZero) {
    Pel* rowResi  = pResidual;
    UInt rowWidth = tuWidth * sizeof(Pel);

    for (UInt y = 0; y < tuHeight; y++) {
      memset(rowResi, 0, rowWidth);
      rowResi += cuStride;
    }

    xClearTbCbf(tu, component);

  // Otherwise, requantize residual coefficients
  } else {
    xApplyResidualTransQuant(tu, component, resiBuff);
  }
  
  // Cross-component prediction
  // TODO: Check the location of this block
  Bool isCrossComponentPredictionUsed =
    cu.getCrossComponentPredictionAlpha(tuPartIndex, component) != 0;

  Bool areAllDecodedLumaCoefficientsZero =
    cu.getCbf(tuPartIndex, COMPONENT_Y, uiTrMode) == 0;

  Bool shouldApplyCrossComponentPrediction = (
    isChroma(component) &&
    isCrossComponentPredictionUsed &&
    !areAllDecodedLumaCoefficientsZero
   );

  if (shouldApplyCrossComponentPrediction) {
    // TODO: Enable cross component prediction
    assert(0);

    const Int  strideLuma    = resiBuff.getStride(COMPONENT_Y);
    const Pel* pResidualLuma = resiBuff.getAddr(COMPONENT_Y) + tuPelOffset;

    transQuant.crossComponentPrediction(
      tu,            //       TComTU&       rTu,
      component,     // const ComponentID   compID,
      pResidualLuma, // const Pel*          piResiL,
      pResidual,     // const Pel*          piResiC,
      pResidual,     //       Pel*          piResiT,
      tuRect.width,  // const Int           width,
      tuRect.height, // const Int           height,
      strideLuma,    // const Int           strideL,
      cuStride,      // const Int           strideC,
      cuStride,      // const Int           strideT,
      true           // const Bool          reverse
    );
  }
}


/**
 * Copies pixels corresponding to a given cu directly from one TComPicYuv to
 *   another
 */
Void TTraTop::xCopyCuPixels(TComDataCU& cu, const TComPicYuv& src, TComPicYuv& dst) {
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
 * Requantizes an intra-predicted cu
 */
Void TTraTop::xRequantizeIntraCu(TComDataCU& cu) {
  TComPic& pic       = *cu.getPic();
  UInt     cuDepth   = cu.getDepth(0);
  UInt     cuWidth   = cu.getWidth(0);
  Bool     hasChroma = pic.getChromaFormat() != CHROMA_400;
  
  // Guess this method can't handle IPCM CUs
  if (cu.getIPCMFlag(0)) {
    assert(0);
    // TODO
    //xReconPCM( pcCU, uiDepth );
    //return;
  }

  // Obtain original signal by copying yuv data from source picture
  TComYuv& origBuff = m_originalBuffer[cuDepth];
  origBuff.copyFromPicYuv(
    pic.getPicYuvOrg(),
    cu.getCtuRsAddr(),
    cu.getZorderIdxInCtu()
  );

  // Requantize luma
  {
    Bool lumaHasNonzeroCoefficients = false;
    Bool allowSplit = (cu.getPartitionSize(0) != SIZE_2Nx2N);

    TComTURecurse parentTu(&cu, 0);
    TComTURecurse tu(
      parentTu,
      false,
      allowSplit ? TComTU::QUAD_SPLIT : TComTU::DONT_SPLIT
    );

    do {
      xRequantizeIntraTu(tu, ChannelType::CHANNEL_TYPE_LUMA);

      lumaHasNonzeroCoefficients =
        lumaHasNonzeroCoefficients ||
        xHasNonzeroCoefficients(tu, ComponentID::COMPONENT_Y);
    } while (tu.nextSection(parentTu));

    if (lumaHasNonzeroCoefficients) {
      xMarkTbCbfTrue(parentTu, ComponentID::COMPONENT_Y);
    }
  }

  // Requantize chroma
  if (hasChroma) {
    Bool hasNonzeroCoeffs[ComponentID::MAX_NUM_COMPONENT] =
      {false, false, false};
    Bool allowSplit = (
      cu.getPartitionSize(0) != SIZE_2Nx2N &&
      enable4ChromaPUsInIntraNxNCU(pic.getChromaFormat())
    );

    TComTURecurse parentTu(&cu, 0);
    TComTURecurse tu(
      parentTu,
      false,
      allowSplit ? TComTU::QUAD_SPLIT : TComTU::DONT_SPLIT
    );

    do {
      xRequantizeIntraTu(tu, ChannelType::CHANNEL_TYPE_CHROMA);

      hasNonzeroCoeffs[ComponentID::COMPONENT_Cr] =
        hasNonzeroCoeffs[ComponentID::COMPONENT_Cr] ||
        xHasNonzeroCoefficients(tu, ComponentID::COMPONENT_Cr);

      hasNonzeroCoeffs[ComponentID::COMPONENT_Cb] =
        hasNonzeroCoeffs[ComponentID::COMPONENT_Cb] ||
        xHasNonzeroCoefficients(tu, ComponentID::COMPONENT_Cb);
    } while (tu.nextSection(parentTu));

    if (hasNonzeroCoeffs[ComponentID::COMPONENT_Cr]) {
      xMarkTbCbfTrue(parentTu, ComponentID::COMPONENT_Cr);
    }

    if (hasNonzeroCoeffs[ComponentID::COMPONENT_Cb]) {
      xMarkTbCbfTrue(parentTu, ComponentID::COMPONENT_Cb);
    }
  }
}


/**
 * Requantizes an intra-predicted tu channel type
 */
Void TTraTop::xRequantizeIntraTu(TComTURecurse& tu, ChannelType channelType) {
  TComDataCU& cu            = *tu.getCU();
  UInt        uiTrDepth     = tu.GetTransformDepthRel();
  UInt        uiAbsPartIdx  = tu.GetAbsPartIdxTU();
  UInt        uiTrMode      = cu.getTransformIdx(uiAbsPartIdx);
  UInt        channelIsLuma = channelType == ChannelType::CHANNEL_TYPE_LUMA;

  // Recurse through tus
  if (uiTrMode != uiTrDepth) {
    Bool hasNonzeroCoeffs[ComponentID::MAX_NUM_COMPONENT] =
      {false, false, false};
    TComTURecurse tuChild(tu, false);

    do {
      xRequantizeIntraTu(tuChild, channelType);

      if (channelIsLuma) {
        hasNonzeroCoeffs[ComponentID::COMPONENT_Y] =
          hasNonzeroCoeffs[ComponentID::COMPONENT_Y] ||
          xHasNonzeroCoefficients(tuChild, ComponentID::COMPONENT_Y);
      } else {
        hasNonzeroCoeffs[ComponentID::COMPONENT_Cr] =
          hasNonzeroCoeffs[ComponentID::COMPONENT_Cr] ||
          xHasNonzeroCoefficients(tuChild, ComponentID::COMPONENT_Cr);

        hasNonzeroCoeffs[ComponentID::COMPONENT_Cb] =
          hasNonzeroCoeffs[ComponentID::COMPONENT_Cb] ||
          xHasNonzeroCoefficients(tuChild, ComponentID::COMPONENT_Cb);
      }
    } while (tuChild.nextSection(tu));

    for (UInt c = 0; c < ComponentID::MAX_NUM_COMPONENT; c++) {
      if (hasNonzeroCoeffs[c]) {
        xMarkTbCbfTrue(tu, static_cast<ComponentID>(c));
      }
    }

    return;
  }

  if (channelIsLuma) {
    xRequantizeIntraTb(tu, COMPONENT_Y);
  } else {
    const UInt numValidComp = getNumberValidComponents(tu.GetChromaFormat());
    for (UInt c = COMPONENT_Cb; c < numValidComp; c++) {
      xRequantizeIntraTb(tu, static_cast<ComponentID>(c));
    }
  }
}


/**
 * Recursively requantizes an intra-predicted transform block
 */
Void TTraTop::xRequantizeIntraTb(TComTURecurse& tu, ComponentID component) {
  // Recursion base case: transform block spans zero pixels
  if (!tu.ProcessComponentSection(component)) {
    return;
  }

        TComDataCU&    cu       = *tu.getCU();
  const UInt           cuDepth  = cu.getDepth(0);
  const TComRectangle& tuRect   = tu.getRect(component);
  const UInt           tuWidth  = tuRect.width;
  const UInt           tuHeight = tuRect.height;

  // Recursively handle vertical split
  if (tuWidth != tuHeight) {
    Bool hasNonzeroCoefficients = false;
    TComTURecurse childTu(tu, false, TComTU::VERTICAL_SPLIT, true, component);

    do {
      xRequantizeIntraTb(childTu, component);

      hasNonzeroCoefficients =
        hasNonzeroCoefficients ||
        xHasNonzeroCoefficients(childTu, component);
    } while (childTu.nextSection(tu));

    if (hasNonzeroCoefficients) {
      xMarkTbCbfTrue(tu, component);
    }

    return;
  }

  // Get original, prediction, residual, and reconstruction yuv buffers
  TComYuv& origBuff = m_originalBuffer[cuDepth];
  TComYuv& predBuff = m_predictionBuffer[cuDepth];
  TComYuv& resiBuff = m_residualBuffer[cuDepth];
  TComYuv& recoBuff = m_reconstructionBuffer[cuDepth];
  
  const UInt tuPartIndex   = tu.GetAbsPartIdxTU();
  const UInt cuStride      = recoBuff.getStride(component);
  const UInt tuPelOffset   = tuRect.x0 + cuStride * tuRect.y0;

  // Get direct pointers to the buffered pixel values
  Pel* pOriginal    = origBuff.getAddr(component, tuPartIndex);
  Pel* pPrediction  = predBuff.getAddr(component, tuPartIndex);
  Pel* pResidual    = resiBuff.getAddr(component, tuPartIndex);
  Pel* pReconstruct = recoBuff.getAddr(component, tuPartIndex);

  // Construct prediction
  xConstructIntraPrediction(tu, component, predBuff);

  // Perform cross component prediction
  // TODO: Double check the location of cross component prediction
  Bool shouldApplyCrossComponentPrediction = (
    isChroma(component) &&
    cu.getCrossComponentPredictionAlpha(tuPartIndex, component) != 0
  );

  if (shouldApplyCrossComponentPrediction) {
    // TODO: Enable cross component prediction
    assert(0);

    const Int  strideLuma    = resiBuff.getStride(COMPONENT_Y);
    const Pel* pResidualLuma = resiBuff.getAddr(COMPONENT_Y) + tuPelOffset;

    TComTrQuant& transQuant = *getTrQuant();
    transQuant.crossComponentPrediction(
      tu,            //       TComTU&       rTu,
      component,     // const ComponentID   compID,
      pResidualLuma, // const Pel*          piResiL,
      pResidual,     // const Pel*          piResiC,
      pResidual,     //       Pel*          piResiT,
      tuRect.width,  // const Int           width,
      tuRect.height, // const Int           height,
      strideLuma,    // const Int           strideL,
      cuStride,      // const Int           strideC,
      cuStride,      // const Int           strideT,
      true           // const Bool          reverse
    );
  }

  Bool areDecodedCoefficientsAllZero =
    cu.getCbf(tuPartIndex, component, tu.GetTransformDepthRel()) == 0;

  // If the source encoding had no residual coefficients, then the recoded block
  //   also should have no residual coefficients
  if (areDecodedCoefficientsAllZero) {
    Pel* rowResi  = pResidual;
    UInt rowWidth = tuWidth * sizeof(Pel);

    for (UInt y = 0; y < tuHeight; y++) {
      memset(rowResi, 0, rowWidth);
      rowResi += cuStride;
    }

    xClearTbCbf(tu, component);

  // Otherwise, requantize residual coefficients
  } else {
    // Calculate the prediction error (store in residual buffer)
    {
      Pel* rowOrig = pOriginal;
      Pel* rowPred = pPrediction;
      Pel* rowResi = pResidual;

      for (UInt y = 0; y < tuHeight; y++) {
        for (UInt x = 0; x < tuWidth; x++) {
          rowResi[x] = rowOrig[x] - rowPred[x];
        }
        rowOrig += cuStride;
        rowPred += cuStride;
        rowResi += cuStride;
      }
    }

    // Transform and quantize prediction error
    xApplyResidualTransQuant(tu, component, resiBuff);
  }

  // Calculate reconstruction (prediction + residual) and copy back to picture
  {
    TComPic& pic     = *cu.getPic();
    Pel*     pPred   = pPrediction;
    Pel*     pResi   = pResidual;
    Pel*     pReco   = pReconstruct;
    Pel*     pRecPic = pic.getPicYuvRec()->getAddr(
      component,
      cu.getCtuRsAddr(),
      cu.getZorderIdxInCtu() + tuPartIndex
    );
  
    const TComSPS& sps          = *cu.getSlice()->getSPS();
    const Int      clipBitDepth = sps.getBitDepth(toChannelType(component));
    const UInt     recPicStride = pic.getPicYuvRec()->getStride(component);

    for (UInt y = 0; y < tuHeight; y++) {
      for (UInt x = 0; x < tuWidth; x++) {
        pReco  [x] = ClipBD(pPred[x] + pResi[x], clipBitDepth);
        pRecPic[x] = pReco[x];
      }
      pPred   += cuStride;
      pResi   += cuStride;
      pReco   += cuStride;
      pRecPic += recPicStride;
    }
  }
}


/**
 * Calculates the intra prediction direction for a given transform block
 */
UInt TTraTop::xGetTbIntraDirection(TComTURecurse& tu, ComponentID component) const {
  const TComDataCU&  cu            = *tu.getCU();
  const TComSPS&     sps           = *cu.getSlice()->getSPS();
  const Bool         isChromaBlock = isChroma(component);
  const UInt         tuPartAddress = tu.GetAbsPartIdxTU();
  const ChromaFormat chromaFormat  = tu.GetChromaFormat();

  const UInt predMode =
    cu.getIntraDir(toChannelType(component), tuPartAddress);

  const UInt partsPerMinCU = 1 <<
    (2 * (sps.getMaxTotalCUDepth() - sps.getLog2DiffMaxMinCodingBlockSize()));

  const UInt intraDirIndex = getChromasCorrespondingPULumaIdx(
    tuPartAddress,
    chromaFormat,
    partsPerMinCU
  );

  const UInt codedMode = (predMode == DM_CHROMA_IDX && isChromaBlock) ?
    cu.getIntraDir(CHANNEL_TYPE_LUMA, intraDirIndex) : predMode;

  if (chromaFormat == CHROMA_422 && isChromaBlock) {
    return g_chroma422IntraAngleMappingTable[codedMode];
  } else {
    return codedMode;
  }
}


/**
 * Determines if the intra prediction source samples for a given tu block
 *   should be filtered before used in intra prediction
 */
Bool TTraTop::xShouldFilterIntraReferenceSamples(TComTURecurse& tu, ComponentID component) const {
  const TComDataCU&  cu           = *tu.getCU();
  const TComSPS&     sps          = *cu.getSlice()->getSPS();
  const ChromaFormat chromaFormat = tu.GetChromaFormat();
  const UInt         intraDir     = xGetTbIntraDirection(tu, component);

  const TComRectangle& tuRect   = tu.getRect(component);
  const UInt           tuWidth  = tuRect.width;
  const UInt           tuHeight = tuRect.height;

  return TComPrediction::filteringIntraReferenceSamples(
    component,
    intraDir,
    tuWidth,
    tuHeight,
    chromaFormat,
    sps.getSpsRangeExtension().getIntraSmoothingDisabledFlag()
  );
}


/**
 * Resets the scaling list on the transform quantizer object for a given slice
 */
Void TTraTop::xResetScalingList(TComSlice& slice) {
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
 * Marks the cbf for a given transform block to indicate that the block contains
 *   non-zero coefficients
 */
Void TTraTop::xMarkTbCbfTrue(TComTURecurse& tu, ComponentID component) {
        TComDataCU& cu              = *tu.getCU();
  const UInt        tuPartIndex     = tu.GetAbsPartIdxTU();
  const UInt        numPartsInTu    = tu.GetAbsPartIdxNumParts();
  const UInt        tuRelativeDepth = tu.GetTransformDepthRel();
  const UInt        cbfValue        = 0x1 << tuRelativeDepth;

  cu.bitwiseOrCbfPartRange(
    cbfValue,
    component,
    tuPartIndex,
    numPartsInTu
  );
}


/**
 * Clears the cbf for a given transform block
 */
Void TTraTop::xClearTbCbf(TComTURecurse& tu, ComponentID component) {
        TComDataCU& cu           = *tu.getCU();
  const UInt        numPartsInTu = tu.GetAbsPartIdxNumParts();
  const UInt        tuPartIndex  = tu.GetAbsPartIdxTU();

  cu.clearCbf(
    tuPartIndex,
    component,
    numPartsInTu
  );
}


/**
 * Checks the cbf of a given transform block to determine if the block has
 *   non-zero coefficients
 */
Bool TTraTop::xHasNonzeroCoefficients(TComTURecurse& tu, ComponentID component) {
        TComDataCU& cu              = *tu.getCU();
  const UInt        tuRelativeDepth = tu.GetTransformDepthRel();
  const UInt        tuPartIndex     = tu.GetAbsPartIdxTU();

  return (cu.getCbf(tuPartIndex, component, tuRelativeDepth) != 0);
}


/**
 * Find an existing TComPic by POC
 */
TComPic* TTraTop::getPicByPoc(Int poc) {
  return xGetEncPicByPoc(poc);
}


/**
 * Detects the case where requantization removed all residual coefficients for
 *   an inter-predicted cu coded in merge mode and adjusts the cu to skip mode
 */
Void TTraTop::xReevaluateSkipModeDecision(TComDataCU& cu) const {
  const Bool isUsingMergePrediction =
    cu.isInter(0) && (cu.getMergeFlag(0) || cu.getSkipFlag(0));

  if (isUsingMergePrediction && cu.getPartitionSize(0) == SIZE_2Nx2N) {
    cu.setSkipFlagSubParts(cu.getQtRootCbf(0) == 0, 0, cu.getDepth(0));
  }
}


/**
 * Applies transform and quantization to prediction error
 */
Void TTraTop::xApplyResidualTransQuant(TComTURecurse& tu, ComponentID component, TComYuv& residualBuffer) {
        TComTrQuant&   transQuant    = *getTrQuant();
        TComDataCU&    cu            = *tu.getCU();
  const TComRectangle& tuRect        = tu.getRect(component);
  const Int            cuStride      = residualBuffer.getStride(component);
  const UInt           tuPelOffset   = tuRect.x0 + cuStride * tuRect.y0;
  const UInt           tuCoeffOffset = tu.getCoefficientOffset(component);
        Pel*           pResidual     = residualBuffer.getAddr(component) + tuPelOffset;
        TCoeff*        pCoeff        = cu.getCoeff(component) + tuCoeffOffset;

  const QpParam qp(cu, component);

  // TODO: Is this necessary for transcoding?
#if RDOQ_CHROMA_LAMBDA
  transQuant.selectLambda(component);
#endif

  // Transform and quantize
  TCoeff absSum = 0;
  transQuant.transformNxN(
    tu,
    component,
    pResidual,
    cuStride,
    pCoeff,
#if ADAPTIVE_QP_SELECTION
    cu.getArlCoeff(component) + tuCoeffOffset,
#endif
    absSum,
    qp
  );

  // Inverse transform and quantize
  transQuant.invTransformNxN(
    tu,
    component,
    pResidual,
    cuStride,
    pCoeff,
    qp
  );
}


/**
 * Constructs the intra-mode prediction for a given prediction block
 */
Void TTraTop::xConstructIntraPrediction(TComTURecurse& pu, ComponentID component, TComYuv& predictionBuffer) {
  TComPrediction& predictor = *getPredSearch();

  const Bool shouldFilterReferenceSamples =
    xShouldFilterIntraReferenceSamples(pu, component);

  // Fill intra prediction reference sample buffers
  predictor.initIntraPatternChType(
    pu,
    component,
    shouldFilterReferenceSamples
  );

  // Construct intra prediction
  predictor.predIntraAng(
    component,
    xGetTbIntraDirection(pu, component),
    nullptr,
    0,
    predictionBuffer.getAddr(component, pu.GetAbsPartIdxTU()),
    predictionBuffer.getStride(component),
    pu,
    shouldFilterReferenceSamples
  );
}
