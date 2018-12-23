#include "TRbrBinVLC.h"


// Sets the underlying TComInputBitstream
Void TRbrBinVLC::init(TComInputBitstream* pcTComBitstream) {
  bitstream = pcTComBitstream;
}


// Removes the underlying TComInputBitstream
Void TRbrBinVLC::uninit() {
  bitstream = nullptr;
}


// Initializes the internal state
// Note: This method does nothing since there is no internal state
Void TRbrBinVLC::start() {
  // Intentionally empty
}


// Writes any buffered bits to the output stream, injecting bits as necessary
//   to unambiguously specify the correct arithmetic code interval
// Note: This method does nothing since this class is not actually a cabac
Void TRbrBinVLC::finish() {
  // Intentionally empty
}


// Copies internal state from another decoder object
// Note: This method does nothing since there is no internal state
Void copyState(const TDecBinIf* pcTDecBinIf) {
  // Intentionally empty
}


// Reads a PCM code
Void TRbrBinVLC::xReadPCMCode(UInt uiLength, UInt& ruiCode) {
  bitstream->read(uiLength, ruiCode);
}


// Decodes a single bit
Void TRbrBinVLC::decodeBin(UInt& ruiBin, ContextModel& rcCtxModel) {
  bitstream->read(1, ruiBin);
}


// Decodes a single bit when a zero and a one are equiprobable (ep)
Void TRbrBinVLC::decodeBinEP(UInt& ruiBin) {
  bitstream->read(1, ruiBin);
}


// Decodes multiple bits when zeros and ones are equiprobable (ep)
Void TRbrBinVLC::decodeBinsEP(UInt& ruiBins, Int numBins) {
  bitstream->read(numBins, ruiBins);
}


// Aligns the cabac interval range
// Note: This method does nothing since this class is not actually a cabac
Void TRbrBinVLC::align() {
  // Intentionally empty
}


// Decodes a terminating character from the bitstream
Void TRbrBinVLC::decodeBinTrm(UInt& ruiBin) {
  bitstream->read(1, ruiBin);
}


// Copies internal state from another decoder object
// Note: This method does nothing since there is no internal state
Void TRbrBinVLC::copyState(const TDecBinIf* pcTDecBinIf) {
  // Intentionally empty
}


// This method throws an error with this implementation of TDecBinIf, since
//   it isn't backed by a TDecBinCABAC.
TDecBinCABAC* TRbrBinVLC::getTDecBinCABAC() {
  assert(0);
}


// This method throws an error with this implementation of TDecBinIf, since
//   it isn't backed by a TDecBinCABAC.
const TDecBinCABAC* TRbrBinVLC::getTDecBinCABAC() const {
  assert(0);
}
