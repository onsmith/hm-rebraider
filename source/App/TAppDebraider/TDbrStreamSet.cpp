#include "TDbrStreamSet.h"


TDbrStreamSet::TDbrStreamSet() :
  bitstreams(TDbrStreamSet::NUM_STREAMS) {
}


TComOutputBitstream& TDbrStreamSet::operator[](Int i) {
  return bitstreams[i];
}


TComOutputBitstream& TDbrStreamSet::operator()(TDbrStreamSet::STREAM i) {
  return bitstreams[static_cast<Int>(i)];
}


Void TDbrStreamSet::clear() {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    bitstreams[i].clear();
  }
}
