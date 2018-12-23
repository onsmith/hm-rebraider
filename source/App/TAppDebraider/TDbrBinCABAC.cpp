#include "TDbrBinCABAC.h"


TComBitIf* TDbrBinCABAC::getOutputBitstream() {
  return SyntaxElementWriter::m_pcBitIf;
}

Void TDbrBinCABAC::setOutputBitstream(TComBitIf* outputBitstream) {
  SyntaxElementWriter::setBitstream(outputBitstream);
}


Void TDbrBinCABAC::decodeBin(UInt& ruiBin, ContextModel& rcCtxModel) {
  TDecBinCABAC::decodeBin(ruiBin, rcCtxModel);
  SyntaxElementWriter::xWriteFlag(ruiBin);
}


Void TDbrBinCABAC::decodeBinEP(UInt& ruiBin) {
  TDecBinCABAC::decodeBinEP(ruiBin);
  SyntaxElementWriter::xWriteFlag(ruiBin);
}


Void TDbrBinCABAC::decodeBinsEP(UInt& ruiBins, Int numBins) {
  TDecBinCABAC::decodeBinsEP(ruiBins, numBins);
  if (numBins > 0) {
    SyntaxElementWriter::xWriteCode(ruiBins, numBins);
  }
}


Void TDbrBinCABAC::decodeBinTrm(UInt& ruiBin) {
  TDecBinCABAC::decodeBinTrm(ruiBin);
  SyntaxElementWriter::xWriteFlag(ruiBin);
}


Void TDbrBinCABAC::xReadPCMCode(UInt uiLength, UInt& ruiCode) {
  TDecBinCABAC::xReadPCMCode(uiLength, ruiCode);
  if (uiLength > 0) {
    SyntaxElementWriter::xWriteCode(ruiCode, uiLength);
  }
}
