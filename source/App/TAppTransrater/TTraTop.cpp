#include "TTraTop.h"


/**
 * Default constructor for transrater class
 */
TTraTop::TTraTop() {
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
 * Encode a slice and write to bitstream
 */
Void TTraTop::xEncodeSlice(TComSlice& slice, TComOutputBitstream& bitstream) {
  TEncEntropy& entropyEncoder = *getEntropyCoder();
  TEncCavlc&   cavlcEncoder   = *getCavlcCoder();
  TEncSlice&   sliceEncoder   = *getSliceEncoder();

  // Write slice head to output bitstream
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.resetEntropy(&slice);
  entropyEncoder.setBitstream(&bitstream);
  slice.setEncCABACTableIdx(sliceEncoder.getEncCABACTableIdx());
  entropyEncoder.encodeSliceHeader(&slice);

  // Encode slice body to temporary bitstream
  UInt numBinsCoded = 0;
  TComOutputBitstream sliceBody;
  slice.setFinalized(true);
  slice.clearSubstreamSizes();
  sliceEncoder.encodeSlice(slice.getPic(), &sliceBody, numBinsCoded);
  
  // Write WPP entry point to output bitstream
  entropyEncoder.setEntropyCoder(&cavlcEncoder);
  entropyEncoder.setBitstream(&bitstream);
  entropyEncoder.encodeTilesWPPEntryPoint(&slice);

  // Write slice body to output bitstream
  bitstream.writeByteAlignment();
  bitstream.addSubstream(&sliceBody);
}


/**
 * Compress a decoded slice by choosing compression parameters
 */
Void TTraTop::xCompressSlice(TComSlice& slice) {
  
}


/**
 * Encode a slice and write to bitstream
 *
UInt TTraTop::xCompressSlice(TComSlice& slice, TComPic& pic, TComOutputBitstream* bitstreams) {
  // Cache beginning/ending CTU address for the current slice
  const UInt startCtuTsAddr = slice.getSliceSegmentCurStartCtuTsAddr();
  const UInt endCtuTsAddr   = slice.getSliceSegmentCurEndCtuTsAddr();

  // Cache frame width
  const UInt frameWidthInCtus = pic.getPicSym()->getFrameWidthInCtus();

  // Cache slice options from PPS
  const Bool areDependentSliceSegmentsEnabled = slice.getPPS()->getDependentSliceSegmentsEnabledFlag();
  const Bool areWavefrontsEnabled             = slice.getPPS()->getEntropyCodingSyncEnabledFlag();

  // Initialise entropy code encoder for the slice
  m_sbacEncoder.init(&m_cabacEncoder);
  m_entropyEncoder.setEntropyCoder(&m_sbacEncoder);
  m_entropyEncoder.resetEntropy(&slice);

  // Reset cabac encoder
  UInt numBinsCoded = 0;
  m_cabacEncoder.setBinCountingEnableFlag(true);
  m_cabacEncoder.setBinsCoded(0);

  // If this is a dependent slice, load contexts from previous slice segment
  if (areDependentSliceSegmentsEnabled) {
    const UInt      ctuRasterScanAddress = pic.getPicSym()->getCtuTsToRsAddrMap(startCtuTsAddr);
    const UInt      currentTileIndex     = pic.getPicSym()->getTileIdxMap(ctuRasterScanAddress);
    const TComTile& currentTile          = *pic.getPicSym()->getTComTile(currentTileIndex);
    const UInt      firstCtuOfTileRsAddr = currentTile.getFirstCtuRsAddr();

    const Bool isDependentSlice = (
      slice.getDependentSliceSegmentFlag() &&
      ctuRasterScanAddress != firstCtuOfTileRsAddr &&
      (currentTile.getTileWidthInCtus() >= 2 || !areWavefrontsEnabled)
    );

    if (isDependentSlice) {
      m_sbacEncoder.loadContexts(&m_lastSliceSegmentEndContextState);
    }
  }

  // Loop through every CTU in the slice segment
  for (UInt ctuTsAddr = startCtuTsAddr; ctuTsAddr < endCtuTsAddr; ctuTsAddr++) {
    const UInt      ctuRsAddr            = pic.getPicSym()->getCtuTsToRsAddrMap(ctuTsAddr);
    const TComTile& currentTile          = *pic.getPicSym()->getTComTile(pic.getPicSym()->getTileIdxMap(ctuRsAddr));
    const UInt      firstCtuRsAddrOfTile = currentTile.getFirstCtuRsAddr();
    const UInt      tileXPosInCtus       = firstCtuRsAddrOfTile % frameWidthInCtus;
    const UInt      tileYPosInCtus       = firstCtuRsAddrOfTile / frameWidthInCtus;
    const UInt      ctuXPosInCtus        = ctuRsAddr % frameWidthInCtus;
    const UInt      ctuYPosInCtus        = ctuRsAddr / frameWidthInCtus;
    const UInt      uiSubStrm            = pic.getSubstreamForCtuAddr(ctuRsAddr, true, &slice);
    TComDataCU&     ctu                  = *pic.getCtu(ctuRsAddr);

    // Set output stream
    m_entropyEncoder.setBitstream(&bitstreams[uiSubStrm]);

    // Set up CABAC contexts state for this CTU
    if (ctuRsAddr == firstCtuRsAddrOfTile) {
      if (ctuTsAddr != startCtuTsAddr) { // if it is the first CTU, then the entropy coder has already been reset
        m_entropyEncoder.resetEntropy(&slice);
      }
    } else if (ctuXPosInCtus == tileXPosInCtus && areWavefrontsEnabled) {
      // Synchronize cabac probabilities with upper-right CTU if it's available and at the start of a line.
      if (ctuTsAddr != startCtuTsAddr) { // if it is the first CTU, then the entropy coder has already been reset
        m_entropyEncoder.resetEntropy(&slice);
      }
      TComDataCU* pCtuUp = ctu.getCtuAbove();
      if (pCtuUp != nullptr && (ctuRsAddr % frameWidthInCtus + 1) < frameWidthInCtus) {
        TComDataCU* pCtuTR = pic.getCtu(ctuRsAddr - frameWidthInCtus + 1);
        if (ctu.CUIsFromSameSliceAndTile(pCtuTR)) {
          // Top-right is available, so use it.
          m_sbacEncoder.loadContexts(&m_entropyCodingSyncContextState);
        }
      }
    }

    // If SAO is enabled, encode SAO parameters
    if (slice.getSPS()->getUseSAO()) {
      Bool isSAOSliceEnabled = false;
      Bool sliceEnabled[MAX_NUM_COMPONENT];
      for (Int comp = 0; comp < MAX_NUM_COMPONENT; comp++) {
        ComponentID compId   = static_cast<ComponentID>(comp);
        ChannelType chanType = toChannelType(compId);

        sliceEnabled[comp] = (
          slice.getSaoEnabledFlag(chanType) &&
          comp < pic.getNumberValidComponents()
        );

        isSAOSliceEnabled = isSAOSliceEnabled || sliceEnabled[comp];
      }

      if (isSAOSliceEnabled) {
        SAOBlkParam& saoblkParam = pic.getPicSym()->getSAOBlkParam()[ctuRsAddr];

        // Merge left condition
        const Bool isLeftMergeAvailable = (
          ctuRsAddr % frameWidthInCtus > 0 &&
          pic.getSAOMergeAvailability(ctuRsAddr, ctuRsAddr - 1)
        );

        // Merge up condition
        const Bool isAboveMergeAvailable = (
          ctuRsAddr / frameWidthInCtus > 0 &&
          pic.getSAOMergeAvailability(ctuRsAddr, ctuRsAddr-frameWidthInCtus)
        );

        // Encode parameters
        m_entropyEncoder.encodeSAOBlkParam(
          saoblkParam,
          pic.getPicSym()->getSPS().getBitDepths(),
          sliceEnabled,
          isLeftMergeAvailable,
          isAboveMergeAvailable
        );
      }
    }

    m_cuEncoder.encodeCtu(&ctu);

    // Store probabilities of second CTU in line into buffer
    if (ctuXPosInCtus == tileXPosInCtus+1 && areWavefrontsEnabled) {
      m_entropyCodingSyncContextState.loadContexts(m_sbacEncoder);
    }

    // terminate the sub-stream, if required (end of slice-segment, end of tile, end of wavefront-CTU-row):
    if (ctuTsAddr+1 == endCtuTsAddr ||
         (  ctuXPosInCtus + 1 == tileXPosInCtus + currentTile.getTileWidthInCtus() &&
          ( ctuYPosInCtus + 1 == tileYPosInCtus + currentTile.getTileHeightInCtus() || areWavefrontsEnabled)
         )
       )
    {
      m_entropyEncoder.encodeTerminatingBit(1);
      m_entropyEncoder.encodeSliceFinish();
      // Byte-alignment in slice_data() when new tile
      bitstreams[uiSubStrm].writeByteAlignment();

      // write sub-stream size
      if (ctuTsAddr+1 != endCtuTsAddr) {
        slice.addSubstreamSize((bitstreams[uiSubStrm].getNumberOfWrittenBits() >> 3) + bitstreams[uiSubStrm].countStartCodeEmulations());
      }
    }
  } // CTU-loop

  if (areDependentSliceSegmentsEnabled) {
    m_lastSliceSegmentEndContextState.loadContexts(m_sbacEncoder); //ctx end of dep.slice
  }

  if (m_pcCfg->getUseAdaptQpSelect()) {
    m_transQuant.storeSliceQpNext(&slice); // TODO: this will only be storing the adaptive QP state of the very last slice-segment that is not dependent in the frame... Perhaps this should be moved to the compress slice loop.
  }

  if (slice.getPPS()->getCabacInitPresentFlag() && !slice.getPPS()->getDependentSliceSegmentsEnabledFlag()) {
    m_encCABACTableIdx = m_entropyEncoder.determineCabacInitIdx(&slice);
  } else {
    m_encCABACTableIdx = slice.getSliceType();
  }

  numBinsCoded = m_cabacEncoder.getBinsCoded();
}*/


/**
 * Resolve a TComSlice into its corresponding encoder TComPic
 */
TComPic& TTraTop::xGetEncPicBySlice(const TComSlice& slice) {
  TComPic* pPic = xGetEncPicByPoc(slice.getPOC());
  if (pPic != nullptr) {
    return *pPic;
  }

  TComPic*& pPicEntry = xGetUnusedEntry();
  if (pPicEntry == nullptr) {
    pPicEntry = new TComPic;
  }

  TComPPS& pps = *getPpsMap()->getPS(slice.getPPSId());
  TComSPS& sps = *getSpsMap()->getPS(pps.getSPSId());
  pPicEntry->create(sps, pps, false, true);
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
  dstSlice.copySliceInfo(&srcSlice);

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
