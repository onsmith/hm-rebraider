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
  //m_vps = &vps;
  setVPS(&vps);
  xEncodeVPS(vps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded SPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSPS& sps) {
  //*m_spsMap.allocatePS(sps.getSPSId()) = sps;
  *getSpsMap()->allocatePS(sps.getSPSId()) = sps;
  xEncodeSPS(sps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded PPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComPPS& pps) {
  //m_ppsMap.allocatePS(pps.getPPSId()) = pps;
  *getPpsMap()->allocatePS(pps.getPPSId()) = pps;
  xEncodePPS(pps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded slice NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSlice& slice) {
  TComPic&   encPic   = xGetEncPicBySlice(slice);
  TComSlice& encSlice = xCopySliceToPic(slice, encPic);

  // Set the current slice
  encPic.setCurrSliceIdx(encPic.getNumAllocatedSlice() - 1);

  // Compress and encode the slice
  xCompressSlice(encSlice);
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

  // Allocate temporary encoding substreams
  std::vector<TComOutputBitstream> substreams(numEncSubstreams(slice));

  // Get encoder references
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  TEncSlice&   sliceEncoder   = *getSliceEncoder();

  // Write slice head to output bitstream
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.resetEntropy(&slice);
  entropyEncoder.setBitstream(&bitstream);
  slice.setEncCABACTableIdx(sliceEncoder.getEncCABACTableIdx());
  entropyEncoder.encodeSliceHeader(&slice);

  // Encode slice body data to temporary substreams
  UInt numBinsCoded = 0;
  slice.setFinalized(true);
  slice.clearSubstreamSizes();
  sliceEncoder.setSliceIdx(slice.getSliceIdx());
  sliceEncoder.encodeSlice(&pic, substreams.data(), numBinsCoded);
  
  // Write WPP entry points to output bitstream
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodeTilesWPPEntryPoint(&slice);

  // Concatenate substreams and append to the output bitstream
  bitstream.writeByteAlignment();
  const Int numZeroSubstreamsAtStartOfSlice = pic.getSubstreamForCtuAddr(slice.getSliceSegmentCurStartCtuTsAddr(), false, &slice);
  const Int numSubstreamsToCode             = slice.getNumberOfSubstreamSizes() + 1;
  for (UInt substreamIndex = 0 ; substreamIndex < numSubstreamsToCode; substreamIndex++) {
    bitstream.addSubstream(substreams.data() + substreamIndex + numZeroSubstreamsAtStartOfSlice);
  }
}


/**
 * Compress a decoded slice by choosing compression parameters
 */
Void TTraTop::xCompressSlice(TComSlice& slice) {
  
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

  // Copy basic slice data to destination slice
  dstSlice.copySliceInfo(&srcSlice);

  // Copy decoded slice information
  {
    const TComPic& srcPic         = *srcSlice.getPic();
    const UInt     startCtuTsAddr = srcSlice.getSliceSegmentCurStartCtuTsAddr();
    const UInt     endCtuTsAddr   = srcSlice.getSliceSegmentCurEndCtuTsAddr();
    for (UInt ctuTsAddr = startCtuTsAddr; ctuTsAddr < endCtuTsAddr; ctuTsAddr++) {
      const UInt        ctuRsAddr = srcPic.getPicSym()->getCtuTsToRsAddrMap(ctuTsAddr);
      const TComDataCU& srcCtu    = *srcPic.getPicSym()->getCtu(ctuRsAddr);
            TComDataCU& dstCtu    = *dstPic.getPicSym()->getCtu(ctuRsAddr);
      dstCtu = srcCtu;
      dstCtu.setSlice(&dstSlice);
    }
  }

  // Use encoder picture object instead of decoder object
  dstSlice.setPic(&dstPic);

  // Use encoder picture references instead of decoder references
  for (Int iList = 0; iList < NUM_REF_PIC_LIST_01; iList++) {
    for (Int iPic = 0; iPic < MAX_NUM_REF; iPic++) {
      RefPicList iRefPicList = static_cast<RefPicList>(iList);
      TComPic* decRefPic = dstSlice.getRefPic(iRefPicList, iPic);
      if (decRefPic != nullptr) {
        TComPic* encRefPic = xGetEncPicByPoc(decRefPic->getPOC());
        dstSlice.setRefPic(encRefPic, iRefPicList, iPic);
      }
    }
  }

  return dstSlice;
}


/**
 * Get an unused entry from the picture buffer
 */
TComPic*& TTraTop::xGetUnusedEntry() {
  TComList<TComPic*>& cpb = *getListPic();

  // Linearly search the buffer for an existing unused entry
  for (auto it = cpb.begin(); it != cpb.end(); it++) {
    TComPic*& pPic = *it;

    if (pPic != nullptr && !pPic->getOutputMark()) {
      // Bypass dpb management, just reuse picture
      if (!pPic->getReconMark() || !pPic->getSlice(0)->isReferenced()) {
        return pPic;
      }

      if (!pPic->getReconMark()) {
        return pPic;

      // TODO: Is this necessary?
      } else if (!pPic->getSlice(0)->isReferenced()) {
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

  // Copy relevant TComPicSym data
  // TODO: m_dpbPerCtuData;
  // TODO: m_saoBlkParams;

#if ADAPTIVE_QP_SELECTION
  // TODO: m_pParentARLBuffer;
#endif

  // Set up m_pictureCtuArray pointers
  for (UInt ctuRsAddr = 0; ctuRsAddr < dstPic.getPicSym()->getNumberOfCtusInFrame(); ctuRsAddr++) {
    dstPic.getCtu(ctuRsAddr)->attachToPic(dstPic, ctuRsAddr);
  }

  // Use decoded TComPicYuv plane as encoder source plane
  TComPicYuv* reconPlane = const_cast<TComPic&>(srcPic).getPicYuvRec();
  dstPic.setPicYuvOrg(reconPlane);
  dstPic.setPicYuvTrueOrg(reconPlane);

  // Clear slice buffer
  dstPic.clearSliceBuffer();
}
