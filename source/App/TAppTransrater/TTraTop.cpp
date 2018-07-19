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
  *getPpsMap()->allocatePS(pps.getPPSId()) = pps;
  xEncodePPS(pps, outputNalu.m_Bitstream);
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
    //saoProcessor.reconstructBlkSAOParams(&pic, saoBlockParam);
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

  // If the current cu is skipped or lossless encoded, we can't requantize
  if (cu.isSkipped(0) || cu.isLosslessCoded(0)) {
    xCopyCuPixels(cu, *pic.getPicYuvOrg(), *pic.getPicYuvRec());
    return;
  }

  // Requantize cu
  if (cu.isInter(0)) {
    xRequantizeInterCu(cu);
  } else if (cu.isIntra(0)) {
    xRequantizeIntraCu(cu);
  } else {
    assert(0);
  }
}


template<typename T>
static Void printMat(const T* pMat, UInt width, UInt height) {
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      printf("%6i ", pMat[w]);
    }
    std::cout << std::endl;
    pMat += width;
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
    xRequantizeInterTu(tu, static_cast<ComponentID>(c));
  }

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
 * Recursively requantizes an inter-predicted tu
 */
Void TTraTop::xRequantizeInterTu(TComTURecurse& tu, ComponentID component) {
  TComTrQuant& transQuant = *getTrQuant();
  TComDataCU&  cu         = *tu.getCU();
  UChar        cuDepth    = cu.getDepth(0);
  TComYuv&     origBuff   = m_originalBuffer[cuDepth];
  TComYuv&     predBuff   = m_predictionBuffer[cuDepth];
  TComYuv&     resiBuff   = m_residualBuffer[cuDepth];

  if (!tu.ProcessComponentSection(component)) {
    return;
  }

  UInt tuPartIndex = tu.GetAbsPartIdxTU();
  UInt uiTrMode = tu.GetTransformDepthRel();

  Bool areAllCoefficientsZero =
    cu.getCbf(tuPartIndex, component, uiTrMode) == 0;

  Bool isCrossComponentPredictionEnabled =
    cu.getSlice()
      ->getPPS()
      ->getPpsRangeExtension()
      .getCrossComponentPredictionEnabledFlag();

  Bool canSkipTransQuant =
    areAllCoefficientsZero &&
    (isLuma(component) || !isCrossComponentPredictionEnabled);

  if (canSkipTransQuant) {
    return;
  }

  // Recurse through tus
  if (uiTrMode != cu.getTransformIdx(tuPartIndex)) {
    TComTURecurse tuChild(tu, false);
    do {
      xRequantizeInterTu(tuChild, component);
    } while (tuChild.nextSection(tu));
    return;
  }

  const TComRectangle& tuRect    = tu.getRect(component);
  const Int            stride    = resiBuff.getStride(component);
  const UInt           tuOffset  = tuRect.x0 + stride * tuRect.y0;
        Pel*           pResidual = resiBuff.getAddr(component) + tuOffset;
        TCoeff*        pCoeff    = cu.getCoeff(component) + tu.getCoefficientOffset(component);

  // Requantization
  if (!areAllCoefficientsZero) {
    const QpParam qp(cu, component);

    // DEBUG: Copy initial coefficients
    UInt numCoeffs =
      (tu.getRect(component).width * tu.getRect(component).height) >>
      (isChroma(component) ? 2 : 0);
    TCoeff* beforeCoeffs = new TCoeff[numCoeffs];
    memcpy(beforeCoeffs, pCoeff, numCoeffs * sizeof(TCoeff));

    // Transform and quantize
    TCoeff absSum = 0;
    transQuant.transformNxN(
      tu,
      component,
      pResidual,
      stride,
      pCoeff,
#if ADAPTIVE_QP_SELECTION
      cu.getArlCoeff(component),
#endif
      absSum,
      qp
    );

    // DEBUG: Compare coefficients and output to stdout if different
    Bool areCoeffsSame = true;
    for (int i = 0; i < numCoeffs; i++) {
      if (beforeCoeffs[i] != pCoeff[i]) {
        areCoeffsSame = false;
        break;
      }
    }
    if (!areCoeffsSame) {
      std::cout << "Source:\n";
      printMat(
        origBuff.getAddr(component) + tuOffset,
        tu.getRect(component).width  >> (isChroma(component) ? 1 : 0),
        tu.getRect(component).height >> (isChroma(component) ? 1 : 0)
      );
      std::cout << std::endl;

      std::cout << "Prediction:\n";
      printMat(
        predBuff.getAddr(component) + tuOffset,
        tu.getRect(component).width  >> (isChroma(component) ? 1 : 0),
        tu.getRect(component).height >> (isChroma(component) ? 1 : 0)
      );
      std::cout << std::endl;

      std::cout << "Residual:\n";
      printMat(
        pResidual,
        tu.getRect(component).width  >> (isChroma(component) ? 1 : 0),
        tu.getRect(component).height >> (isChroma(component) ? 1 : 0)
      );
      std::cout << std::endl;

      std::cout << "Decoded Coeff:\n";
      printMat(
        beforeCoeffs,
        tu.getRect(component).width  >> (isChroma(component) ? 1 : 0),
        tu.getRect(component).height >> (isChroma(component) ? 1 : 0)
      );
      std::cout << std::endl;

      std::cout << "Recoded Coeff:\n";
      printMat(
        pCoeff,
        tu.getRect(component).width  >> (isChroma(component) ? 1 : 0),
        tu.getRect(component).height >> (isChroma(component) ? 1 : 0)
      );
      std::cout << std::endl;
      std::getchar();
    } else {
      std::cout << "Coeffs match!\n";
    }
    delete[] beforeCoeffs;

    // Inverse transform and quantize
    transQuant.invTransformNxN(
      tu,
      component,
      pResidual,
      stride,
      pCoeff,
      qp
    );
  }

  Bool isCrossComponentPredictionUsed =
    cu.getCrossComponentPredictionAlpha(tuPartIndex, component) != 0;

  Bool areAllLumaCoefficientsZero =
    cu.getCbf(tuPartIndex, COMPONENT_Y, uiTrMode) == 0;

  Bool shouldApplyCrossComponentPrediction = (
    isChroma(component) &&
    isCrossComponentPredictionUsed &&
    !areAllLumaCoefficientsZero
   );

  // Cross-component prediction
  if (shouldApplyCrossComponentPrediction) {
    const Int  strideLuma    = resiBuff.getStride(COMPONENT_Y);
    const Pel* pResidualLuma = resiBuff.getAddr(COMPONENT_Y) + tuOffset;

    transQuant.crossComponentPrediction(
      tu,            //       TComTU&       rTu,
      component,     // const ComponentID   compID,
      pResidualLuma, // const Pel*          piResiL,
      pResidual,     // const Pel*          piResiC,
      pResidual,     //       Pel*          piResiT,
      tuRect.width,  // const Int           width,
      tuRect.height, // const Int           height,
      strideLuma,    // const Int           strideL,
      stride,        // const Int           strideC,
      stride,        // const Int           strideT,
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
    //xReconPCM( pcCU, uiDepth );
    //return;
  }

  // Requantize luma
  {
    Bool allowSplit = (cu.getPartitionSize(0) != SIZE_2Nx2N);

    TComTURecurse parentTu(&cu, 0);
    TComTURecurse tu(
      parentTu,
      false,
      allowSplit ? TComTU::QUAD_SPLIT : TComTU::DONT_SPLIT
    );

    do {
      xRequantizeIntraLumaTu(tu);
    } while (tu.nextSection(parentTu));
  }

  // Requantize chroma
  if (hasChroma) {
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
      xRequantizeIntraChromaTu(tu);
    } while (tu.nextSection(parentTu));
  }
}


/**
 * Requantizes an intra-predicted luma tu
 */
Void TTraTop::xRequantizeIntraLumaTu(TComTURecurse& tu) {
  TComDataCU& cu           = *tu.getCU();
  UInt        uiTrDepth    = tu.GetTransformDepthRel();
  UInt        uiAbsPartIdx = tu.GetAbsPartIdxTU();
  UInt        uiTrMode     = cu.getTransformIdx(uiAbsPartIdx);

  // Recurse through tus
  if (uiTrMode != uiTrDepth) {
    TComTURecurse tuChild(tu, false);
    do {
      xRequantizeIntraLumaTu(tuChild);
    } while (tuChild.nextSection(tu));
    return;
  }

  xRequantizeIntraTu(tu, COMPONENT_Y);
}


/**
 * Requantizes an intra-predicted chroma tu
 */
Void TTraTop::xRequantizeIntraChromaTu(TComTURecurse& tu) {
  TComDataCU& cu           = *tu.getCU();
  UInt        uiTrDepth    = tu.GetTransformDepthRel();
  UInt        uiAbsPartIdx = tu.GetAbsPartIdxTU();
  UInt        uiTrMode     = cu.getTransformIdx(uiAbsPartIdx);

  // Recurse through tus
  if (uiTrMode != uiTrDepth) {
    TComTURecurse tuChild(tu, false);
    do {
      xRequantizeIntraLumaTu(tuChild);
    } while (tuChild.nextSection(tu));
    return;
  }

  const UInt numValidComp = getNumberValidComponents(tu.GetChromaFormat());
  for (UInt c = COMPONENT_Cb; c < numValidComp; c++) {
    xRequantizeIntraTu(tu, static_cast<ComponentID>(c));
  }
}


/**
 * Recursively requantizes an intra-predicted tu
 */
Void TTraTop::xRequantizeIntraTu(TComTURecurse& tu, ComponentID component) {
  // Bail if this TU should not be processed
  if (!tu.ProcessComponentSection(component)) {
    return;
  }

        TComDataCU& cu           = *tu.getCU();
  const UInt        cuDepth      = cu.getDepth(0);

  // Get original, prediction, residual, and reconstruction yuv buffers
  TComYuv& origBuff = m_originalBuffer[cuDepth];
  TComYuv& predBuff = m_predictionBuffer[cuDepth];
  TComYuv& resiBuff = m_residualBuffer[cuDepth];
  TComYuv& recoBuff = m_reconstructionBuffer[cuDepth];

  const TComRectangle& tuRect   = tu.getRect(component);
  const UInt           tuWidth  = tuRect.width;
  const UInt           tuHeight = tuRect.height;

  // Handle vertical split
  if (tuWidth != tuHeight) {
    TComTURecurse childTu(tu, false, TComTU::VERTICAL_SPLIT, true, component);

    do {
      xRequantizeIntraTu(childTu, component);
    } while (childTu.nextSection(tu));
    return;
  }
  
  const UInt         uiAbsPartIdx = tu.GetAbsPartIdxTU();
  const TComSPS&     sps          = *cu.getSlice()->getSPS();
  const ChromaFormat chromaFormat = tu.GetChromaFormat();
  const UInt         cuStride     = recoBuff.getStride(component);
        Pel*         pPredict     = predBuff.getAddr(component, uiAbsPartIdx);
  const Bool         bIsLuma      = isLuma(component);

  const UInt predMode      = cu.getIntraDir(toChannelType(component), uiAbsPartIdx);
  const UInt partsPerMinCU = 1 << (2 * (sps.getMaxTotalCUDepth() - sps.getLog2DiffMaxMinCodingBlockSize()));
  const UInt codedMode     = (predMode == DM_CHROMA_IDX && !bIsLuma) ? cu.getIntraDir(CHANNEL_TYPE_LUMA, getChromasCorrespondingPULumaIdx(uiAbsPartIdx, chromaFormat, partsPerMinCU)) : predMode;
  const UInt finalMode     = (chromaFormat == CHROMA_422 && !bIsLuma) ? g_chroma422IntraAngleMappingTable[codedMode] : codedMode;

  const Bool shouldFilterPredictions =
    TComPrediction::filteringIntraReferenceSamples(
      component,
      finalMode,
      tuWidth,
      tuHeight,
      chromaFormat,
      sps.getSpsRangeExtension().getIntraSmoothingDisabledFlag()
  );

  TComPrediction& predictor = *getPredSearch();

  predictor.initIntraPatternChType(
    tu,
    component,
    shouldFilterPredictions
  );

  predictor.predIntraAng(
    component,
    finalMode,
    nullptr,
    0,
    pPredict,
    cuStride,
    tu,
    shouldFilterPredictions
  );
}
