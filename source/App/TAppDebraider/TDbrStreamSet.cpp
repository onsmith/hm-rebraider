#include "TDbrStreamSet.h"


const std::string TDbrStreamSet::STREAM_NAMES[] = {
  "CHUNKS",
  "NALU",
  "VPS",
  "SPS",
  "PPS",
  "SLICE",
  "DQP",
  "COEFF",
  "MVP_INDEX",
  "TQ_BYPASS_FLAG",
  "SKIP_FLAG",
  "MERGE_FLAG",
  "MERGE_INDEX",
  "SPLIT_FLAG",
  "PART_SIZE",
  "PRED_MODE",
  "IPCM",
  "TU_SUBDIV_FLAG",
  "QT_CBF",
  "INTRA_DIR_LUMA",
  "INTRA_DIR_CHROMA",
  "INTER_DIR",
  "REF_FRAME_INDEX",
  "MVD",
  "CROSS_COMP_PRED",
  "CHROMA_QP_ADJ",
  "TU_SKIP_FLAG",
  "SAO_PARAMS",
  "SAO_BLOCK_PARAMS",
  "LAST_SIG_XY",
  "SCALING_LIST",
  "RDPCM",
  "OTHER"
};


TDbrStreamSet::TDbrStreamSet() :
  bitstreams(TDbrStreamSet::NUM_STREAMS),
  ifstreams(TDbrStreamSet::NUM_STREAMS) {
}


TComInputBitstream& TDbrStreamSet::getBitstream(STREAM i) {
  return bitstreams[static_cast<Int>(i)];
}


TComInputBitstream& TDbrStreamSet::getBitstream(Int i) {
  return bitstreams[i];
}


std::ifstream& TDbrStreamSet::getIfstream(STREAM i) {
  return ifstreams[static_cast<Int>(i)];
}


std::ifstream& TDbrStreamSet::getIfstream(Int i) {
  return ifstreams[i];
}


const TComInputBitstream& TDbrStreamSet::getBitstream(STREAM i) const {
  return bitstreams[static_cast<Int>(i)];
}


const TComInputBitstream& TDbrStreamSet::getBitstream(Int i) const {
  return bitstreams[i];
}


const std::ifstream& TDbrStreamSet::getIfstream(STREAM i) const {
  return ifstreams[static_cast<Int>(i)];
}


const std::ifstream& TDbrStreamSet::getIfstream(Int i) const {
  return ifstreams[i];
}


Void TDbrStreamSet::xReadBytesIntoBitstream(STREAM stream, UInt numBytes) {
  if (numBytes == 0) {
    return;
  }
  TComInputBitstream&   bitstream = getBitstream(stream);
  std::ifstream&        ifstream  = getIfstream(stream);
  std::vector<uint8_t>& fifo      = bitstream.getFifo();
  fifo.resize(numBytes * sizeof(uint8_t));
  ifstream.read(
    reinterpret_cast<char*>(&fifo.front()),
    numBytes * sizeof(uint8_t)
  );
  bitstream.resetToStart();
}


Bool TDbrStreamSet::hasAnotherNalUnit() const {
  const std::ifstream& naluStream = getIfstream(STREAM::NALU);
  return !naluStream.eof() && !naluStream.fail();
}


Void TDbrStreamSet::xReadNextNalUnitHeader(InputNALUnit& nalu) {
  TComInputBitstream& naluStream = getBitstream(STREAM::NALU);
  UInt forbiddenZeroBit = naluStream.read(1); // forbidden zero bit
  assert(forbiddenZeroBit == 0);
  nalu.m_nalUnitType = static_cast<NalUnitType>(naluStream.read(6));
  nalu.m_nuhLayerId  = naluStream.read(6);
  nalu.m_temporalId  = naluStream.read(3);
}


UInt TDbrStreamSet::xReadNextChunkLength() {
  return getBitstream(STREAM::CHUNKS).read(16);
}


Void TDbrStreamSet::readNextNalUnit(InputNALUnit& nalu) {
  xReadBytesIntoBitstream(STREAM::CHUNKS, 2 * (NUM_STREAMS - 1));

  for (Int i = 1; i < NUM_STREAMS; i++) {
    xReadBytesIntoBitstream(static_cast<STREAM>(i), xReadNextChunkLength());
  }

  xReadNextNalUnitHeader(nalu);
}


Void TDbrStreamSet::open(const std::string& basename) {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    const std::string str = basename + '/' + STREAM_NAMES[i] + ".dat";

    ifstreams[i].open(
      str.c_str(),
      std::fstream::binary | std::fstream::in
    );

    if (!ifstreams[i].is_open() || !ifstreams[i].good()) {
      fprintf(
        stderr,
        "\nfailed to open bitstream file `%s' for writing\n",
        str.c_str()
      );
      exit(EXIT_FAILURE);
    }
  }
}


Bool TDbrStreamSet::fail() const {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    if (getIfstream(i).fail()) {
      return true;
    }
  }
  return false;
}
