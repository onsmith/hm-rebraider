#include "TDbrDummyBitstream.h"



UInt TDbrDummyBitstream::readByteAlignment() {
  return 1;
}


TComInputBitstream* TDbrDummyBitstream::extractSubstream(UInt uiNumBits) {
  return this;
}
