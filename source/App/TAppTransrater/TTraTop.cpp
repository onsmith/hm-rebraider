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
    //saoProcessor.SAOProcess(&pic);
    //saoProcessor.PCMLFDisableProcess(&pic);
  }

  // Compress motion
  pic.compressMotion();

  // Mark for output
  pic.setOutputMark(pic.getSlice(0)->getPicOutputFlag());
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

    if (pPic != nullptr && pPic->getOutputMark() && !(pPic->getReconMark() && pPic->getSlice(0)->isReferenced())) {
      pPic->setReconMark(false);
      pPic->getPicYuvRec()->setBorderExtension(false);
      return pPic;
    }


    if (pPic != nullptr && !pPic->getOutputMark()) {
      if (!pPic->getReconMark()) {
        return pPic;
      }

      if (!pPic->getSlice(0)->isReferenced()) {
        pPic->setReconMark(false);
        pPic->getPicYuvRec()->setBorderExtension(false);
        return pPic;
      }
    }
  }

  // All entries are used, so make a new entry at the back of the list
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
 * Set up cu buffers for every cu depth possible
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
  dstPic.setTLayer(srcPic.getTLayer());
  dstPic.setUsedByCurr(srcPic.getUsedByCurr());
  dstPic.setIsLongTerm(srcPic.getIsLongTerm());
  dstPic.setReconMark(srcPic.getReconMark());
  dstPic.setOutputMark(srcPic.getOutputMark());
  dstPic.setCurrSliceIdx(srcPic.getCurrSliceIdx());
  dstPic.setCheckLTMSBPresent(srcPic.getCheckLTMSBPresent());
  dstPic.setTopField(srcPic.isTopField());
  dstPic.setField(srcPic.isField());
  // TODO: m_SEIs

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


  // If the current cu is skipped or lossless encoded, we can't requantize
  if (ctu.isSkipped(cuPartAddr) || ctu.isLosslessCoded(cuPartAddr)) {
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

  // Requantize cu
  if (ctu.isInter(cuPartAddr)) {
    xRequantizeInterCu(cu);
  } else if (ctu.isIntra(cuPartAddr)) {
    //xRequantizeIntraCu(cu);
  } else {
    assert(0);
  }
}


/**
 * Requantizes an inter-predicted cu
 */
Void TTraTop::xRequantizeInterCu(TComDataCU& cu) {
  UInt cuDepth = cu.getDepth(0);

  // Get original, prediction, residual, and reconstruction yuv buffers
  TComYuv& origBuff = m_originalBuffer[cuDepth];
  TComYuv& predBuff = m_predictionBuffer[cuDepth];
  TComYuv& resiBuff = m_residualBuffer[cuDepth];
  TComYuv& recoBuff = m_reconstructionBuffer[cuDepth];
  
  // Obtain original by copying yuv data from source picture
  origBuff.copyFromPicYuv(
    cu.getPic()->getPicYuvOrg(),
    cu.getCtuRsAddr(),
    cu.getZorderIdxInCtu()
  );

  // Obtain prediction by applying motion vectors
  getPredSearch()->motionCompensation(&cu, &predBuff);

  // Obtain prediction error by computing (original - prediction)
  // Note: store in residual
  resiBuff.subtract(&origBuff, &predBuff, 0, cu.getWidth(0));

  // Obtain real residual by transforming and quantizing prediction error
  TComTURecurse tu(&cu, 0, cuDepth);
  for (UInt c = 0; c < cu.getPic()->getNumberValidComponents(); c++) {
    xRequantizeInterTu(tu, static_cast<ComponentID>(c));
  }

  // Obtain reconstructed signal by computing (prediction + residual)
  recoBuff.addClip(
    &predBuff,
    &resiBuff,
    0,
    cu.getWidth(0),
    cu.getSlice()->getSPS()->getBitDepths()
  );

  // Copy reconstructed signal back into picture
  recoBuff.copyToPicYuv(
    cu.getPic()->getPicYuvRec(),
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
  TComYuv&     resiBuff   = m_originalBuffer[cu.getDepth(0)];

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

  // Handle recursive case
  if (uiTrMode != cu.getTransformIdx(tuPartIndex)) {
    TComTURecurse tuChild(tu, false);
    do {
      xRequantizeInterTu(tuChild, component);
    } while (tuChild.nextSection(tu));
    return;
  }

  const TComRectangle& tuRect      = tu.getRect(component);
  const Int            uiStride    = resiBuff.getStride(component);
        Pel*           rpcResidual = resiBuff.getAddr(component);
        UInt           uiAddr      = tuRect.x0 + uiStride * tuRect.y0;
        Pel*           pResi       = rpcResidual + uiAddr;
        TCoeff*        pcCoeff     = cu.getCoeff(component) + tu.getCoefficientOffset(component);

  // Requantization
  if (!areAllCoefficientsZero) {
    const QpParam qp(cu, component);

    // Transform and quantize
    TCoeff absSum = 0;
    transQuant.transformNxN(
      tu,
      component,
      pResi,
      uiStride,
      pcCoeff,
#if ADAPTIVE_QP_SELECTION
      cu.getArlCoeff(component),
#endif
      absSum,
      qp
    );

    // Inverse transform and quantize
    transQuant.invTransformNxN(
      tu,
      component,
      pResi,
      uiStride,
      pcCoeff,
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
    const Pel *piResiLuma = resiBuff.getAddr(COMPONENT_Y);
    const Int  strideLuma = resiBuff.getStride(COMPONENT_Y);
    const Int  tuWidth    = tu.getRect(component).width;
    const Int  tuHeight   = tu.getRect(component).height;
          Pel* pResi     = rpcResidual + uiAddr;
    const Pel* pResiLuma = piResiLuma  + uiAddr;

    transQuant.crossComponentPrediction(
      tu,
      component,
      pResiLuma,
      pResi,
      pResi,
      tuWidth,
      tuHeight,
      strideLuma,
      uiStride,
      uiStride,
      true
    );
  }
}



/**
 * Requantizes an intra-predicted cu
 *
Void TTraTop::xRequantizeIntraCu(TComDataCU& cu, TComYuv& predBuff, TComYuv& resiBuff, TComYuv& recoBuff) {
  if (cu.getIPCMFlag(0)) {
    assert(0);
    // xReconPCM(&cu, cu.getDepth(0));
    return;
  }

  ChromaFormat chromaFormat = cu.getPic()->getChromaFormat();
  const UInt numChannelTypes = (chromaFormat == CHROMA_400 ? 1 : 2);

  for (UInt i = CHANNEL_TYPE_LUMA; i < numChannelTypes; i++) {
    const ChannelType channelType = static_cast<ChannelType>(i);

    Bool shouldSplit = (
      cu.getPartitionSize(0) != SIZE_2Nx2N &&
      (isLuma(channelType) || enable4ChromaPUsInIntraNxNCU(chromaFormat))
    );

    TComTURecurse tuParent(&cu, 0);

    TComTURecurse tu(
      tuParent,
      false,
      shouldSplit ? TComTU::QUAD_SPLIT : TComTU::DONT_SPLIT
    );

    do {
      xPredictIntraTu(tu, predictionBuffer, channelType);
    } while (tu.nextSection(tuParent));
  }
}


/**
 * Recursively requantizes an intra-predicted tu
 *
Void TTraTop::xRequantizeIntraTu(TComTURecurse& tu, ChannelType channelType, TComYuv& predBuff) {
        TComPrediction& predictor = *getPredSearch();
        TComDataCU&     cu        = *tu.getCU();
  const TComSPS&        sps       = *cu.getSlice()->getSPS();

  const UInt uiChPredMode  = cu.getIntraDir(toChannelType(compID), uiAbsPartIdx);
  const UInt partsPerMinCU = 1 << (2 * (sps.getMaxTotalCUDepth() - sps.getLog2DiffMaxMinCodingBlockSize()));
  const UInt uiChCodedMode = (uiChPredMode==DM_CHROMA_IDX && !bIsLuma) ? cu.getIntraDir(CHANNEL_TYPE_LUMA, getChromasCorrespondingPULumaIdx(uiAbsPartIdx, chFmt, partsPerMinCU)) : uiChPredMode;
  const UInt uiChFinalMode = ((chFmt == CHROMA_422)       && !bIsLuma) ? g_chroma422IntraAngleMappingTable[uiChCodedMode] : uiChCodedMode;

  const Bool bUseFilteredPredictions = TComPrediction::filteringIntraReferenceSamples(compID, uiChFinalMode, uiWidth, uiHeight, chFmt, sps.getSpsRangeExtension().getIntraSmoothingDisabledFlag());

  predictor.initIntraPatternChType(rTu, compID, bUseFilteredPredictions);

  predictor.predIntraAng(compID, uiChFinalMode, nullptr, 0, piPred, uiStride, rTu, bUseFilteredPredictions);
}
*/
