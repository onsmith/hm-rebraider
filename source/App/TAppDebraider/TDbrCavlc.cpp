#include "TDbrCavlc.h"


TDbrXmlReader* TDbrCavlc::getXmlReader() {
  return stream;
}

Void TDbrCavlc::setXmlReader(TDbrXmlReader* stream) {
  this->stream = stream;
}


Void TDbrCavlc::parseShortTermRefPicSet(TComSPS* sps, TComReferencePictureSet* rps, Int idx) {
  assert(stream != nullptr);
  stream->readOpenTag("short-term-ref-pic-set");
  TDecCavlc::parseShortTermRefPicSet(sps, rps, idx);
  stream->readCloseTag("short-term-ref-pic-set");
}


Void TDbrCavlc::parseVPS(TComVPS* pcVPS) {
  assert(stream != nullptr);
  stream->readOpenTag("vps");
  TDecCavlc::parseVPS(pcVPS);
  stream->readCloseTag("vps");
}


Void TDbrCavlc::parseSPS(TComSPS* pcSPS) {
  assert(stream != nullptr);
  stream->readOpenTag("sps");
  TDecCavlc::parseSPS(pcSPS);
  stream->readCloseTag("sps");
}


Void TDbrCavlc::parsePPS(TComPPS* pcPPS) {
  assert(stream != nullptr);
  stream->readOpenTag("pps");
  TDecCavlc::parsePPS(pcPPS);
  stream->readCloseTag("pps");
}


Void TDbrCavlc::parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC) {
  assert(stream != nullptr);
  stream->readOpenTag("slice-header");
  setBitstream(&dummyStream);
  TDecCavlc::parseSliceHeader(pcSlice, parameterSetManager, prevTid0POC);
  stream->readCloseTag("slice-header");
}


Void TDbrCavlc::parseTerminatingBit(UInt& ruilsLast) {
  assert(stream != nullptr);
  stream->readOpenTag("terminating-bit");
  TDecCavlc::parseTerminatingBit(ruilsLast);
  stream->readCloseTag("terminating-bit");
}


Void TDbrCavlc::parseRemainingBytes(Bool noTrailingBytesExpected) {
  assert(stream != nullptr);
  stream->readOpenTag("remaining-bytes");
  TDecCavlc::parseRemainingBytes(noTrailingBytesExpected);
  stream->readCloseTag("remaining-bytes");
}


Void TDbrCavlc::parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  assert(stream != nullptr);
  stream->readOpenTag("delta-qp");
  TDecCavlc::parseDeltaQP(pcCU, uiAbsPartIdx, uiDepth);
  stream->readCloseTag("delta-qp");
}


Void TDbrCavlc::xReadCode(UInt length, UInt& val) {
  val = stream->readValueTag("code");
}


Void TDbrCavlc::xReadUvlc(UInt& val) {
  val = stream->readValueTag("uvlc");
}


Void TDbrCavlc::xReadSvlc(Int& val) {
  val = stream->readValueTag("svlc");
}


Void TDbrCavlc::xReadFlag(UInt& val) {
  val = stream->readValueTag("flag");
}


Void TDbrCavlc::xReadRbspTrailingBits() {
  stream->readValueTag("rbsp-trailing-bits");
}
