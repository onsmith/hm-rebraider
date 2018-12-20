#include "TDbrEntropy.h"




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

}


Void TDbrEntropy::codeMVPIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {

}


/**
  * 
  */
Void TDbrEntropy::codeCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeMergeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {

}




/**
  *
  */
Void TDbrEntropy::codePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {

}


Void TDbrEntropy::codePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}




/**
  *
  */
Void TDbrEntropy::codeIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}




/**
  *
  */
Void TDbrEntropy::codeTransformSubdivFlag(UInt uiSymbol, UInt uiCtx) {

}


Void TDbrEntropy::codeQtCbf(TComTU &rTu, const ComponentID compID, const Bool lowestLevel) {

}


Void TDbrEntropy::codeQtRootCbf(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeQtCbfZero(TComTU &rTu, const ChannelType chType) {

}


Void TDbrEntropy::codeQtRootCbfZero() {

}


Void TDbrEntropy::codeIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool isMultiplePU) {

}




/**
  *
  */
Void TDbrEntropy::codeIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeRefFrmIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {

}


Void TDbrEntropy::codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {

}




/**
  *
  */
Void TDbrEntropy::codeCrossComponentPrediction(TComTU &rTu, ComponentID compID) {

}




/**
  *
  */
Void TDbrEntropy::codeDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx) {
  cavlc.setBitstream(dqp_bitstream);
  cavlc.codeDeltaQP(pcCU, uiAbsPartIdx);
}


Void TDbrEntropy::codeChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeCoeffNxN(TComTU &rTu, TCoeff* pcCoef, const ComponentID compID) {

}


Void TDbrEntropy::codeTransformSkipFlags(TComTU &rTu, ComponentID component) {

}


Void TDbrEntropy::codeSAOBlkParam(SAOBlkParam& saoBlkParam, const BitDepths &bitDepths, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, Bool onlyEstMergeInfo = false) {

}


Void TDbrEntropy::estBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType, COEFF_SCAN_TYPE scanType) {

}


Void TDbrEntropy::codeExplicitRdpcmMode(TComTU &rTu, const ComponentID compID) {

}
