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
  init(cabacReader);
  TDecSbac::parseVPS(pcVPS);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSPS(TComSPS* pcSPS) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPS));
  init(cabacReader);
  TDecSbac::parseSPS(pcSPS);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parsePPS(TComPPS* pcPPS) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PPS));
  init(cabacReader);
  TDecSbac::parsePPS(pcPPS);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  init(cabacReader);
  TDecSbac::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseTerminatingBit(UInt& ruilsLast) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  TComInputBitstream* newInputBitstream = &bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE);
  cabacReader->setInputBitstream(newInputBitstream);
  init(cabacReader);
  TDecSbac::setBitstream(newInputBitstream);
  TDecSbac::parseTerminatingBit(ruilsLast);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseRemainingBytes(Bool noTrailingBytesExpected) {
  //TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  //cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  //init(cabacReader);
  //TDecSbac::parseRemainingBytes(noTrailingBytesExpected);
  //cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMVPIdx(Int& riMVPIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVP_INDEX));
  init(cabacReader);
  TDecSbac::parseMVPIdx(riMVPIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SKIP_FLAG));
  init(cabacReader);
  TDecSbac::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TQ_BYPASS_FLAG));
  init(cabacReader);
  TDecSbac::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPLIT_FLAG));
  init(cabacReader);
  TDecSbac::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_FLAG));
  init(cabacReader);
  TDecSbac::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_INDEX));
  init(cabacReader);
  TDecSbac::parseMergeIndex(pcCU, ruiMergeIndex);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PART_SIZE));
  init(cabacReader);
  TDecSbac::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PRED_MODE));
  init(cabacReader);
  TDecSbac::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_LUMA));
  init(cabacReader);
  TDecSbac::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_CHROMA));
  init(cabacReader);
  TDecSbac::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTER_DIR));
  init(cabacReader);
  TDecSbac::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::REF_FRAME_INDEX));
  init(cabacReader);
  TDecSbac::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVD));
  init(cabacReader);
  TDecSbac::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CROSS_COMP_PRED));
  init(cabacReader);
  TDecSbac::parseCrossComponentPrediction(rTu, compID);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  init(cabacReader);
  TDecSbac::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  init(cabacReader);
  TDecSbac::parseQtCbf(rTu, compID, lowestLevel);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  init(cabacReader);
  TDecSbac::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::DQP));
  init(cabacReader);
  TDecSbac::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CHROMA_QP_ADJ));
  init(cabacReader);
  TDecSbac::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::IPCM));
  init(cabacReader);
  TDecSbac::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::COEFF));
  init(cabacReader);
  TDecSbac::parseCoeffNxN(rTu, compID);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SKIP_FLAG));
  init(cabacReader);
  TDecSbac::parseTransformSkipFlags(rTu, component);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::RDPCM));
  init(cabacReader);
  TDecSbac::parseExplicitRdpcmMode(rTu, compID);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoMaxUvlc(UInt& val, UInt maxSymbol) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  init(cabacReader);
  TDecSbac::parseSaoMaxUvlc(val, maxSymbol);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoMerge(UInt& ruiVal) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  init(cabacReader);
  TDecSbac::parseSaoMerge(ruiVal);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoTypeIdx(UInt& ruiVal) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  init(cabacReader);
  TDecSbac::parseSaoTypeIdx(ruiVal);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoUflc(UInt uiLength, UInt& ruiVal) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  init(cabacReader);
  TDecSbac::parseSaoUflc(uiLength, ruiVal);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSAOBlkParam(SAOBlkParam& saoBlkParam, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, const BitDepths& bitDepths) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_BLOCK_PARAMS));
  init(cabacReader);
  TDecSbac::parseSAOBlkParam(saoBlkParam, sliceEnabled, leftMergeAvail, aboveMergeAvail, bitDepths);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseSaoSign(UInt& val) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  init(cabacReader);
  TDecSbac::parseSaoSign(val);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseLastSignificantXY(UInt& uiPosLastX, UInt& uiPosLastY, Int width, Int height, ComponentID component, UInt uiScanIdx) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::LAST_SIG_XY));
  init(cabacReader);
  TDecSbac::parseLastSignificantXY(uiPosLastX, uiPosLastY, width, height, component, uiScanIdx);
  cabacReader->setInputBitstream(oldInputBitstream);
}


Void TDbrSbac::parseScalingList(TComScalingList* scalingList) {
  TComInputBitstream* oldInputBitstream = cabacReader->getInputBitstream();
  cabacReader->setInputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SCALING_LIST));
  init(cabacReader);
  TDecSbac::parseScalingList(scalingList);
  cabacReader->setInputBitstream(oldInputBitstream);
}
