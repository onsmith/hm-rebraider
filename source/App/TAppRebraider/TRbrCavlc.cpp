#include "TRbrCavlc.h"


Void TRbrCavlc::setBitstreams(TRbrStreamSet* bitstreams) {
  this->bitstreams = bitstreams;
}


TRbrStreamSet* TRbrCavlc::getBitstreams() {
  return bitstreams;
}


Void TRbrCavlc::parseVPS(TComVPS* pcVPS) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::VPS));
  TDecCavlc::parseVPS(pcVPS);
}


Void TRbrCavlc::parseSPS(TComSPS* pcSPS) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SPS));
  TDecCavlc::parseSPS(pcSPS);
}


Void TRbrCavlc::parsePPS(TComPPS* pcPPS) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::PPS));
  TDecCavlc::parsePPS(pcPPS);
}


Void TRbrCavlc::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SLICE));
  TDecCavlc::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
}


Void TRbrCavlc::parseTerminatingBit(UInt& ruilsLast) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SLICE));
  TDecCavlc::parseTerminatingBit(ruilsLast);
}


Void TRbrCavlc::parseRemainingBytes(Bool noTrailingBytesExpected) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SLICE));
  TDecCavlc::parseRemainingBytes(noTrailingBytesExpected);
}


Void TRbrCavlc::parseMVPIdx(Int& riMVPIdx) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MVP_INDEX));
  TDecCavlc::parseMVPIdx(riMVPIdx);
}


Void TRbrCavlc::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SKIP_FLAG));
  TDecCavlc::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::TQ_BYPASS_FLAG));
  TDecCavlc::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::SPLIT_FLAG));
  TDecCavlc::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MERGE_FLAG));
  TDecCavlc::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
}


Void TRbrCavlc::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MERGE_INDEX));
  TDecCavlc::parseMergeIndex(pcCU, ruiMergeIndex);
}


Void TRbrCavlc::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::PART_SIZE));
  TDecCavlc::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::PRED_MODE));
  TDecCavlc::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::INTRA_DIR_LUMA));
  TDecCavlc::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::INTRA_DIR_CHROMA));
  TDecCavlc::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::INTER_DIR));
  TDecCavlc::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
}


Void TRbrCavlc::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::REF_FRAME_INDEX));
  TDecCavlc::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
}


Void TRbrCavlc::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::MVD));
  TDecCavlc::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
}


Void TRbrCavlc::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::CROSS_COMP_PRED));
  TDecCavlc::parseCrossComponentPrediction(rTu, compID);
}


Void TRbrCavlc::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  TDecCavlc::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
}


Void TRbrCavlc::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::QT_CBF));
  TDecCavlc::parseQtCbf(rTu, compID, lowestLevel);
}


Void TRbrCavlc::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::QT_CBF));
  TDecCavlc::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
}


Void TRbrCavlc::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::DQP));
  TDecCavlc::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::CHROMA_QP_ADJ));
  TDecCavlc::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::IPCM));
  TDecCavlc::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
}


Void TRbrCavlc::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::COEFF));
  TDecCavlc::parseCoeffNxN(rTu, compID);
}


Void TRbrCavlc::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::TU_SKIP_FLAG));
  TDecCavlc::parseTransformSkipFlags(rTu, component);
}


Void TRbrCavlc::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  setBitstream(&bitstreams->getStream(TRbrStreamSet::STREAM::RDPCM));
  TDecCavlc::parseExplicitRdpcmMode(rTu, compID);
}
