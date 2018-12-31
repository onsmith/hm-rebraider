#include "TDbrBinCABAC.h"


TComInputBitstream* TDbrBinCABAC::getInputBitstream() {
  return SyntaxElementParser::m_pcBitstream;
}


Void TDbrBinCABAC::setInputBitstream(TComInputBitstream* outputBitstream) {
  SyntaxElementParser::m_pcBitstream = outputBitstream;
}


Void TDbrBinCABAC::start() {
}


Void TDbrBinCABAC::finish() {
}


Void TDbrBinCABAC::decodeBin(UInt& ruiBin, ContextModel& rcCtxModel) {
  SyntaxElementParser::xReadFlag(ruiBin);
}


Void TDbrBinCABAC::decodeBinEP(UInt& ruiBin) {
  SyntaxElementParser::xReadFlag(ruiBin);
}


Void TDbrBinCABAC::decodeBinsEP(UInt& ruiBins, Int numBins) {
  if (numBins > 0) {
    SyntaxElementParser::xReadCode(numBins, ruiBins);
  }
}


Void TDbrBinCABAC::decodeBinTrm(UInt& ruiBin) {
  SyntaxElementParser::xReadFlag(ruiBin);
}


Void TDbrBinCABAC::xReadPCMCode(UInt uiLength, UInt& ruiCode) {
  if (uiLength > 0) {
    SyntaxElementParser::xReadCode(uiLength, ruiCode);
  }
}
