#include "TDbrEntropy.h"



/**
 * Default constructor passes the cabac object to the sbac object
 */
TDbrEntropy::TDbrEntropy() {
  sbac.init(&cabac);
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
  cavlc.setBitstream(vps_bitstream);
  cavlc.codeVPS(pcVPS);
}


/**
 * Encodes a sequence parameter set to a bitstream
 */
Void TDbrEntropy::codeSPS(const TComSPS* pcSPS) {
  cavlc.setBitstream(sps_bitstream);
  cavlc.codeSPS(pcSPS);
}


/**
 * Encodes a picture parameter set to a bitstream
 */
Void TDbrEntropy::codePPS(const TComPPS* pcPPS) {
  cavlc.setBitstream(pps_bitstream);
  cavlc.codePPS(pcPPS);
}


/**
 * Encodes a slice header to a bitstream
 */
Void TDbrEntropy::codeSliceHeader(TComSlice* pcSlice) {
  cavlc.setBitstream(slice_bitstream);
  cavlc.codeSliceHeader(pcSlice);
}


/**
  * Encodes the end of a slice to a bitstream
  */
Void TDbrEntropy::codeSliceFinish() {
  cavlc.setBitstream(slice_bitstream);
  cavlc.codeSliceFinish();
}


/**
  * Encodes tiles and wavefront substreams sizes for the slice header (entry
  *   points).
  */
Void TDbrEntropy::codeTilesWPPEntryPoint(TComSlice* pSlice) {
  cavlc.setBitstream(slice_bitstream);
  cavlc.codeTilesWPPEntryPoint(pSlice);
}








Void TDbrEntropy::codeTerminatingBit(UInt uilsLast) {
  // sbac.codeTerminatingBit(uilsLast);
}


Void TDbrEntropy::codeMVPIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {
  cabac.init(mvp_index_bitstream);
  sbac.codeMVPIdx(pcCU, uiAbsPartIdx, eRefList);
}


Void TDbrEntropy::codeCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(tq_bypass_bitstream);
  sbac.codeCUTransquantBypassFlag(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(skip_flag_bitstream);
  sbac.codeSkipFlag(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(merge_flag_bitstream);
  sbac.codeMergeFlag(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeMergeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(merge_index_bitstream);
  sbac.codeMergeIndex(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  cabac.init(split_flag_bitstream);
  sbac.codeSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrEntropy::codePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  cabac.init(part_size_bitstream);
  sbac.codePartSize(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrEntropy::codePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(pred_mode_bitstream);
  sbac.codePredMode(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(ipcm_bitstream);
  sbac.codeIPCMInfo(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeTransformSubdivFlag(UInt uiSymbol, UInt uiCtx) {
  cabac.init(tu_subdiv_flag_bitstream);
  sbac.codeTransformSubdivFlag(uiSymbol, uiCtx);
}


Void TDbrEntropy::codeQtCbf(TComTU &rTu, const ComponentID compID, const Bool lowestLevel) {
  cabac.init(qt_cbf_bitstream);
  sbac.codeQtCbf(rTu, compID, lowestLevel);
}


Void TDbrEntropy::codeQtRootCbf(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(qt_cbf_bitstream);
  sbac.codeQtRootCbf(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeQtCbfZero(TComTU &rTu, const ChannelType chType) {
  cabac.init(qt_cbf_bitstream);
  sbac.codeQtCbfZero(rTu, chType);
}


Void TDbrEntropy::codeQtRootCbfZero() {
  cabac.init(qt_cbf_bitstream);
  sbac.codeQtRootCbfZero();
}


Void TDbrEntropy::codeIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool isMultiplePU) {
  cabac.init(intra_mode_luma_bitstream);
  sbac.codeIntraDirLumaAng(pcCU, uiAbsPartIdx, isMultiplePU);
}


Void TDbrEntropy::codeIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(intra_mode_chroma_bitstream);
  sbac.codeIntraDirChroma(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(inter_dir_bitstream);
  sbac.codeInterDir(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeRefFrmIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {
  cabac.init(ref_frame_index_bitstream);
  sbac.codeRefFrmIdx(pcCU, uiAbsPartIdx, eRefList);
}


Void TDbrEntropy::codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {
  cabac.init(mvd_bitstream);
  sbac.codeMvd(pcCU, uiAbsPartIdx, eRefList);
}


Void TDbrEntropy::codeCrossComponentPrediction(TComTU &rTu, ComponentID compID) {
  cabac.init(cross_comp_pred_bitstream);
  sbac.codeCrossComponentPrediction(rTu, compID);
}


Void TDbrEntropy::codeDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cavlc.setBitstream(dqp_bitstream);
  cavlc.codeDeltaQP(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cabac.init(chroma_qp_adj_bitstream);
  sbac.codeChromaQpAdjustment(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeCoeffNxN(TComTU &rTu, TCoeff* pcCoef, const ComponentID compID) {
  cabac.init(coeff_bitstream);
  sbac.codeCoeffNxN(rTu, pcCoef, compID);
}


Void TDbrEntropy::codeTransformSkipFlags(TComTU &rTu, ComponentID component) {
  cabac.init(tu_skip_flag_bitstream);
  sbac.codeTransformSkipFlags(rTu, component);
}


Void TDbrEntropy::codeSAOBlkParam(SAOBlkParam& saoBlkParam, const BitDepths &bitDepths, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, Bool onlyEstMergeInfo = false) {
  cabac.init(sao_blk_param_bitstream);
  sbac.codeSAOBlkParam(saoBlkParam, bitDepths, sliceEnabled, leftMergeAvail, aboveMergeAvail, onlyEstMergeInfo);
}


Void TDbrEntropy::estBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType, COEFF_SCAN_TYPE scanType) {
  assert(0);
}


Void TDbrEntropy::codeExplicitRdpcmMode(TComTU &rTu, const ComponentID compID) {
  cabac.init(rdpcm_bitstream);
  sbac.codeExplicitRdpcmMode(rTu, compID);
}
