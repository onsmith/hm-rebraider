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
 *  \file     TAppTraTop.cpp
 *  \project  TAppTransrater
 *  \brief    Transrater application class implementation
 */


#include <list>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppTraTop.h"
#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"

#if RExt__DECODER_DEBUG_BIT_STATISTICS
#include "TLibCommon/TComCodingStatistics.h"
#endif


//! \ingroup TAppTransrater
//! \{


/**
 * Default constructor
 */
TAppTraTop::TAppTraTop() :
  m_lastOutputPOC(-MAX_INT) {
}


/**
 * Gets the number of decoding errors detected
 */
UInt TAppTraTop::numDecodingErrorsDetected() const {
  return m_decoder.getNumberOfChecksumErrorsDetected();
}


/**
 * Main transrate processing function. Performs the following steps:
 *   1. Opens the input and output bitstream files
 *   2. Creates encoder and decoder objects
 *   3. Initializes encoder and decoder using defined configuration
 *   4. Until the end of the bitstream, decode and recode the video
 *   5. Destroy the internal encoder and decoder objects
 */
Void TAppTraTop::transrate() {
  // Picture order count
  Int poc;

  // Decoded picture buffer
  TComList<TComPic*>* dpb = NULL;

  // Open input h265 bitstream for reading source video
  ifstream sourceInputStream;
  xOpenSourceInputStream(sourceInputStream);
  InputByteStream inputStream(sourceInputStream);

  // Configure the encoder and decoder
  xConfigDecoder();
  xConfigEncoder();

  // Adjust the last output POC index if seeking is required before decoding
  m_lastOutputPOC += m_iSkipFrame;

  // TODO: Write description of this variable's purpose
  Bool loopFiltered = false;

  // Main decoder loop
  while (!sourceInputStream.fail()) {
    // Remember the starting position of the input bitstream file cursor
    /* Note: This serves to work around a design fault in the decoder, whereby
     * the process of reading a new slice that is the first slice of a new frame
     * requires the TDecTop::decode() method to be called again with the same
     * nal unit. */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
    auto backupStats(TComCodingStatistics::GetStatistics());
    const streampos initialPositionInInputBitstream =
      sourceInputStream.tellg() - streampos(inputStream.GetNumBufferedBytes());
#else
    const streampos initialPositionInInputBitstream = sourceInputStream.tellg();
#endif

    // Read one NAL unit from input bitstream
    InputNALUnit nalu;
    AnnexBStats  stats;
    byteStreamNALUnit(inputStream, nalu.getBitstream().getFifo(), stats);

    // True if a new picture is found within the current NAL unit
    Bool wasNewPictureFound = false;

    // Call decoding function
    if (nalu.getBitstream().getFifo().empty()) {
      /* this can happen if the following occur:
       *  - empty input file
       *  - two back-to-back start_code_prefixes
       *  - start_code_prefix immediately followed by EOF
       */
      fprintf(stderr, "Warning: Attempt to decode an empty NAL unit\n");
    } else {
      read(nalu);

      Bool willDecodeTemporalId = (
        m_iMaxTemporalLayer < 0 || nalu.m_temporalId <= m_iMaxTemporalLayer
      );

      if (willDecodeTemporalId && xWillDecodeLayer(nalu.m_nuhLayerId)) {
        wasNewPictureFound =
          m_decoder.decode(nalu, m_iSkipFrame, m_lastOutputPOC);
      }
    }

    // If a new picture was found in the current NAL unit, adjust the input
    //   bitstream file cursor
    if (wasNewPictureFound) {
      sourceInputStream.clear();
      /* initialPositionInInputBitstream points to the current nalunit
       * payload[1] due to the need for the annexB parser to read three extra
       * bytes. [1] except for the first NAL unit in the file (but
       * wasNewPictureFound doesn't happen then)
       */
#if RExt__DECODER_DEBUG_BIT_STATISTICS
      sourceInputStream.seekg(initialPositionInInputBitstream);
      inputStream.reset();
      TComCodingStatistics::SetStatistics(backupStats);
#else
      sourceInputStream.seekg(initialPositionInInputBitstream - streamoff(3));
      inputStream.reset();
#endif
    }

    // True if a picture was finished while decoding the current NAL unit
    Bool wasPictureFinished =
      (wasNewPictureFound || sourceInputStream.fail() || nalu.isEOS());

    // Apply in-loop filters to reconstructed picture
    if (wasPictureFinished && !m_decoder.getFirstSliceInSequence()) {
      if (!loopFiltered || !sourceInputStream.fail()) {
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
      if (!m_reconFileName.empty() && !m_sourceYUVOutputStream.isOpen()) {
        // Set the output pixel bit depths to be the same as the bitdepths used
        //   by the first reconstructed picture
        const BitDepths& bitDepths =
          dpb->front()->getPicSym()->getSPS().getBitDepths();
        xSetOutputBitDepths(bitDepths);

        // Open YUV reconstruction output file
        m_sourceYUVOutputStream.open(
          m_reconFileName,
          true,
          m_outputBitDepth,
          m_outputBitDepth,
          bitDepths.recon
        );
      }

      // Write reconstructed frames to output
      if (wasNewPictureFound) {
        xWriteOutput(dpb);
      }

      if ((wasNewPictureFound || nalu.isCRASlice()) && m_decoder.getNoOutputPriorPicsFlag()) {
        m_decoder.checkNoOutputPriorPics(dpb);
        m_decoder.setNoOutputPriorPicsFlag(false);
      }

      // Flush output frames when IDR or BLA NAL units are encountered
      // TODO: Why don't we include CRA NAL units here?
      if (wasNewPictureFound && (nalu.isIDRSlice() || nalu.isBLASlice())) {
        xFlushOutput(dpb);
      }

      // Handle end of sequence (EOS) NAL units
      if (nalu.isEOS()) {
        xWriteOutput(dpb);
        m_decoder.setFirstSliceInPicture(false);
      }

      // Write reconstructed frames to output
      //   Note: This is for additional bumping as defined in C.5.2.3
      if (!wasNewPictureFound && nalu.m_nalUnitType >= NAL_UNIT_CODED_SLICE_TRAIL_N && nalu.m_nalUnitType <= NAL_UNIT_RESERVED_VCL31) {
        xWriteOutput(dpb);
      }
    }
  }

  // Send any remaining pictures to output
  xFlushOutput(dpb);

  // Free decoder resources
  m_decoder.deletePicBuffer();
  m_decoder.setDecodedSEIMessageOutputStream(nullptr);
}


/**
 * Helper method to open an ifstream for reading the source hevc bitstream
 */
Void TAppTraTop::xOpenSourceInputStream(ifstream& stream) const {
  stream.open(
    m_bitstreamFileName.c_str(),
    ifstream::in | ifstream::binary
  );

  if (!stream.is_open() || !stream.good()) {
    fprintf(
      stderr,
      "\nfailed to open bitstream file `%s' for reading\n",
      m_bitstreamFileName.c_str()
    );
    exit(EXIT_FAILURE);
  }
}


/**
 * Overwrites the default configuration for output bit depth
 */
Void TAppTraTop::xSetOutputBitDepths(const BitDepths& bitDepths) {
  for (UInt c = 0; c < MAX_NUM_CHANNEL_TYPE; c++) {
    if (m_outputBitDepth[c] == 0) {
      m_outputBitDepth[c] = bitDepths.recon[c];
    }
  }
}


/**
 * Transfers the current configuration to the decoder object
 */
Void TAppTraTop::xConfigDecoder() {
  // Reset source reconstruction yuv output file
  if (m_sourceYUVOutputStream.isOpen()) {
    m_sourceYUVOutputStream.close();
  }

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
 * Transfers the current configuration to the encoder object
 */
Void TAppTraTop::xConfigEncoder() {
}


/**
 * Writes complete, reconstructed pictures in the decoded picture buffer to
 *   the output file.
 *
 * \param pcListPic  list of pictures to be written to file
 * \param tId        temporal sub-layer ID
 */
Void TAppTraTop::xWriteOutput(TComList<TComPic*>* pcListPic) {
  if (pcListPic == nullptr || pcListPic->empty()) {
    return;
  }

  const TComSPS* sps              = &(pcListPic->front()->getPicSym()->getSPS());
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
  for (auto p = pcListPic->begin(); p != pcListPic->end(); p++) {
    TComPic* pic = *p;
    if (pic->getOutputMark() && pic->getPOC() > m_lastOutputPOC) {
      numPicsNotYetDisplayed++;
      numDecodedBufferedPictures++;
    } else if (pic->getSlice(0)->isReferenced()) {
      numDecodedBufferedPictures++;
    }
  }

  auto iterPic = pcListPic->begin();

  if (numPicsNotYetDisplayed > 2) {
    iterPic++;
  }

  TComPic* pcPic = *iterPic;
  
  // Interlaced field output
  if (numPicsNotYetDisplayed > 2 && pcPic->isField()) {
    auto endPic = pcListPic->end();
    endPic--;
    iterPic = pcListPic->begin();

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
    iterPic = pcListPic->begin();

    while (iterPic != pcListPic->end()) {
      pcPic = *(iterPic);

      Bool isBufferFull = (
        numPicsNotYetDisplayed > maxNumReorderedPics ||
        numDecodedBufferedPictures > maxNumBufferedPics
      );

      // Write frame to output file
      if (pcPic->getOutputMark() && isBufferFull) {
        numPicsNotYetDisplayed--;

        if (!pcPic->getSlice(0)->isReferenced()) {
          numDecodedBufferedPictures--;
        }

        xWriteFrameToOutput(pcPic);

        // Update last displayed picture index
        m_lastOutputPOC = pcPic->getPOC();

        // Mark non-referenced fields so they can be reused by the decoder
        if (!pcPic->getSlice(0)->isReferenced() && pcPic->getReconMark()) {
          pcPic->setReconMark(false);
          pcPic->getPicYuvRec()->setBorderExtension(false);
        }

        pcPic->setOutputMark(false);
      }

      iterPic++;
    }
  }
}


/** \param pcListPic list of pictures to be written to file
 */
Void TAppTraTop::xFlushOutput(TComList<TComPic*>* pcListPic) {
  if (pcListPic == nullptr || pcListPic->empty()) {
    return;
  }

  // Interlaced field output
  if (pcListPic->front()->isField()) {
    auto     iterPic     = pcListPic->begin();
    auto     endPic      = pcListPic->end();
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
    for (auto p = pcListPic->begin(); p != pcListPic->end(); p++) {
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
  pcListPic->clear();

  // Reset last displayed index
  m_lastOutputPOC = -MAX_INT;
}


/**
 * Writes a raw reconstructed frame to the output bitstream.
 */
Void TAppTraTop::xWriteFrameToOutput(TComPic* frame) {
  if (m_sourceYUVOutputStream.isOpen()) {
    const Window &conf    = frame->getConformanceWindow();
    const Window  defDisp = (m_respectDefDispWindow ? frame->getDefDisplayWindow() : Window());

    m_sourceYUVOutputStream.write(
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
Void TAppTraTop::xWriteFrameToOutput(TComPic* field1, TComPic* field2) {
  if (m_sourceYUVOutputStream.isOpen()) {
    const Window& conf    = field1->getConformanceWindow();
    const Window  defDisp = (m_respectDefDispWindow ? field1->getDefDisplayWindow() : Window());

    m_sourceYUVOutputStream.write(
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
Bool TAppTraTop::xWillDecodeLayer(Int layerId) const {
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
Bool TAppTraTop::xWillDecodeAllLayers() const {
  return (m_targetDecLayerIdSet.size() == 0);
}


//! \}
