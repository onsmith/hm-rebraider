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
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::VPS));
  TDecSbac::parseVPS(pcVPS);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSPS(TComSPS* pcSPS) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPS));
  TDecSbac::parseSPS(pcSPS);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parsePPS(TComPPS* pcPPS) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PPS));
  TDecSbac::parsePPS(pcPPS);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecSbac::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseTerminatingBit(UInt& ruilsLast) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecSbac::parseTerminatingBit(ruilsLast);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseRemainingBytes(Bool noTrailingBytesExpected) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecSbac::parseRemainingBytes(noTrailingBytesExpected);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseMVPIdx(Int& riMVPIdx) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVP_INDEX));
  TDecSbac::parseMVPIdx(riMVPIdx);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SKIP_FLAG));
  TDecSbac::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TQ_BYPASS_FLAG));
  TDecSbac::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPLIT_FLAG));
  TDecSbac::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_FLAG));
  TDecSbac::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_INDEX));
  TDecSbac::parseMergeIndex(pcCU, ruiMergeIndex);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PART_SIZE));
  TDecSbac::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PRED_MODE));
  TDecSbac::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_LUMA));
  TDecSbac::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_CHROMA));
  TDecSbac::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTER_DIR));
  TDecSbac::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::REF_FRAME_INDEX));
  TDecSbac::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVD));
  TDecSbac::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CROSS_COMP_PRED));
  TDecSbac::parseCrossComponentPrediction(rTu, compID);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  TDecSbac::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  TDecSbac::parseQtCbf(rTu, compID, lowestLevel);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  TDecSbac::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::DQP));
  TDecSbac::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CHROMA_QP_ADJ));
  TDecSbac::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::IPCM));
  TDecSbac::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::COEFF));
  TDecSbac::parseCoeffNxN(rTu, compID);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SKIP_FLAG));
  TDecSbac::parseTransformSkipFlags(rTu, component);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::RDPCM));
  TDecSbac::parseExplicitRdpcmMode(rTu, compID);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSaoMaxUvlc(UInt& val, UInt maxSymbol) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoMaxUvlc(val, maxSymbol);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSaoMerge(UInt& ruiVal) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoMerge(ruiVal);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSaoTypeIdx(UInt& ruiVal) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoTypeIdx(ruiVal);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSaoUflc(UInt uiLength, UInt& ruiVal) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoUflc(uiLength, ruiVal);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSAOBlkParam(SAOBlkParam& saoBlkParam, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, const BitDepths& bitDepths) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_BLOCK_PARAMS));
  TDecSbac::parseSAOBlkParam(saoBlkParam, sliceEnabled, leftMergeAvail, aboveMergeAvail, bitDepths);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseSaoSign(UInt& val) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SAO_PARAMS));
  TDecSbac::parseSaoSign(val);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseLastSignificantXY(UInt& uiPosLastX, UInt& uiPosLastY, Int width, Int height, ComponentID component, UInt uiScanIdx) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::LAST_SIG_XY));
  TDecSbac::parseLastSignificantXY(uiPosLastX, uiPosLastY, width, height, component, uiScanIdx);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


Void TDbrSbac::parseScalingList(TComScalingList* scalingList) {
  TComBitIf* oldOutputBitstream = cabacReader->getOutputBitstream();
  cabacReader->setOutputBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SCALING_LIST));
  TDecSbac::parseScalingList(scalingList);
  cabacReader->setOutputBitstream(oldOutputBitstream);
}


