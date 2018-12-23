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
  "RDPCM"
};


TDbrStreamSet::TDbrStreamSet() :
  bitstreams(TDbrStreamSet::NUM_STREAMS),
  ofstreams(TDbrStreamSet::NUM_STREAMS) {
}


TComOutputBitstream& TDbrStreamSet::getBitstream(STREAM i) {
  return bitstreams[static_cast<Int>(i)];
}


TComOutputBitstream& TDbrStreamSet::getBitstream(Int i) {
  return bitstreams[i];
}


std::ofstream& TDbrStreamSet::getOfstream(STREAM i) {
  return ofstreams[static_cast<Int>(i)];
}


std::ofstream& TDbrStreamSet::getOfstream(Int i) {
  return ofstreams[i];
}


Void TDbrStreamSet::byteAlign() {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    bitstreams[i].writeAlignZero();
  }
}


Void TDbrStreamSet::flush() {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    const UInt numBytes = bitstreams[i].getByteStreamLength();
    if (numBytes > 0) {
      ofstreams[i].write(
        reinterpret_cast<char*>(bitstreams[i].getByteStream()),
        numBytes
      );
    }
  }
}


Void TDbrStreamSet::clear() {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    bitstreams[i].clear();
  }
}


Void TDbrStreamSet::open(const std::string& basename) {
  for (Int i = 0; i < NUM_STREAMS; i++) {
    const std::string str = basename + STREAM_NAMES[i] + ".dat";

    ofstreams[i].open(
      str.c_str(),
      std::fstream::binary | std::fstream::out
    );

    if (!ofstreams[i].is_open() || !ofstreams[i].good()) {
      fprintf(
        stderr,
        "\nfailed to open bitstream file `%s' for writing\n",
        str.c_str()
      );
      exit(EXIT_FAILURE);
    }
  }
}


Void TDbrStreamSet::writeNalHeader(const InputNALUnit& nalu) {
  TComOutputBitstream& naluStream = getBitstream(STREAM::NALU);
  naluStream.write(0, 1); // forbidden zero bit
  naluStream.write(nalu.m_nalUnitType, 6);
  naluStream.write(nalu.m_nuhLayerId, 6);
  naluStream.write(nalu.m_temporalId, 3);
}


Void TDbrStreamSet::writeLengths() {
  TComOutputBitstream& chunks = getBitstream(STREAM::CHUNKS);
  for (Int i = 1; i < NUM_STREAMS; i++) {
    const UInt numBytes = bitstreams[i].getByteStreamLength();
    chunks.write(numBytes, 16);
  }
}
