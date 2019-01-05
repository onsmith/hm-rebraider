#include "TDbrSbac.h"


TDbrXmlReader* TDbrSbac::getXmlReader() {
  return stream;
}


Void TDbrSbac::setXmlReader(TDbrXmlReader* stream) {
  this->stream = stream;
}


Void TDbrSbac::setCabacReader(TDbrBinCABAC* cabacReader) {
  this->cabacReader = cabacReader;
}


Void TDbrSbac::parseTerminatingBit(UInt& ruilsLast) {
  assert(stream != nullptr);
  stream->readOpenTag("terminating-bit");
  setBitstream(&dummyStream);
  TDecSbac::parseTerminatingBit(ruilsLast);
  stream->readCloseTag("terminating-bit");
}


Void TDbrSbac::parseRemainingBytes(Bool noTrailingBytesExpected) {
  assert(stream != nullptr);
  stream->readOpenTag("remaining-bytes");
  TDecSbac::parseRemainingBytes(noTrailingBytesExpected);
  stream->readCloseTag("remaining-bytes");
}


Void TDbrSbac::parseMVPIdx(Int& riMVPIdx) {
  assert(stream != nullptr);
  stream->readOpenTag("mv-pred-index");
  TDecSbac::parseMVPIdx(riMVPIdx);
  stream->readCloseTag("mv-pred-index");
}


Void TDbrSbac::parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("skip-flag");
  TDecSbac::parseSkipFlag(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("skip-flag");
}


Void TDbrSbac::parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("tq-bypass-flag");
  TDecSbac::parseCUTransquantBypassFlag(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("tq-bypass-flag");
}


Void TDbrSbac::parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("split-flag");
  TDecSbac::parseSplitFlag(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("split-flag");
}


Void TDbrSbac::parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx) {
  assert(stream != nullptr);
  stream->readOpenTag("merge-flag");
  TDecSbac::parseMergeFlag(pcCU, uiAbsPartIdx, uiDepth, uiPUIdx);
  stream->readCloseTag("merge-flag");
}


Void TDbrSbac::parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex) {
  assert(stream != nullptr);
  stream->readOpenTag("merge-index");
  TDecSbac::parseMergeIndex(pcCU, ruiMergeIndex);
  stream->readCloseTag("merge-index");
}


Void TDbrSbac::parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("part-size");
  TDecSbac::parsePartSize(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("part-size");
}


Void TDbrSbac::parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("pred-mode");
  TDecSbac::parsePredMode(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("pred-mode");
}


Void TDbrSbac::parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("intra-dir-luma");
  TDecSbac::parseIntraDirLumaAng(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("intra-dir-luma");
}


Void TDbrSbac::parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("intra-dir-chroma");
  TDecSbac::parseIntraDirChroma(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("intra-dir-chroma");
}


Void TDbrSbac::parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx) {
  assert(stream != nullptr);
  stream->readOpenTag("inter-dir");
  TDecSbac::parseInterDir(pcCU, ruiInterDir, uiAbsPartIdx);
  stream->readCloseTag("inter-dir");
}


Void TDbrSbac::parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList) {
  assert(stream != nullptr);
  stream->readOpenTag("reference-frame-index");
  TDecSbac::parseRefFrmIdx(pcCU, riRefFrmIdx, eRefList);
  stream->readCloseTag("reference-frame-index");
}


Void TDbrSbac::parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList) {
  assert(stream != nullptr);
  stream->readOpenTag("mv-delta");
  TDecSbac::parseMvd(pcCU, uiAbsPartAddr, uiPartIdx, uiDepth, eRefList);
  stream->readCloseTag("mv-delta");
}


Void TDbrSbac::parseCrossComponentPrediction(TComTU& rTu, ComponentID compID) {
  assert(stream != nullptr);
  stream->readOpenTag("cross-comp-pred");
  TDecSbac::parseCrossComponentPrediction(rTu, compID);
  stream->readCloseTag("cross-comp-pred");
}


Void TDbrSbac::parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize) {
  assert(stream != nullptr);
  stream->readOpenTag("tu-subdiv-flag");
  TDecSbac::parseTransformSubdivFlag(ruiSubdivFlag, uiLog2TransformBlockSize);
  stream->readCloseTag("tu-subdiv-flag");
}


Void TDbrSbac::parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel) {
  assert(stream != nullptr);
  stream->readOpenTag("qt-cbf");
  TDecSbac::parseQtCbf(rTu, compID, lowestLevel);
  stream->readCloseTag("qt-cbf");
}


Void TDbrSbac::parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf) {
  assert(stream != nullptr);
  stream->readOpenTag("qt-root-cbf");
  TDecSbac::parseQtRootCbf(uiAbsPartIdx, uiQtRootCbf);
  stream->readCloseTag("qt-root-cbf");
}


Void TDbrSbac::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("delta-qp");
  TDecSbac::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("delta-qp");
}


Void TDbrSbac::parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("chroma-qp-adj");
  TDecSbac::parseChromaQpAdjustment(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("chroma-qp-adj");
}


Void TDbrSbac::parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("intra-pcm");
  TDecSbac::parseIPCMInfo(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("intra-pcm");
}


Void TDbrSbac::parseCoeffNxN(TComTU& rTu, ComponentID compID) {
  assert(stream != nullptr);
  stream->readOpenTag("coeff");
  TDecSbac::parseCoeffNxN(rTu, compID);
  stream->readCloseTag("coeff");
}


Void TDbrSbac::parseTransformSkipFlags(TComTU& rTu, ComponentID component) {
  assert(stream != nullptr);
  stream->readOpenTag("tu-skip");
  TDecSbac::parseTransformSkipFlags(rTu, component);
  stream->readCloseTag("tu-skip");
}


Void TDbrSbac::parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID) {
  assert(stream != nullptr);
  stream->readOpenTag("rd-pcm");
  TDecSbac::parseExplicitRdpcmMode(rTu, compID);
  stream->readCloseTag("rd-pcm");
}


Void TDbrSbac::parseSaoMaxUvlc(UInt& val, UInt maxSymbol) {
  assert(stream != nullptr);
  stream->readOpenTag("sao-max-uvlc");
  TDecSbac::parseSaoMaxUvlc(val, maxSymbol);
  stream->readCloseTag("sao-max-uvlc");
}


Void TDbrSbac::parseSaoMerge(UInt& ruiVal) {
  assert(stream != nullptr);
  stream->readOpenTag("sao-merge");
  TDecSbac::parseSaoMerge(ruiVal);
  stream->readCloseTag("sao-merge");
}


Void TDbrSbac::parseSaoTypeIdx(UInt& ruiVal) {
  assert(stream != nullptr);
  stream->readOpenTag("sao-type-index");
  TDecSbac::parseSaoTypeIdx(ruiVal);
  stream->readCloseTag("sao-type-index");
}


Void TDbrSbac::parseSaoUflc(UInt uiLength, UInt& ruiVal) {
  assert(stream != nullptr);
  stream->readOpenTag("sao-uflc");
  TDecSbac::parseSaoUflc(uiLength, ruiVal);
  stream->readCloseTag("sao-uflc");
}


Void TDbrSbac::parseSAOBlkParam(SAOBlkParam& saoBlkParam, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, const BitDepths& bitDepths) {
  assert(stream != nullptr);
  stream->readOpenTag("sao-block-params");
  TDecSbac::parseSAOBlkParam(saoBlkParam, sliceEnabled, leftMergeAvail, aboveMergeAvail, bitDepths);
  stream->readCloseTag("sao-block-params");
}


Void TDbrSbac::parseSaoSign(UInt& val) {
  assert(stream != nullptr);
  stream->readOpenTag("sao-sign");
  TDecSbac::parseSaoSign(val);
  stream->readCloseTag("sao-sign");
}


Void TDbrSbac::parseLastSignificantXY(UInt& uiPosLastX, UInt& uiPosLastY, Int width, Int height, ComponentID component, UInt uiScanIdx) {
  assert(stream != nullptr);
  stream->readOpenTag("last-sig-xy");
  TDecSbac::parseLastSignificantXY(uiPosLastX, uiPosLastY, width, height, component, uiScanIdx);
  stream->readCloseTag("last-sig-xy");
}


Void TDbrSbac::parseScalingList(TComScalingList* scalingList) {
  assert(stream != nullptr);
  stream->readOpenTag("scaling-list");
  TDecSbac::parseScalingList(scalingList);
  stream->readCloseTag("scaling-list");
}
