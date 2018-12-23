#include "TDbrCavlc.h"


Void TDbrCavlc::setBitstreams(TDbrStreamSet* bitstreams) {
  this->bitstreams = bitstreams;
}


TDbrStreamSet* TDbrCavlc::getBitstreams() {
  return bitstreams;
}


Void TDbrCavlc::parseVPS(TComVPS* pcVPS) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::VPS));
  TDecCavlc::parseVPS(pcVPS);
}


Void TDbrCavlc::parseSPS(TComSPS* pcSPS) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPS));
  TDecCavlc::parseSPS(pcSPS);
}


Void TDbrCavlc::parsePPS(TComPPS* pcPPS) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PPS));
  TDecCavlc::parsePPS(pcPPS);
}


Void TDbrCavlc::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecCavlc::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
}


Void TDbrCavlc::parseTerminatingBit(UInt& ruilsLast) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecCavlc::parseTerminatingBit(ruilsLast);
}


Void TDbrCavlc::parseRemainingBytes(Bool noTrailingBytesExpected) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SLICE));
  TDecCavlc::parseRemainingBytes(noTrailingBytesExpected);
}


Void TDbrCavlc::parseMVPIdx(Int& riMVPIdx) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVP_INDEX));
  TDecCavlc::parseMVPIdx(riMVPIdx);
}


Void TDbrCavlc::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SKIP_FLAG));
  TDecCavlc::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TQ_BYPASS_FLAG));
  TDecCavlc::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::SPLIT_FLAG));
  TDecCavlc::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_FLAG));
  TDecCavlc::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
}


Void TDbrCavlc::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MERGE_INDEX));
  TDecCavlc::parseMergeIndex(pcCU, ruiMergeIndex);
}


Void TDbrCavlc::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PART_SIZE));
  TDecCavlc::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::PRED_MODE));
  TDecCavlc::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_LUMA));
  TDecCavlc::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTRA_DIR_CHROMA));
  TDecCavlc::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::INTER_DIR));
  TDecCavlc::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
}


Void TDbrCavlc::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::REF_FRAME_INDEX));
  TDecCavlc::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
}


Void TDbrCavlc::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::MVD));
  TDecCavlc::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
}


Void TDbrCavlc::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CROSS_COMP_PRED));
  TDecCavlc::parseCrossComponentPrediction(rTu, compID);
}


Void TDbrCavlc::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SUBDIV_FLAG));
  TDecCavlc::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
}


Void TDbrCavlc::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  TDecCavlc::parseQtCbf(rTu, compID, lowestLevel);
}


Void TDbrCavlc::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::QT_CBF));
  TDecCavlc::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
}


Void TDbrCavlc::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::DQP));
  TDecCavlc::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::CHROMA_QP_ADJ));
  TDecCavlc::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::IPCM));
  TDecCavlc::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
}


Void TDbrCavlc::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::COEFF));
  TDecCavlc::parseCoeffNxN(rTu, compID);
}


Void TDbrCavlc::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::TU_SKIP_FLAG));
  TDecCavlc::parseTransformSkipFlags(rTu, component);
}


Void TDbrCavlc::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  SyntaxElementWriter::setBitstream(&bitstreams->getBitstream(TDbrStreamSet::STREAM::RDPCM));
  TDecCavlc::parseExplicitRdpcmMode(rTu, compID);
}


Void TDbrCavlc::xReadCode(UInt length, UInt& val) {
  SyntaxElementParser::xReadCode(length, val);
  SyntaxElementWriter::xWriteCode(val, length);
}


Void TDbrCavlc::xReadUvlc(UInt& val) {
  SyntaxElementParser::xReadUvlc(val);
  SyntaxElementWriter::xWriteUvlc(val);
}


Void TDbrCavlc::xReadSvlc(Int& val) {
  SyntaxElementParser::xReadSvlc(val);
  SyntaxElementWriter::xWriteSvlc(val);
}


Void TDbrCavlc::xReadFlag(UInt& val) {
  SyntaxElementParser::xReadFlag(val);
  SyntaxElementWriter::xWriteFlag(val);
}


Void TDbrCavlc::xReadRbspTrailingBits() {
  SyntaxElementParser::xReadRbspTrailingBits();
  SyntaxElementWriter::xWriteRbspTrailingBits();
}
