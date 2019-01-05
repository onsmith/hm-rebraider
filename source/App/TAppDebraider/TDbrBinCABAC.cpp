#include "TDbrBinCABAC.h"


TDbrXmlReader* TDbrBinCABAC::getXmlReader() {
  return stream;
}

Void TDbrBinCABAC::setXmlReader(TDbrXmlReader* stream) {
  this->stream = stream;
}


Void TDbrBinCABAC::start() {
}


Void TDbrBinCABAC::finish() {
}


Void TDbrBinCABAC::decodeBin(UInt& ruiBin, ContextModel& rcCtxModel) {
  assert(stream != nullptr);
  ruiBin = stream->readValueTag("bin");
}


Void TDbrBinCABAC::decodeBinEP(UInt& ruiBin) {
  assert(stream != nullptr);
  ruiBin = stream->readValueTag("bin-ep");
}


Void TDbrBinCABAC::decodeBinsEP(UInt& ruiBins, Int numBins) {
  if (numBins > 0) {
    assert(stream != nullptr);
    ruiBins = stream->readValueTag("bins-ep");
  } else {
    ruiBins = 0;
  }
}


Void TDbrBinCABAC::decodeBinTrm(UInt& ruiBin) {
  assert(stream != nullptr);
  ruiBin = stream->readValueTag("bin-trm");
}


Void TDbrBinCABAC::xReadPCMCode(UInt uiLength, UInt& ruiCode) {
  if (uiLength > 0) {
    assert(stream != nullptr);
    ruiCode = stream->readValueTag("pcm-code");
  } else {
    ruiCode = 0;
  }
}
