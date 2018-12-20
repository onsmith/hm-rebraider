#include "TDbrCabac.h"


// Sets the underlying bitstream
Void TDbrCabac::init(TComBitIf* pcTComBitIf) {
  bitstream = pcTComBitIf;
}


// Removes the underlying bitstream
Void TDbrCabac::uninit() {
  bitstream = nullptr;
}


// Initializes the cabac internal state
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::start() {
  // Intentionally empty
}


// Writes any buffered bits to the output stream, injecting bits as necessary to
//   unambiguously specify the correct arithmetic code interval
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::finish() {
  // Intentionally empty
}


// Copies internal cabac state from another cabac encoder class
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::copyState(const TEncBinIf* pcTEncBinIf) {
  // Intentionally empty
}


// Writes a terminating character to the bitstream, finishes the stream, and
//   resets the stream
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::flush() {
  // Intentionally empty
}


// Resets the cabac internal state
Void TDbrCabac::resetBac() {
  start();
}


// Encodes PCM alignment zero bits
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::encodePCMAlignBits() {
  // Intentionally empty
}


// Writes a PCM code
Void TDbrCabac::xWritePCMCode(UInt uiCode, UInt uiLength) {
  bitstream->write(uiCode, uiLength);
}


// Resets the internal bit buffer
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::resetBits() {
  // Intentionally empty
}


// Gets the number of bits written to the bitstream
UInt TDbrCabac::getNumWrittenBits() {
  return bitstream->getNumberOfWrittenBits();
}


// Encodes a single bit
Void TDbrCabac::encodeBin(UInt uiBin, ContextModel& /* rcCtxModel */) {
  bitstream->write(uiBin, 1);
}


// Encodes a single bit such that a zero and a one are equiprobable (ep)
Void TDbrCabac::encodeBinEP(UInt uiBin) {
  bitstream->write(uiBin, 1);
}


// Encodes multiple bits such that zeros and ones are equiprobable (ep)
Void TDbrCabac::encodeBinsEP(UInt uiBins, Int numBins) {
  bitstream->write(uiBins, numBins);
}

// Encodes a terminating character to the bitstream
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::encodeBinTrm(UInt uiBin) {
  // Intentionally empty
}


// Aligns the cabac interval range
// Note: This method does nothing since this class is not actually a cabac
Void TDbrCabac::align() {
  // Intentionally empty
}
