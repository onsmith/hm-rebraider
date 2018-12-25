#include "TDbrSbac.h"


Void TDbrSbac::setBitstreams(TDbrStreamSet* bitstreams) {
  this->bitstreams = bitstreams;
}


TDbrStreamSet* TDbrSbac::getBitstreams() {
  return bitstreams;
}


Void TDbrSbac::setCabacReader(TDbrBinCABAC* cabacReader) {
  this->cabacReader = cabacReader;
}


Void TDbrSbac::parseVPS(TComVPS* pcVPS) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::VPS));
  TDecSbac::parseVPS(pcVPS);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSPS(TComSPS* pcSPS) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPS));
  TDecSbac::parseSPS(pcSPS);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parsePPS(TComPPS* pcPPS) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PPS));
  TDecSbac::parsePPS(pcPPS);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecSbac::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseTerminatingBit(UInt& ruilsLast) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecSbac::parseTerminatingBit(ruilsLast);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseRemainingBytes(Bool noTrailingBytesExpected) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecSbac::parseRemainingBytes(noTrailingBytesExpected);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMVPIdx(Int& riMVPIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVP_INDEX));
  TDecSbac::parseMVPIdx(riMVPIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SKIP_FLAG));
  TDecSbac::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TQ_BYPASS_FLAG));
  TDecSbac::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPLIT_FLAG));
  TDecSbac::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_FLAG));
  TDecSbac::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_INDEX));
  TDecSbac::parseMergeIndex(pcCU, ruiMergeIndex);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PART_SIZE));
  TDecSbac::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PRED_MODE));
  TDecSbac::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_LUMA));
  TDecSbac::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_CHROMA));
  TDecSbac::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTER_DIR));
  TDecSbac::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::REF_FRAME_INDEX));
  TDecSbac::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVD));
  TDecSbac::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CROSS_COMP_PRED));
  TDecSbac::parseCrossComponentPrediction(rTu, compID);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  TDecSbac::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  TDecSbac::parseQtCbf(rTu, compID, lowestLevel);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  TDecSbac::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::DQP));
  TDecSbac::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CHROMA_QP_ADJ));
  TDecSbac::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::IPCM));
  TDecSbac::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::COEFF));
  TDecSbac::parseCoeffNxN(rTu, compID);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SKIP_FLAG));
  TDecSbac::parseTransformSkipFlags(rTu, component);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::RDPCM));
  TDecSbac::parseExplicitRdpcmMode(rTu, compID);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoMaxUvlc(UInt& val, UInt maxSymbol) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoMaxUvlc(val, maxSymbol);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoMerge(UInt& ruiVal) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoMerge(ruiVal);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoTypeIdx(UInt& ruiVal) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoTypeIdx(ruiVal);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoUflc(UInt uiLength, UInt& ruiVal) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoUflc(uiLength, ruiVal);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSAOBlkParam(SAOBlkParam& saoBlkParam, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, const BitDepths& bitDepths) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_BLOCK_PARAMS));
  TDecSbac::parseSAOBlkParam(saoBlkParam, sliceEnabled, leftMergeAvail, aboveMergeAvail, bitDepths);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoSign(UInt& val) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoSign(val);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseLastSignificantXY(UInt& uiPosLastX, UInt& uiPosLastY, Int width, Int height, ComponentID component, UInt uiScanIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::LAST_SIG_XY));
  TDecSbac::parseLastSignificantXY(uiPosLastX, uiPosLastY, width, height, component, uiScanIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseScalingList(TComScalingList* scalingList) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SCALING_LIST));
  TDecSbac::parseScalingList(scalingList);
  cabacReader->setInputBitstream(oldInputBitstream);
}
