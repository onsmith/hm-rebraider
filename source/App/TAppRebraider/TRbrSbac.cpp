#include "TRbrSbac.h"


Void TRbrSbac::setBitstreams(TRbrStreamSet* bitstreams) {
  this->bitstreams = bitstreams;
}


TRbrStreamSet* TRbrSbac::getBitstreams() {
  return bitstreams;
}


Void TRbrSbac::parseVPS(TComVPS* pcVPS) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::VPS));
  TDecSbac::parseVPS(pcVPS);
}


Void TRbrSbac::parseSPS(TComSPS* pcSPS) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SPS));
  TDecSbac::parseSPS(pcSPS);
}


Void TRbrSbac::parsePPS(TComPPS* pcPPS) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::PPS));
  TDecSbac::parsePPS(pcPPS);
}


Void TRbrSbac::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SLICE));
  TDecSbac::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
}


Void TRbrSbac::parseTerminatingBit(UInt& ruilsLast) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SLICE));
  TDecSbac::parseTerminatingBit(ruilsLast);
}


Void TRbrSbac::parseRemainingBytes(Bool noTrailingBytesExpected) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SLICE));
  TDecSbac::parseRemainingBytes(noTrailingBytesExpected);
}


Void TRbrSbac::parseMVPIdx(Int& riMVPIdx) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MVP_INDEX));
  TDecSbac::parseMVPIdx(riMVPIdx);
}


Void TRbrSbac::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SKIP_FLAG));
  TDecSbac::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::TQ_BYPASS_FLAG));
  TDecSbac::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SPLIT_FLAG));
  TDecSbac::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MERGE_FLAG));
  TDecSbac::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
}


Void TRbrSbac::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MERGE_INDEX));
  TDecSbac::parseMergeIndex(pcCU, ruiMergeIndex);
}


Void TRbrSbac::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::PART_SIZE));
  TDecSbac::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::PRED_MODE));
  TDecSbac::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::INTRA_DIR_LUMA));
  TDecSbac::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::INTRA_DIR_CHROMA));
  TDecSbac::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::INTER_DIR));
  TDecSbac::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
}


Void TRbrSbac::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::REF_FRAME_INDEX));
  TDecSbac::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
}


Void TRbrSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MVD));
  TDecSbac::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
}


Void TRbrSbac::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::CROSS_COMP_PRED));
  TDecSbac::parseCrossComponentPrediction(rTu, compID);
}


Void TRbrSbac::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  TDecSbac::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
}


Void TRbrSbac::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::QT_CBF));
  TDecSbac::parseQtCbf(rTu, compID, lowestLevel);
}


Void TRbrSbac::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::QT_CBF));
  TDecSbac::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
}


Void TRbrSbac::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::DQP));
  TDecSbac::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::CHROMA_QP_ADJ));
  TDecSbac::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::IPCM));
  TDecSbac::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrSbac::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::COEFF));
  TDecSbac::parseCoeffNxN(rTu, compID);
}


Void TRbrSbac::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::TU_SKIP_FLAG));
  TDecSbac::parseTransformSkipFlags(rTu, component);
}


Void TRbrSbac::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  init(&vlcReader);
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::RDPCM));
  TDecSbac::parseExplicitRdpcmMode(rTu, compID);
}
