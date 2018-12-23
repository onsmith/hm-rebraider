#include "TRbrStreamSet.h"


TRbrStreamSet::TRbrStreamSet() :
  bitstreams(TRbrStreamSet::NUM_STREAMS) {
}


TComInputBitstream& TRbrStreamSet::getStream(TRbrStreamSet::STREAM i) {
  return bitstreams[static_cast<Int>(i)];
}


TComInputBitstream& TRbrStreamSet::operator[](Int i) {
  return bitstreams[i];
}


TComInputBitstream& TRbrStreamSet::operator()(TRbrStreamSet::STREAM i) {
  return getStream(i);
}
