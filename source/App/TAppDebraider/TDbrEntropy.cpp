#include "TDbrEntropy.h"



/**
 * Default constructor passes the cabac object to the sbac object
 */
TDbrEntropy::TDbrEntropy() {
  sbac.init(&cabac);
}


/**
 * Bitstream object setter
 */
Void TDbrEntropy::setBitstreams(TDbrStreamSet* bitstreams) {
  this->bitstreams = bitstreams;
}


/**
 * Bitstream object getter
 */
TDbrStreamSet* TDbrEntropy::getBitstreams() {
  return bitstreams;
}


/**
 * Writes a codeword to a bitstream
 */
Void TDbrEntropy::xWriteCode(TComBitIf& bitstream, UInt uiCode, UInt uiLength, const string& /* name */) {
  assert (uiLength > 0);
  bitstream.write(uiCode, uiLength);
}


/**
 * Writes an unsigned integer to a bitstream
 */
Void TDbrEntropy::xWriteUvlc(TComBitIf& bitstream, UInt uiCode, const string& /* name */) {
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;

  assert(uiTemp);

  while (1 != uiTemp) {
    uiTemp >>= 1;
    uiLength += 2;
  }

  // Handle case where uiLength > 32
  bitstream.write(0, uiLength >> 1);
  bitstream.write(uiCode, (uiLength + 1) >> 1);
}


/**
 * Writes an signed integer to a bitstream
 */
Void TDbrEntropy::xWriteSvlc(TComBitIf& bitstream, Int iCode, const string& name) {
  xWriteUvlc(bitstream, xConvertToUInt(iCode), name);
}


/**
 * Writes a boolean flag to a bitstream
 */
Void TDbrEntropy::xWriteFlag(TComBitIf& bitstream, UInt uiCode, const string& /* name*/ ) {
  bitstream.write(uiCode, 1);
}


/**
 * Writes aligning bits to a bitstream
 */
Void TDbrEntropy::xWriteRbspTrailingBits(TComBitIf& bitstream) {
  xWriteFlag(bitstream, 1, "rbsp_stop_one_bit");

  Int cnt = 0;
  while (bitstream.getNumBitsUntilByteAligned()) {
    xWriteFlag(bitstream, 0, "rbsp_alignment_zero_bit");
    cnt++;
  }

  assert(cnt < 8);
}


/**
 * Converts a signed integer to an unsigned integer
 */
UInt TDbrEntropy::xConvertToUInt(Int iValue) const {
  return (iValue <= 0) ? -iValue << 1 : (iValue << 1) - 1;
}




/**
 * Bitstream control methods
 */
Void TDbrEntropy::resetEntropy(const TComSlice *pSlice) {

}


SliceType TDbrEntropy::determineCabacInitIdx(const TComSlice *pSlice) {

}


Void TDbrEntropy::setBitstream(TComBitIf* p) {

}


Void TDbrEntropy::resetBits() {

}


UInt TDbrEntropy::getNumberOfWrittenBits() {

}




/**
 * Encodes a video parameter set to a bitstream
 */
Void TDbrEntropy::codeVPS(const TComVPS* pcVPS) {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::VPS));
  cavlc.codeVPS(pcVPS);
}


/**
 * Encodes a sequence parameter set to a bitstream
 */
Void TDbrEntropy::codeSPS(const TComSPS* pcSPS) {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::SPS));
  cavlc.codeSPS(pcSPS);
}


/**
 * Encodes a picture parameter set to a bitstream
 */
Void TDbrEntropy::codePPS(const TComPPS* pcPPS) {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::PPS));
  cavlc.codePPS(pcPPS);
}


/**
 * Encodes a slice header to a bitstream
 */
Void TDbrEntropy::codeSliceHeader(TComSlice* pcSlice) {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::SLICE));
  cavlc.codeSliceHeader(pcSlice);
}


/**
  * Encodes the end of a slice to a bitstream
  */
Void TDbrEntropy::codeSliceFinish() {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::SLICE));
  cavlc.codeSliceFinish();
}


/**
  * Encodes tiles and wavefront substreams sizes for the slice header (entry
  *   points).
  */
Void TDbrEntropy::codeTilesWPPEntryPoint(TComSlice* pSlice) {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::SLICE));
  cavlc.codeTilesWPPEntryPoint(pSlice);
}








Void TDbrEntropy::codeTerminatingBit(UInt uilsLast) {
  // sbac.codeTerminatingBit(uilsLast);
}


Void TDbrEntropy::codeMVPIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::MVP_INDEX));
  sbac.codeMVPIdx(pcCU, uiAbsPartIdx, eRefList);
}


Void TDbrEntropy::codeCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::TQ_BYPASS));
  sbac.codeCUTransquantBypassFlag(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::SKIP_FLAG));
  sbac.codeSkipFlag(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::MERGE_FLAG));
  sbac.codeMergeFlag(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeMergeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::MERGE_INDEX));
  sbac.codeMergeIndex(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::SPLIT_FLAG));
  sbac.codeSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrEntropy::codePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::PART_SIZE));
  sbac.codePartSize(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrEntropy::codePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::PRED_MODE));
  sbac.codePredMode(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::IPCM));
  sbac.codeIPCMInfo(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeTransformSubdivFlag(UInt uiSymbol, UInt uiCtx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  sbac.codeTransformSubdivFlag(uiSymbol, uiCtx);
}


Void TDbrEntropy::codeQtCbf(TComTU &rTu, const ComponentID compID, const Bool lowestLevel) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::QT_CBF));
  sbac.codeQtCbf(rTu, compID, lowestLevel);
}


Void TDbrEntropy::codeQtRootCbf(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::QT_CBF));
  sbac.codeQtRootCbf(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeQtCbfZero(TComTU &rTu, const ChannelType chType) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::QT_CBF));
  sbac.codeQtCbfZero(rTu, chType);
}


Void TDbrEntropy::codeQtRootCbfZero() {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::QT_CBF));
  sbac.codeQtRootCbfZero();
}


Void TDbrEntropy::codeIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool isMultiplePU) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::INTRA_MODE_LUMA));
  sbac.codeIntraDirLumaAng(pcCU, uiAbsPartIdx, isMultiplePU);
}


Void TDbrEntropy::codeIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::INTRA_MODE_CHROMA));
  sbac.codeIntraDirChroma(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::INTER_DIR));
  sbac.codeInterDir(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeRefFrmIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::REF_FRAME_INDEX));
  sbac.codeRefFrmIdx(pcCU, uiAbsPartIdx, eRefList);
}


Void TDbrEntropy::codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::MVD));
  sbac.codeMvd(pcCU, uiAbsPartIdx, eRefList);
}


Void TDbrEntropy::codeCrossComponentPrediction(TComTU &rTu, ComponentID compID) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::CROSS_COMP_PRED));
  sbac.codeCrossComponentPrediction(rTu, compID);
}


Void TDbrEntropy::codeDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cavlc.setBitstream(&bitstreams->operator()(TDbrStreamSet::STREAM::DQP));
  cavlc.codeDeltaQP(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::CHROMA_QP_ADJ));
  sbac.codeChromaQpAdjustment(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeCoeffNxN(TComTU &rTu, TCoeff* pcCoef, const ComponentID compID) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::COEFF));
  sbac.codeCoeffNxN(rTu, pcCoef, compID);
}


Void TDbrEntropy::codeTransformSkipFlags(TComTU &rTu, ComponentID component) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::TU_SKIP_FLAG));
  sbac.codeTransformSkipFlags(rTu, component);
}


Void TDbrEntropy::codeSAOBlkParam(SAOBlkParam& saoBlkParam, const BitDepths &bitDepths, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, Bool onlyEstMergeInfo = false) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::SAO_BLOCK_PARAMS));
  sbac.codeSAOBlkParam(saoBlkParam, bitDepths, sliceEnabled, leftMergeAvail, aboveMergeAvail, onlyEstMergeInfo);
}


Void TDbrEntropy::estBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType, COEFF_SCAN_TYPE scanType) {
  assert(0);
}


Void TDbrEntropy::codeExplicitRdpcmMode(TComTU &rTu, const ComponentID compID) {
  cabac.init(&bitstreams->operator()(TDbrStreamSet::STREAM::RDPCM));
  sbac.codeExplicitRdpcmMode(rTu, compID);
}
