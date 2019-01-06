/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */


/**
 *  \file     TAppDbrTop.cpp
 *  \project  TAppDebraider
 *  \brief    Debraider application class implementation
 */


#include <list>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppDbrTop.h"
#include "TDbrXmlReader.h"

#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"

#include "TLibEncoder/AnnexBwrite.h"

#if RExt__DECODER_DEBUG_BIT_STATISTICS
#include "TLibCommon/TComCodingStatistics.h"
#endif


//! \ingroup TAppDebraider
//! \{


/**
 * Default constructor
 */
TAppDbrTop::TAppDbrTop() :
  m_lastOutputPOC(-MAX_INT) {
}


/**
 * Gets the number of decoding errors detected
 */
UInt TAppDbrTop::numDecodingErrorsDetected() const {
  return m_decoder.getNumberOfChecksumErrorsDetected();
}


/**
 * Main debraid processing function. Performs the following steps:
 *   1. Opens the input and output bitstream files
 *   2. Creates encoder and decoder objects
 *   3. Initializes encoder and decoder using defined configuration
 *   4. Until the end of the bitstream, decode and debraid video frames
 *   5. Destroy the internal encoder and decoder objects
 */
Void TAppDbrTop::debraid() {
  // Picture order count
  Int poc = -1;

  // Pointer to decoded picture buffer
  TComList<TComPic*>* dpb = nullptr;

  // Open input debraided bitstreams for reading source video
  std::ifstream inputStream;
  xOpenInputStream(inputStream);

  // Open output h265 bitstream for writing rebraided video
  std::ofstream outputStream;
  xOpenOutputStream(outputStream);

  // Create xml reader object
  TDbrXmlReader xmlReader(inputStream);

  // Reset decoded yuv output stream
  if (m_decodedYUVOutputStream.isOpen()) {
    m_decodedYUVOutputStream.close();
  }

  // Copy decoding configuration to the decoder
  {
    TDbrCavlc&    cavlcDecoder = m_decoder.getCavlcDecoder();
    TDbrSbac&     sbacDecoder  = m_decoder.getSbacDecoder();
    TDbrBinCABAC& cabacReader  = m_decoder.getCabacReader();
    cavlcDecoder.setXmlReader(&xmlReader);
    sbacDecoder.setXmlReader(&xmlReader);
    cabacReader.setXmlReader(&xmlReader);
    sbacDecoder.setCabacReader(&cabacReader);
    xConfigDecoder();
  }

  // Adjust the last output POC index if seeking is required before decoding
  m_lastOutputPOC += m_iSkipFrame;

  // TODO: Write description of this variable's purpose
  Bool loopFiltered = false;

  // Main decoder loop
  while (!inputStream.fail()) {
    // Remember the starting position of the input bitstream file cursor
    /* Note: This serves to work around a design fault in the decoder, whereby
     * the process of reading a new slice that is the first slice of a new frame
     * requires the TDecTop::decode() method to be called again with the same
     * nal unit. */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
    auto backupStats(TComCodingStatistics::GetStatistics());
    const streampos initialPositionInInputBitstream =
      inputStream.tellg() - streampos(inputByteStream.GetNumBufferedBytes());
#else
    const streampos initialPositionInInputBitstream = inputStream.tellg();
#endif

    // Read one NAL unit from input bitstream
    InputNALUnit nalu;
    xmlReader.readOpenTag(nalu);

    // If it's a parameter set nal unit, copy the nal unit contents into the
    //   bitstream. This is so the parameter set manager knows whether the
    //   parameter set is new or if it is a duplicate.
    if (xIsParameterSetNalUnit(nalu.m_nalUnitType)) {
      xCopyStringIntoNaluBitstream(xmlReader.peekNextCompleteTag("nalu"), nalu);
    }

    // Create an output NAL unit to store the reencoded data
    OutputNALUnit reencodedNalu(
      nalu.m_nalUnitType,
      nalu.m_temporalId,
      nalu.m_nuhLayerId
    );

    // True if a new picture is found within the current NAL unit
    Bool wasNewPictureFound = false;

    // Determine if the nal unit comes from a temporal ID marked for decoding
    Bool willDecodeTemporalId = (
      m_iMaxTemporalLayer < 0 || nalu.m_temporalId <= m_iMaxTemporalLayer
    );

    // Determine if the nal unit should be decoded
    Bool willDecodeNalUnit = (
      willDecodeTemporalId &&
      xWillDecodeLayer(nalu.m_nuhLayerId) &&
      !xIsNalUnitBodyRawEncoded(nalu.m_nalUnitType)
    );

    // Call decoding function
    if (willDecodeNalUnit) {
      wasNewPictureFound = m_decoder.decode(nalu, m_iSkipFrame, m_lastOutputPOC);
      if (!wasNewPictureFound) {
        xEncodeUnit(nalu, reencodedNalu);
      }
    } else {
      xReadRawNaluBody(nalu, xmlReader);
      m_transcoder.transcode(nalu, reencodedNalu);
    }

    // If a new picture was found in the current NAL unit, rewind the input
    //   bitstream file cursor
    if (wasNewPictureFound) {
      inputStream.clear();
      /* initialPositionInInputBitstream points to the current nalunit
       * payload[1] due to the need for the annexB parser to read three extra
       * bytes. [1] except for the first NAL unit in the file (but
       * wasNewPictureFound doesn't happen then)
       */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
      inputStream.seekg(initialPositionInInputBitstream);
      inputByteStream.reset();
      TComCodingStatistics::SetStatistics(backupStats);
#else
      inputStream.seekg(initialPositionInInputBitstream);
#endif
    }

    // Add the nal unit to the current access unit
    if (!wasNewPictureFound) {
      m_currentAccessUnit.push_back(new NALUnitEBSP(reencodedNalu));

      // TODO: Technically, it's probably better to wait for a new access unit
      //   signal before flushing the access unit, but for now we will flush
      xFlushAccessUnit(outputStream);

      // Flush access unit
      //if (xIsFirstNalUnitOfNewAccessUnit(nalu)) {
      //  xFlushAccessUnit(outputStream);
      //}

      // Consume nal unit closing xml tag
      xmlReader.readCloseTag("nalu");
      xmlReader.consumeEmptyLines();
    }

    // True if a picture was finished while decoding the current NAL unit
    Bool wasPictureFinished =
      (wasNewPictureFound || inputStream.fail() || nalu.isEOS());

    // Apply in-loop filters to reconstructed picture
    if (wasPictureFinished && !m_decoder.getFirstSliceInSequence()) {
      if (!loopFiltered || !inputStream.fail()) {
        m_decoder.executeLoopFilters(poc, dpb);
      }
      loopFiltered = nalu.isEOS();
      if (nalu.isEOS()) {
        m_decoder.setFirstSliceInSequence(true);
      }
    } else if (wasPictureFinished && m_decoder.getFirstSliceInSequence()) {
      m_decoder.setFirstSliceInPicture(true);
    }

    if (dpb != nullptr) {
      // Open YUV reconstruction output file
      if (!m_reconFileName.empty() && !m_decodedYUVOutputStream.isOpen()) {
        // Set the output pixel bit depths to be the same as the bitdepths used
        //   by the first reconstructed picture
        const BitDepths& bitDepths =
          dpb->front()->getPicSym()->getSPS().getBitDepths();
        xSetOutputBitDepths(bitDepths);

        // Open YUV reconstruction output file
        m_decodedYUVOutputStream.open(
          m_reconFileName,
          true,
          m_outputBitDepth,
          m_outputBitDepth,
          bitDepths.recon
        );
      }

      // Write reconstructed frames to output
      if (wasNewPictureFound) {
        xDisplayDecodedFrames(dpb);
      }

      if ((wasNewPictureFound || nalu.isCRASlice()) && m_decoder.getNoOutputPriorPicsFlag()) {
        m_decoder.checkNoOutputPriorPics(dpb);
        m_decoder.setNoOutputPriorPicsFlag(false);
      }

      // Flush output frames when IDR or BLA NAL units are encountered
      // TODO: Why don't we include CRA NAL units here?
      if (wasNewPictureFound && (nalu.isIDRSlice() || nalu.isBLASlice())) {
        xFlushPictureBuffer(dpb);
      }

      // Handle end of sequence (EOS) NAL units
      if (nalu.isEOS()) {
        xDisplayDecodedFrames(dpb);
        m_decoder.setFirstSliceInPicture(false);
      }

      // Write reconstructed frames to output
      //   Note: This is for additional bumping as defined in C.5.2.3
      if (!wasNewPictureFound && nalu.m_nalUnitType >= NAL_UNIT_CODED_SLICE_TRAIL_N && nalu.m_nalUnitType <= NAL_UNIT_RESERVED_VCL31) {
        xDisplayDecodedFrames(dpb);
      }
    }
  }

  // Send any remaining pictures to output
  xFlushPictureBuffer(dpb);

  // Free decoder resources
  m_decoder.deletePicBuffer();
  m_decoder.setDecodedSEIMessageOutputStream(nullptr);
}


/**
 * Helper method to open an ifstream for reading the source hevc bitstream
 */
Void TAppDbrTop::xOpenInputStream(ifstream& stream) const {
  stream.open(
    m_inputFileName.c_str(),
    ifstream::in | ifstream::binary
  );

  if (!stream.is_open() || !stream.good()) {
    fprintf(
      stderr,
      "\nfailed to open bitstream file `%s' for reading\n",
      m_inputFileName.c_str()
    );
    exit(EXIT_FAILURE);
  }
}


/**
 * Helper method to open an ofstream for writing the transrated hevc bitstream
 */
Void TAppDbrTop::xOpenOutputStream(ofstream& stream) const {
  stream.open(
    m_outputFileName.c_str(),
    fstream::binary | fstream::out
  );

  if (!stream.is_open() || !stream.good()) {
    fprintf(
      stderr,
      "\nfailed to open bitstream file `%s' for writing\n",
      m_outputFileName.c_str()
    );
    exit(EXIT_FAILURE);
  }
}


/**
 * Overwrites the default configuration for output bit depth
 */
Void TAppDbrTop::xSetOutputBitDepths(const BitDepths& bitDepths) {
  for (UInt c = 0; c < MAX_NUM_CHANNEL_TYPE; c++) {
    if (m_outputBitDepth[c] == 0) {
      m_outputBitDepth[c] = bitDepths.recon[c];
    }
  }
}


/**
 * Transfers the current configuration to the encoder object
 */
Void TAppDbrTop::xConfigTranscoder() {
}


/**
 * Transfers the current configuration to the decoder object
 */
Void TAppDbrTop::xConfigDecoder() {
  // Reset decoder
  m_decoder.destroy();
  m_decoder.create();
  m_decoder.init();

  // Enable/disable hashing reconstructed frames to verify correct decoding
  m_decoder.setDecodedPictureHashSEIEnabled(m_decodedPictureHashSEIEnabled);

#if MCTS_ENC_CHECK
  // Enable/disable checking motion constrained tile set (MCTS)
  m_decoder.setTMctsCheckEnabled(m_tmctsCheck);
#endif

#if O0043_BEST_EFFORT_DECODING
  m_decoder.setForceDecodeBitDepth(m_forceDecodeBitDepth);
#endif
}


/**
 * Writes reconstructed frames in the decoded picture buffer to the output
 *   bitstream
 *
 * \param dpb  Decoded picture buffer, sorted in increasing POC order
 */
Void TAppDbrTop::xDisplayDecodedFrames(TComList<TComPic*>* dpb) {
  if (dpb == nullptr || dpb->empty()) {
    return;
  }

  const TComSPS* sps              = &(dpb->front()->getPicSym()->getSPS());
        UInt     maxTemporalLayer = sps->getMaxTLayers();

  // TODO: Write description for this variable
  UInt maxNumReorderedPics;

  // TODO: Write description for this variable
  UInt maxNumBufferedPics;

  // Find correct values for maxNumReorderedPics and maxNumBufferedPics in SPS
  if (m_iMaxTemporalLayer == -1 || maxTemporalLayer <= m_iMaxTemporalLayer) {
    maxNumReorderedPics = sps->getNumReorderPics(maxTemporalLayer - 1);
    maxNumBufferedPics  = sps->getMaxDecPicBuffering(maxTemporalLayer - 1);
  } else {
    maxNumReorderedPics = sps->getNumReorderPics(m_iMaxTemporalLayer);
    maxNumBufferedPics  = sps->getMaxDecPicBuffering(m_iMaxTemporalLayer);
  }

  // Counts the number of pictures in the decoded picture buffer (DBP)
  Int numDecodedBufferedPictures = 0;

  // Counts the number of pictures that have been decoded but not yet displayed
  Int numPicsNotYetDisplayed = 0;

  // Loop through picture buffer to calculate numDecodedBufferedPictures and
  //   numPicsNotYetDisplayed
  for (auto p = dpb->begin(); p != dpb->end(); p++) {
    TComPic* pic = *p;
    if (pic->getOutputMark() && pic->getPOC() > m_lastOutputPOC) {
      numPicsNotYetDisplayed++;
      numDecodedBufferedPictures++;
    } else if (pic->getSlice(0)->isReferenced()) {
      numDecodedBufferedPictures++;
    }
  }

  auto iterPic = dpb->begin();

  if (numPicsNotYetDisplayed > 2) {
    iterPic++;
  }

  TComPic* pcPic = *iterPic;
  
  // Interlaced field output
  if (numPicsNotYetDisplayed > 2 && pcPic->isField()) {
    auto endPic = dpb->end();
    endPic--;
    iterPic = dpb->begin();

    while (iterPic != endPic) {
      TComPic* pcPicTop = *iterPic;
      iterPic++;
      TComPic* pcPicBottom = *iterPic;

      Bool areFieldsMarkedForOutput =
        (pcPicTop->getOutputMark() && pcPicBottom->getOutputMark());
      Bool areFieldsAdjacent =
        (pcPicBottom->getPOC() == pcPicTop->getPOC() + 1);
      Bool areFieldsCoupled =
        (areFieldsAdjacent && pcPicTop->getPOC() % 2 == 0);
      Bool areFieldsNextForOutput = 
        (pcPicTop->getPOC() == m_lastOutputPOC + 1 || m_lastOutputPOC < 0);
      Bool isBufferFull = (
        numPicsNotYetDisplayed > maxNumReorderedPics ||
        numDecodedBufferedPictures > maxNumBufferedPics
      );

      // Write frame to output file
      if (areFieldsMarkedForOutput &&
          isBufferFull &&
          areFieldsCoupled &&
          areFieldsNextForOutput) {

        numPicsNotYetDisplayed = numPicsNotYetDisplayed - 2;

        if (!m_reconFileName.empty()) {
          Bool display = true;
          if (m_decodedNoDisplaySEIEnabled) {
            SEIMessages noDisplay = getSeisByType(pcPic->getSEIs(), SEI::NO_DISPLAY);
            const SEINoDisplay *nd = (noDisplay.size() > 0) ? (SEINoDisplay*) *(noDisplay.begin()) : NULL;
            if (nd != NULL && nd->m_noDisplay) {
              display = false;
            }
          }

          if (display) {
            xWriteFrameToOutput(pcPicTop, pcPicBottom);
          }
        }

        // Update last displayed picture index
        m_lastOutputPOC = pcPicBottom->getPOC();

        // Mark non-referenced fields so they can be reused by the decoder
        if (!pcPicTop->getSlice(0)->isReferenced() && pcPicTop->getReconMark()) {
          pcPicTop->setReconMark(false);
          pcPicTop->getPicYuvRec()->setBorderExtension(false);
        }

        // Mark non-referenced fields so they can be reused by the decoder
        if (!pcPicBottom->getSlice(0)->isReferenced() && pcPicBottom->getReconMark()) {
          pcPicBottom->setReconMark(false);
          pcPicBottom->getPicYuvRec()->setBorderExtension(false);
        }

        pcPicTop->setOutputMark(false);
        pcPicBottom->setOutputMark(false);
      }
    }

  // Frame output
  } else if (!pcPic->isField()) {
    iterPic = dpb->begin();

    for (auto p = dpb->begin(); p != dpb->end(); p++) {
      TComPic* pic = *p;

      Bool isBufferFull = (
        numPicsNotYetDisplayed     > maxNumReorderedPics ||
        numDecodedBufferedPictures > maxNumBufferedPics
      );

      // Write frame to output file
      if (pic->getOutputMark() && isBufferFull) {
        numPicsNotYetDisplayed--;

        if (!pic->getSlice(0)->isReferenced()) {
          numDecodedBufferedPictures--;
        }

        xWriteFrameToOutput(pic);

        // Update last displayed picture index
        m_lastOutputPOC = pic->getPOC();

        // Mark the frame so it can be reused by the decoder
        if (!pic->getSlice(0)->isReferenced() && pic->getReconMark()) {
          pic->setReconMark(false);
          pic->getPicYuvRec()->setBorderExtension(false);
        }

        pic->setOutputMark(false);
      }
    }
  }
}


/**
 * Writes reconstructed frames in the decoded picture buffer to the output
 *   bitstream and flushes the buffer
 *
 * \param dpb  Decoded picture buffer, sorted in increasing POC order
 */
Void TAppDbrTop::xFlushPictureBuffer(TComList<TComPic*>* dpb) {
  if (dpb == nullptr || dpb->empty()) {
    return;
  }

  // Interlaced field output
  if (dpb->front()->isField()) {
    auto     iterPic     = dpb->begin();
    auto     endPic      = dpb->end();
    TComPic* pcPic       = *iterPic;
    TComPic* pcPicTop;
    TComPic* pcPicBottom = NULL;
    endPic--;
    while (iterPic != endPic) {
      pcPicTop = *iterPic;
      iterPic++;
      pcPicBottom = *iterPic;

      Bool areFieldsMarkedForOutput =
        (pcPicTop->getOutputMark() && pcPicBottom->getOutputMark());
      Bool areFieldsAdjacent =
        (pcPicBottom->getPOC() == pcPicTop->getPOC() + 1);
      Bool areFieldsCoupled =
        (areFieldsAdjacent && pcPicTop->getPOC() % 2 == 0);

      // Write to output file
      if (areFieldsMarkedForOutput && areFieldsCoupled) {
        xWriteFrameToOutput(pcPicTop, pcPicBottom);

        // update POC of display order
        m_lastOutputPOC = pcPicBottom->getPOC();

        // erase non-referenced picture in the reference picture list
        if (!pcPicTop->getSlice(0)->isReferenced() && pcPicTop->getReconMark()) {
          pcPicTop->setReconMark(false);

          // mark it should be extended later
          pcPicTop->getPicYuvRec()->setBorderExtension(false);
        }

        if (!pcPicBottom->getSlice(0)->isReferenced() && pcPicBottom->getReconMark()) {
          pcPicBottom->setReconMark(false);

          // mark it should be extended later
          pcPicBottom->getPicYuvRec()->setBorderExtension(false);
        }

        pcPicTop->setOutputMark(false);
        pcPicBottom->setOutputMark(false);

        if (pcPicTop) {
          pcPicTop->destroy();
          delete pcPicTop;
          pcPicTop = NULL;
        }
      }
    }

    if (pcPicBottom) {
      pcPicBottom->destroy();
      delete pcPicBottom;
      pcPicBottom = NULL;
    }
  }

  // Frame output
  else {
    for (auto p = dpb->begin(); p != dpb->end(); p++) {
      TComPic* pic = *p;

      if (pic->getOutputMark()) {
        // Write to output file
        xWriteFrameToOutput(pic);

        // Update POC of display order
        m_lastOutputPOC = pic->getPOC();

        // Erase non-referenced picture from the reference picture list
        if (!pic->getSlice(0)->isReferenced() && pic->getReconMark()) {
          pic->setReconMark(false);

          // Mark it should be extended later
          pic->getPicYuvRec()->setBorderExtension(false);
        }

        pic->setOutputMark(false);
      }

      // Delete picture
      if (pic != NULL) {
        pic->destroy();
        delete pic;
        pic = NULL;
      }
    }
  }

  // Clear decoded picture buffer
  dpb->clear();

  // Reset last displayed index
  m_lastOutputPOC = -MAX_INT;
}


/**
 * Writes a raw reconstructed frame to the output bitstream.
 */
Void TAppDbrTop::xWriteFrameToOutput(TComPic* frame) {
  if (m_decodedYUVOutputStream.isOpen()) {
    const Window &conf    = frame->getConformanceWindow();
    const Window  defDisp = (m_respectDefDispWindow ? frame->getDefDisplayWindow() : Window());

    m_decodedYUVOutputStream.write(
      frame->getPicYuvRec(),
      m_outputColourSpaceConvert,
      conf.getWindowLeftOffset()   + defDisp.getWindowLeftOffset(),
      conf.getWindowRightOffset()  + defDisp.getWindowRightOffset(),
      conf.getWindowTopOffset()    + defDisp.getWindowTopOffset(),
      conf.getWindowBottomOffset() + defDisp.getWindowBottomOffset(),
      NUM_CHROMA_FORMAT,
      m_bClipOutputVideoToRec709Range
    );
  }
}


/**
 * Writes a raw reconstructed interlaced frame to the output bitstream.
 */
Void TAppDbrTop::xWriteFrameToOutput(TComPic* field1, TComPic* field2) {
  if (m_decodedYUVOutputStream.isOpen()) {
    const Window& conf    = field1->getConformanceWindow();
    const Window  defDisp = (m_respectDefDispWindow ? field1->getDefDisplayWindow() : Window());

    m_decodedYUVOutputStream.write(
      field1->getPicYuvRec(),
      field2->getPicYuvRec(),
      m_outputColourSpaceConvert,
      conf.getWindowLeftOffset()   + defDisp.getWindowLeftOffset(),
      conf.getWindowRightOffset()  + defDisp.getWindowRightOffset(),
      conf.getWindowTopOffset()    + defDisp.getWindowTopOffset(),
      conf.getWindowBottomOffset() + defDisp.getWindowBottomOffset(),
      NUM_CHROMA_FORMAT,
      field1->isTopField()
    );
  }
}


/**
 * Checks whether a given layerId should be decoded.
 */
Bool TAppDbrTop::xWillDecodeLayer(Int layerId) const {
  if (xWillDecodeAllLayers()) {
    return true;
  }

  for (auto p = m_targetDecLayerIdSet.begin(); p != m_targetDecLayerIdSet.end(); p++) {
    if (layerId == *p) {
      return true;
    }
  }

  return false;
}


/**
 * Checks whether all layerIds should be decoded.
 */
Bool TAppDbrTop::xWillDecodeAllLayers() const {
  return (m_targetDecLayerIdSet.size() == 0);
}


/**
 * Encodes a decoded NAL unit
 */
Void TAppDbrTop::xEncodeUnit(const InputNALUnit& sourceNalu, OutputNALUnit& encodedNalu) {
  switch (sourceNalu.m_nalUnitType) {
    case NAL_UNIT_VPS:
      m_transcoder.transcode(sourceNalu, encodedNalu, *m_decoder.getVPS());
      break;

    case NAL_UNIT_SPS:
      m_transcoder.transcode(sourceNalu, encodedNalu, *m_decoder.getSPS());
      break;

    case NAL_UNIT_PPS:
      m_transcoder.transcode(sourceNalu, encodedNalu, *m_decoder.getPPS());
      break;

    case NAL_UNIT_PREFIX_SEI:
      m_transcoder.transcode(sourceNalu, encodedNalu);
      break;

    case NAL_UNIT_SUFFIX_SEI:
      m_transcoder.transcode(sourceNalu, encodedNalu);
      break;

    case NAL_UNIT_CODED_SLICE_TRAIL_R:
    case NAL_UNIT_CODED_SLICE_TRAIL_N:
    case NAL_UNIT_CODED_SLICE_TSA_R:
    case NAL_UNIT_CODED_SLICE_TSA_N:
    case NAL_UNIT_CODED_SLICE_STSA_R:
    case NAL_UNIT_CODED_SLICE_STSA_N:
    case NAL_UNIT_CODED_SLICE_BLA_W_LP:
    case NAL_UNIT_CODED_SLICE_BLA_W_RADL:
    case NAL_UNIT_CODED_SLICE_BLA_N_LP:
    case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
    case NAL_UNIT_CODED_SLICE_IDR_N_LP:
    case NAL_UNIT_CODED_SLICE_CRA:
    case NAL_UNIT_CODED_SLICE_RADL_N:
    case NAL_UNIT_CODED_SLICE_RADL_R:
    case NAL_UNIT_CODED_SLICE_RASL_N:
    case NAL_UNIT_CODED_SLICE_RASL_R:
      {
        TComSlice& slice = *m_decoder.getCurSlice();
        slice.setPic(m_decoder.getCurPic());
        m_transcoder.transcode(sourceNalu, encodedNalu, slice);
      }
      break;

    case NAL_UNIT_EOS:

    case NAL_UNIT_ACCESS_UNIT_DELIMITER:

    case NAL_UNIT_EOB:

    case NAL_UNIT_FILLER_DATA:

    case NAL_UNIT_RESERVED_VCL_N10:
    case NAL_UNIT_RESERVED_VCL_R11:
    case NAL_UNIT_RESERVED_VCL_N12:
    case NAL_UNIT_RESERVED_VCL_R13:
    case NAL_UNIT_RESERVED_VCL_N14:
    case NAL_UNIT_RESERVED_VCL_R15:

    case NAL_UNIT_RESERVED_IRAP_VCL22:
    case NAL_UNIT_RESERVED_IRAP_VCL23:

    case NAL_UNIT_RESERVED_VCL24:
    case NAL_UNIT_RESERVED_VCL25:
    case NAL_UNIT_RESERVED_VCL26:
    case NAL_UNIT_RESERVED_VCL27:
    case NAL_UNIT_RESERVED_VCL28:
    case NAL_UNIT_RESERVED_VCL29:
    case NAL_UNIT_RESERVED_VCL30:
    case NAL_UNIT_RESERVED_VCL31:

    case NAL_UNIT_RESERVED_NVCL41:
    case NAL_UNIT_RESERVED_NVCL42:
    case NAL_UNIT_RESERVED_NVCL43:
    case NAL_UNIT_RESERVED_NVCL44:
    case NAL_UNIT_RESERVED_NVCL45:
    case NAL_UNIT_RESERVED_NVCL46:
    case NAL_UNIT_RESERVED_NVCL47:

    case NAL_UNIT_UNSPECIFIED_48:
    case NAL_UNIT_UNSPECIFIED_49:
    case NAL_UNIT_UNSPECIFIED_50:
    case NAL_UNIT_UNSPECIFIED_51:
    case NAL_UNIT_UNSPECIFIED_52:
    case NAL_UNIT_UNSPECIFIED_53:
    case NAL_UNIT_UNSPECIFIED_54:
    case NAL_UNIT_UNSPECIFIED_55:
    case NAL_UNIT_UNSPECIFIED_56:
    case NAL_UNIT_UNSPECIFIED_57:
    case NAL_UNIT_UNSPECIFIED_58:
    case NAL_UNIT_UNSPECIFIED_59:
    case NAL_UNIT_UNSPECIFIED_60:
    case NAL_UNIT_UNSPECIFIED_61:
    case NAL_UNIT_UNSPECIFIED_62:
    case NAL_UNIT_UNSPECIFIED_63:
      m_transcoder.transcode(sourceNalu, encodedNalu);
      break;

    default:
      assert(0);
      break;
  }
}


/**
 * Checks if the given nal unit signals the start of a new access unit
 *
 * See: Section 7.4.2.4.4 of the HEVC HM spec
 */
Bool TAppDbrTop::xIsFirstNalUnitOfNewAccessUnit(const NALUnit& nalu) const {
  if (nalu.m_nuhLayerId != 0) {
    return false;
  }

  switch (nalu.m_nalUnitType) {
    case NAL_UNIT_ACCESS_UNIT_DELIMITER:

    case NAL_UNIT_VPS:
    case NAL_UNIT_SPS:
    case NAL_UNIT_PPS:

    case NAL_UNIT_PREFIX_SEI:

    case NAL_UNIT_RESERVED_NVCL41:
    case NAL_UNIT_RESERVED_NVCL42:
    case NAL_UNIT_RESERVED_NVCL43:
    case NAL_UNIT_RESERVED_NVCL44:

    case NAL_UNIT_UNSPECIFIED_48:
    case NAL_UNIT_UNSPECIFIED_49:
    case NAL_UNIT_UNSPECIFIED_50:
    case NAL_UNIT_UNSPECIFIED_51:
    case NAL_UNIT_UNSPECIFIED_52:
    case NAL_UNIT_UNSPECIFIED_53:
    case NAL_UNIT_UNSPECIFIED_54:
    case NAL_UNIT_UNSPECIFIED_55:
      return true;
  }

  return false;
}


/**
 * Writes the current access unit to the given bitstream and resets the current
 *   access unit list
 */
Void TAppDbrTop::xFlushAccessUnit(ostream& stream) {
  writeAnnexB(stream, m_currentAccessUnit);

  for (auto it = m_currentAccessUnit.begin(); it != m_currentAccessUnit.end(); it++) {
    delete *it;
  }

  m_currentAccessUnit.clear();
}


/**
 * Directly copies raw data from the xml reader into the nal unit body
 */
Void TAppDbrTop::xReadRawNaluBody(InputNALUnit& nalu, TDbrXmlReader& xmlReader) {
  xmlReader.readOpenTag("raw");

  std::string line;
  std::string contents = "";
  std::istream& underlyingStream = *xmlReader.getStream();
  while (std::getline(underlyingStream, line)) {
    if (line.back() == '\r') {
      line.erase(line.length() - 1);
    }
    if (TDbrXmlReader::isCloseTag(line, "raw")) {
      if (contents.size() > 0) {
        contents.erase(contents.length() - 1);
      }
      break;
    } else {
      contents += line + "\n";
    }
  }

  std::vector<UChar>& fifo = nalu.getBitstream().getFifo();
  fifo.resize(contents.size() + 2);
  std::copy(contents.begin(), contents.end(), fifo.data() + 2);
}


/**
 * Directly copies raw data from the xml reader into the nal unit body
 */
Void TAppDbrTop::xCopyStringIntoNaluBitstream(const string& str, InputNALUnit& nalu) {
  std::vector<UChar>& fifo = nalu.getBitstream().getFifo();
  fifo.resize(str.size());
  std::copy(str.begin(), str.end(), fifo.data());
}


/**
 * True if a nal unit's contents are raw-encoded
 */
Bool TAppDbrTop::xIsNalUnitBodyRawEncoded(NalUnitType nalUnitType) {
  switch (nalUnitType) {
    case NAL_UNIT_VPS:
    case NAL_UNIT_SPS:
    case NAL_UNIT_PPS:

    case NAL_UNIT_CODED_SLICE_TRAIL_R:
    case NAL_UNIT_CODED_SLICE_TRAIL_N:
    case NAL_UNIT_CODED_SLICE_TSA_R:
    case NAL_UNIT_CODED_SLICE_TSA_N:
    case NAL_UNIT_CODED_SLICE_STSA_R:
    case NAL_UNIT_CODED_SLICE_STSA_N:
    case NAL_UNIT_CODED_SLICE_BLA_W_LP:
    case NAL_UNIT_CODED_SLICE_BLA_W_RADL:
    case NAL_UNIT_CODED_SLICE_BLA_N_LP:
    case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
    case NAL_UNIT_CODED_SLICE_IDR_N_LP:
    case NAL_UNIT_CODED_SLICE_CRA:
    case NAL_UNIT_CODED_SLICE_RADL_N:
    case NAL_UNIT_CODED_SLICE_RADL_R:
    case NAL_UNIT_CODED_SLICE_RASL_N:
    case NAL_UNIT_CODED_SLICE_RASL_R:
      return false;

    default:
      return true;
  }
}


/**
 * True if the nal unit contains a parameter set
 */
Bool TAppDbrTop::xIsParameterSetNalUnit(NalUnitType nalUnitType) {
  switch (nalUnitType) {
    case NAL_UNIT_VPS:
    case NAL_UNIT_SPS:
    case NAL_UNIT_PPS:
      return true;
    default:
      return false;
  }
}


//! \}
