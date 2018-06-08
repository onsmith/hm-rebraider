#include "TTraTop.h"


/**
 * Default constructor for transrater class
 */
TTraTop::TTraTop() :
  m_vps(nullptr) {
}


/**
 * Transcode a NAL unit without decoding
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu) {
  outputNalu = inputNalu;
}


/**
 * Transcode a decoded VPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComVPS& vps) {
  m_vps = &vps;
  xEncodeVPS(vps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded SPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSPS& sps) {
  *m_spsMap.allocatePS(sps.getSPSId()) = sps;
  xEncodeSPS(sps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded PPS NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComPPS& pps) {
  *m_ppsMap.allocatePS(pps.getPPSId()) = pps;
  xEncodePPS(pps, outputNalu.m_Bitstream);
}


/**
 * Transcode a decoded slice NAL unit
 */
Void TTraTop::transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSlice& slice) {
  TComPic* pEncPic = m_cpb.findByPOC(slice.getPic()->getPOC());
  xCopySlice(slice);
  xEncodeSlice(slice, outputNalu.m_Bitstream)
}


/**
 * Encode a VPS and write to bitstream
 */
Void TTraTop::xEncodeVPS(const TComVPS& vps, TComOutputBitstream& bitstream) {
  m_entropyEncoder.setEntropyCoder(&m_cavlcEncoder);
  m_entropyEncoder.setBitstream(&bitstream);
  m_entropyEncoder.encodeVPS(&vps);
}


/**
 * Encode a SPS and write to bitstream
 */
Void TTraTop::xEncodeSPS(const TComSPS& sps, TComOutputBitstream& bitstream) {
  m_entropyEncoder.setEntropyCoder(&m_cavlcEncoder);
  m_entropyEncoder.setBitstream(&bitstream);
  m_entropyEncoder.encodeSPS(&sps);
}


/**
 * Encode a PPS and write to bitstream
 */
Void TTraTop::xEncodePPS(const TComPPS& pps, TComOutputBitstream& bitstream) {
  m_entropyEncoder.setEntropyCoder(&m_cavlcEncoder);
  m_entropyEncoder.setBitstream(&bitstream);
  m_entropyEncoder.encodePPS(&pps);
}


/**
 * Encode a slice and write to bitstream
 */
Void TTraTop::xEncodeSlice(TComSlice& slice, TComOutputBitstream& bitstream) {
  // Write slice head to output bitstream
  m_entropyEncoder.setEntropyCoder(&m_cavlcEncoder);
  m_entropyEncoder.resetEntropy(&slice);
  m_entropyEncoder.setBitstream(&bitstream);
  slice.setEncCABACTableIdx(m_sliceEncoder.getEncCABACTableIdx());
  m_entropyEncoder.encodeSliceHeader(&slice);

  // Encode slice body to temporary bitstream
  UInt numBinsCoded = 0;
  TComOutputBitstream sliceBody;
  slice.setFinalized(true);
  slice.clearSubstreamSizes();
  m_sliceEncoder.encodeSlice(slice.getPic(), &sliceBody, numBinsCoded);
  
  // Write WPP entry point to output bitstream
  m_entropyEncoder.setEntropyCoder(&m_cavlcEncoder);
  m_entropyEncoder.setBitstream(&bitstream);
  m_entropyEncoder.encodeTilesWPPEntryPoint(&slice);

  // Write slice body to output bitstream
  bitstream.writeByteAlignment();
  bitstream.addSubstream(&sliceBody);
}
