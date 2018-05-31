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

#include "TLibEncoder/AnnexBwrite.h"

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

  // Pointer to decoded picture buffer
  TComList<TComPic*>* dpb = NULL;

  // Open input h265 bitstream for reading source video
  ifstream inputStream;
  xOpenInputStream(inputStream);
  InputByteStream inputByteStream(inputStream);

  // Open output h265 bitstream for writing transcoded video
  ofstream outputStream;
  xOpenOutputStream(outputStream);

  // Reset decoded yuv output stream
  if (m_decodedYUVOutputStream.isOpen()) {
    m_decodedYUVOutputStream.close();
  }

  // Copy transcoding configuration to the encoder and decoder
  xConfigDecoder();
  xConfigEncoder();

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
    AnnexBStats  stats;
    byteStreamNALUnit(inputByteStream, nalu.getBitstream().getFifo(), stats);

    // True if a new picture is found within the current NAL unit
    Bool wasNewPictureFound = false;
    
    /* Check for empty bitstream. This can happen if the following occur:
     *  - empty input file
     *  - two back-to-back start_code_prefixes
     *  - start_code_prefix immediately followed by EOF
     */
    if (nalu.getBitstream().getFifo().empty()) {
      fprintf(stderr, "Warning: Attempt to decode an empty NAL unit\n");

    // Parse nal unit header
    } else {
      read(nalu);

      Bool willDecodeTemporalId = (
        m_iMaxTemporalLayer < 0 || nalu.m_temporalId <= m_iMaxTemporalLayer
      );

      // List of access units to write to output stream
      list<AccessUnit> outputAUs;

      // Call decoding function
      if (willDecodeTemporalId && xWillDecodeLayer(nalu.m_nuhLayerId)) {
        wasNewPictureFound =
          m_decoder.decode(nalu, m_iSkipFrame, m_lastOutputPOC);
        xEncodeUnit(nalu, outputAUs);
      } else {
        m_encoder.encode(nalu, outputAUs);
      }

      // Write any produced access units to output stream
      xWriteOutput(outputStream, outputAUs);

    }

    // If a new picture was found in the current NAL unit, adjust the input
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
      inputStream.seekg(initialPositionInInputBitstream - streamoff(3));
      inputByteStream.reset();
#endif
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
Void TAppTraTop::xOpenInputStream(ifstream& stream) const {
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
Void TAppTraTop::xOpenOutputStream(ofstream& stream) const {
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
  /*TComVPS vps;

  vps.setMaxTLayers(m_maxTempLayer);
  if (m_maxTempLayer == 1) {
    vps.setTemporalNestingFlag(true);
  }
  vps.setMaxLayers(1);
  for (Int i = 0; i < MAX_TLAYER; i++) {
    vps.setNumReorderPics                                         ( m_numReorderPics[i], i );
    vps.setMaxDecPicBuffering                                     ( m_maxDecPicBuffering[i], i );
  }
  m_encoder.setVPS(&vps);

  m_encoder.setProfile                                           ( m_profile);
  m_encoder.setLevel                                             ( m_levelTier, m_level);
  m_encoder.setProgressiveSourceFlag                             ( m_progressiveSourceFlag);
  m_encoder.setInterlacedSourceFlag                              ( m_interlacedSourceFlag);
  m_encoder.setNonPackedConstraintFlag                           ( m_nonPackedConstraintFlag);
  m_encoder.setFrameOnlyConstraintFlag                           ( m_frameOnlyConstraintFlag);
  m_encoder.setBitDepthConstraintValue                           ( m_bitDepthConstraint );
  m_encoder.setChromaFormatConstraintValue                       ( m_chromaFormatConstraint );
  m_encoder.setIntraConstraintFlag                               ( m_intraConstraintFlag );
  m_encoder.setOnePictureOnlyConstraintFlag                      ( m_onePictureOnlyConstraintFlag );
  m_encoder.setLowerBitRateConstraintFlag                        ( m_lowerBitRateConstraintFlag );

  m_encoder.setPrintMSEBasedSequencePSNR                         ( m_printMSEBasedSequencePSNR);
  m_encoder.setPrintFrameMSE                                     ( m_printFrameMSE);
  m_encoder.setPrintSequenceMSE                                  ( m_printSequenceMSE);
#if JVET_F0064_MSSSIM
  m_encoder.setPrintMSSSIM                                       ( m_printMSSSIM );
#endif
  m_encoder.setCabacZeroWordPaddingEnabled                       ( m_cabacZeroWordPaddingEnabled );

  m_encoder.setFrameRate                                         ( m_iFrameRate );
  m_encoder.setFrameSkip                                         ( m_FrameSkip );
  m_encoder.setTemporalSubsampleRatio                            ( m_temporalSubsampleRatio );
  m_encoder.setSourceWidth                                       ( m_iSourceWidth );
  m_encoder.setSourceHeight                                      ( m_iSourceHeight );
  m_encoder.setConformanceWindow                                 ( m_confWinLeft, m_confWinRight, m_confWinTop, m_confWinBottom );
  m_encoder.setFramesToBeEncoded                                 ( m_framesToBeEncoded );

  //====== Coding Structure ========
  m_encoder.setIntraPeriod                                       ( m_iIntraPeriod );
  m_encoder.setDecodingRefreshType                               ( m_iDecodingRefreshType );
  m_encoder.setGOPSize                                           ( m_iGOPSize );
#if JCTVC_Y0038_PARAMS
  m_encoder.setReWriteParamSetsFlag                              ( m_bReWriteParamSetsFlag );
#endif
  m_encoder.setGopList                                           ( m_GOPList );
  m_encoder.setExtraRPSs                                         ( m_extraRPSs );
  for(Int i = 0; i < MAX_TLAYER; i++)
  {
    m_encoder.setNumReorderPics                                  ( m_numReorderPics[i], i );
    m_encoder.setMaxDecPicBuffering                              ( m_maxDecPicBuffering[i], i );
  }
  for( UInt uiLoop = 0; uiLoop < MAX_TLAYER; ++uiLoop )
  {
    m_encoder.setLambdaModifier                                  ( uiLoop, m_adLambdaModifier[ uiLoop ] );
  }
  m_encoder.setIntraLambdaModifier                               ( m_adIntraLambdaModifier );
  m_encoder.setIntraQpFactor                                     ( m_dIntraQpFactor );

  m_encoder.setQP                                                ( m_iQP );

#if X0038_LAMBDA_FROM_QP_CAPABILITY
  m_encoder.setIntraQPOffset                                     ( m_intraQPOffset );
  m_encoder.setLambdaFromQPEnable                                ( m_lambdaFromQPEnable );
#endif
  m_encoder.setPad                                               ( m_aiPad );

  m_encoder.setAccessUnitDelimiter                               ( m_AccessUnitDelimiter );

  m_encoder.setMaxTempLayer                                      ( m_maxTempLayer );
  m_encoder.setUseAMP( m_enableAMP );

  //===== Slice ========

  //====== Loop/Deblock Filter ========
  m_encoder.setLoopFilterDisable                                 ( m_bLoopFilterDisable       );
  m_encoder.setLoopFilterOffsetInPPS                             ( m_loopFilterOffsetInPPS );
  m_encoder.setLoopFilterBetaOffset                              ( m_loopFilterBetaOffsetDiv2  );
  m_encoder.setLoopFilterTcOffset                                ( m_loopFilterTcOffsetDiv2    );
  m_encoder.setDeblockingFilterMetric                            ( m_deblockingFilterMetric );

  //====== Motion search ========
  m_encoder.setDisableIntraPUsInInterSlices                      ( m_bDisableIntraPUsInInterSlices );
  m_encoder.setMotionEstimationSearchMethod                      ( m_motionEstimationSearchMethod  );
  m_encoder.setSearchRange                                       ( m_iSearchRange );
  m_encoder.setBipredSearchRange                                 ( m_bipredSearchRange );
  m_encoder.setClipForBiPredMeEnabled                            ( m_bClipForBiPredMeEnabled );
  m_encoder.setFastMEAssumingSmootherMVEnabled                   ( m_bFastMEAssumingSmootherMVEnabled );
  m_encoder.setMinSearchWindow                                   ( m_minSearchWindow );
  m_encoder.setRestrictMESampling                                ( m_bRestrictMESampling );

  //====== Quality control ========
  m_encoder.setMaxDeltaQP                                        ( m_iMaxDeltaQP  );
  m_encoder.setMaxCuDQPDepth                                     ( m_iMaxCuDQPDepth  );
  m_encoder.setDiffCuChromaQpOffsetDepth                         ( m_diffCuChromaQpOffsetDepth );
  m_encoder.setChromaCbQpOffset                                  ( m_cbQpOffset     );
  m_encoder.setChromaCrQpOffset                                  ( m_crQpOffset  );
  m_encoder.setWCGChromaQpControl                                ( m_wcgChromaQpControl );
  m_encoder.setSliceChromaOffsetQpIntraOrPeriodic                ( m_sliceChromaQpOffsetPeriodicity, m_sliceChromaQpOffsetIntraOrPeriodic );
  m_encoder.setChromaFormatIdc                                   ( m_chromaFormatIDC  );

#if ADAPTIVE_QP_SELECTION
  m_encoder.setUseAdaptQpSelect                                  ( m_bUseAdaptQpSelect   );
#endif

  m_encoder.setUseAdaptiveQP                                     ( m_bUseAdaptiveQP  );
  m_encoder.setQPAdaptationRange                                 ( m_iQPAdaptationRange );
  m_encoder.setExtendedPrecisionProcessingFlag                   ( m_extendedPrecisionProcessingFlag );
  m_encoder.setHighPrecisionOffsetsEnabledFlag                   ( m_highPrecisionOffsetsEnabledFlag );

  m_encoder.setWeightedPredictionMethod( m_weightedPredictionMethod );

  //====== Tool list ========
  m_encoder.setLumaLevelToDeltaQPControls                        ( m_lumaLevelToDeltaQPMapping );
#if X0038_LAMBDA_FROM_QP_CAPABILITY
  m_encoder.setDeltaQpRD( (m_costMode==COST_LOSSLESS_CODING) ? 0 : m_uiDeltaQpRD );
#else
  m_encoder.setDeltaQpRD                                         ( m_uiDeltaQpRD  );
#endif
  m_encoder.setFastDeltaQp                                       ( m_bFastDeltaQP  );
  m_encoder.setUseASR                                            ( m_bUseASR      );
  m_encoder.setUseHADME                                          ( m_bUseHADME    );
  m_encoder.setdQPs                                              ( m_aidQP        );
  m_encoder.setUseRDOQ                                           ( m_useRDOQ     );
  m_encoder.setUseRDOQTS                                         ( m_useRDOQTS   );
  m_encoder.setUseSelectiveRDOQ                                  ( m_useSelectiveRDOQ );
  m_encoder.setRDpenalty                                         ( m_rdPenalty );
  m_encoder.setMaxCUWidth                                        ( m_uiMaxCUWidth );
  m_encoder.setMaxCUHeight                                       ( m_uiMaxCUHeight );
  m_encoder.setMaxTotalCUDepth                                   ( m_uiMaxTotalCUDepth );
  m_encoder.setLog2DiffMaxMinCodingBlockSize                     ( m_uiLog2DiffMaxMinCodingBlockSize );
  m_encoder.setQuadtreeTULog2MaxSize                             ( m_uiQuadtreeTULog2MaxSize );
  m_encoder.setQuadtreeTULog2MinSize                             ( m_uiQuadtreeTULog2MinSize );
  m_encoder.setQuadtreeTUMaxDepthInter                           ( m_uiQuadtreeTUMaxDepthInter );
  m_encoder.setQuadtreeTUMaxDepthIntra                           ( m_uiQuadtreeTUMaxDepthIntra );
  m_encoder.setFastInterSearchMode                               ( m_fastInterSearchMode );
  m_encoder.setUseEarlyCU                                        ( m_bUseEarlyCU  );
  m_encoder.setUseFastDecisionForMerge                           ( m_useFastDecisionForMerge  );
  m_encoder.setUseCbfFastMode                                    ( m_bUseCbfFastMode  );
  m_encoder.setUseEarlySkipDetection                             ( m_useEarlySkipDetection );
  m_encoder.setCrossComponentPredictionEnabledFlag               ( m_crossComponentPredictionEnabledFlag );
  m_encoder.setUseReconBasedCrossCPredictionEstimate             ( m_reconBasedCrossCPredictionEstimate );
  m_encoder.setLog2SaoOffsetScale                                ( CHANNEL_TYPE_LUMA  , m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA]   );
  m_encoder.setLog2SaoOffsetScale                                ( CHANNEL_TYPE_CHROMA, m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA] );
  m_encoder.setUseTransformSkip                                  ( m_useTransformSkip      );
  m_encoder.setUseTransformSkipFast                              ( m_useTransformSkipFast  );
  m_encoder.setTransformSkipRotationEnabledFlag                  ( m_transformSkipRotationEnabledFlag );
  m_encoder.setTransformSkipContextEnabledFlag                   ( m_transformSkipContextEnabledFlag   );
  m_encoder.setPersistentRiceAdaptationEnabledFlag               ( m_persistentRiceAdaptationEnabledFlag );
  m_encoder.setCabacBypassAlignmentEnabledFlag                   ( m_cabacBypassAlignmentEnabledFlag );
  m_encoder.setLog2MaxTransformSkipBlockSize                     ( m_log2MaxTransformSkipBlockSize  );
  for (UInt signallingModeIndex = 0; signallingModeIndex < NUMBER_OF_RDPCM_SIGNALLING_MODES; signallingModeIndex++)
  {
    m_encoder.setRdpcmEnabledFlag                                ( RDPCMSignallingMode(signallingModeIndex), m_rdpcmEnabledFlag[signallingModeIndex]);
  }
  m_encoder.setUseConstrainedIntraPred                           ( m_bUseConstrainedIntraPred );
  m_encoder.setFastUDIUseMPMEnabled                              ( m_bFastUDIUseMPMEnabled );
  m_encoder.setFastMEForGenBLowDelayEnabled                      ( m_bFastMEForGenBLowDelayEnabled );
  m_encoder.setUseBLambdaForNonKeyLowDelayPictures               ( m_bUseBLambdaForNonKeyLowDelayPictures );
  m_encoder.setPCMLog2MinSize                                    ( m_uiPCMLog2MinSize);
  m_encoder.setUsePCM                                            ( m_usePCM );

  // set internal bit-depth and constants
  for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
  {
    m_encoder.setBitDepth((ChannelType)channelType, m_internalBitDepth[channelType]);
    m_encoder.setPCMBitDepth((ChannelType)channelType, m_bPCMInputBitDepthFlag ? m_MSBExtendedBitDepth[channelType] : m_internalBitDepth[channelType]);
  }

  m_encoder.setPCMLog2MaxSize                                    ( m_pcmLog2MaxSize);
  m_encoder.setMaxNumMergeCand                                   ( m_maxNumMergeCand );


  //====== Weighted Prediction ========
  m_encoder.setUseWP                                             ( m_useWeightedPred     );
  m_encoder.setWPBiPred                                          ( m_useWeightedBiPred   );

  //====== Parallel Merge Estimation ========
  m_encoder.setLog2ParallelMergeLevelMinus2                      ( m_log2ParallelMergeLevel - 2 );

  //====== Slice ========
  m_encoder.setSliceMode                                         ( m_sliceMode );
  m_encoder.setSliceArgument                                     ( m_sliceArgument );

  //====== Dependent Slice ========
  m_encoder.setSliceSegmentMode                                  ( m_sliceSegmentMode );
  m_encoder.setSliceSegmentArgument                              ( m_sliceSegmentArgument );

  if(m_sliceMode == NO_SLICES )
  {
    m_bLFCrossSliceBoundaryFlag = true;
  }
  m_encoder.setLFCrossSliceBoundaryFlag                          ( m_bLFCrossSliceBoundaryFlag );
  m_encoder.setUseSAO                                            ( m_bUseSAO );
  m_encoder.setTestSAODisableAtPictureLevel                      ( m_bTestSAODisableAtPictureLevel );
  m_encoder.setSaoEncodingRate                                   ( m_saoEncodingRate );
  m_encoder.setSaoEncodingRateChroma                             ( m_saoEncodingRateChroma );
  m_encoder.setMaxNumOffsetsPerPic                               ( m_maxNumOffsetsPerPic);

  m_encoder.setSaoCtuBoundary                                    ( m_saoCtuBoundary);
  m_encoder.setSaoResetEncoderStateAfterIRAP                     ( m_saoResetEncoderStateAfterIRAP);
  m_encoder.setPCMInputBitDepthFlag                              ( m_bPCMInputBitDepthFlag);
  m_encoder.setPCMFilterDisableFlag                              ( m_bPCMFilterDisableFlag);

  m_encoder.setIntraSmoothingDisabledFlag                        (!m_enableIntraReferenceSmoothing );
  m_encoder.setDecodedPictureHashSEIType                         ( m_decodedPictureHashSEIType );
  m_encoder.setRecoveryPointSEIEnabled                           ( m_recoveryPointSEIEnabled );
  m_encoder.setBufferingPeriodSEIEnabled                         ( m_bufferingPeriodSEIEnabled );
  m_encoder.setPictureTimingSEIEnabled                           ( m_pictureTimingSEIEnabled );
  m_encoder.setToneMappingInfoSEIEnabled                         ( m_toneMappingInfoSEIEnabled );
  m_encoder.setTMISEIToneMapId                                   ( m_toneMapId );
  m_encoder.setTMISEIToneMapCancelFlag                           ( m_toneMapCancelFlag );
  m_encoder.setTMISEIToneMapPersistenceFlag                      ( m_toneMapPersistenceFlag );
  m_encoder.setTMISEICodedDataBitDepth                           ( m_toneMapCodedDataBitDepth );
  m_encoder.setTMISEITargetBitDepth                              ( m_toneMapTargetBitDepth );
  m_encoder.setTMISEIModelID                                     ( m_toneMapModelId );
  m_encoder.setTMISEIMinValue                                    ( m_toneMapMinValue );
  m_encoder.setTMISEIMaxValue                                    ( m_toneMapMaxValue );
  m_encoder.setTMISEISigmoidMidpoint                             ( m_sigmoidMidpoint );
  m_encoder.setTMISEISigmoidWidth                                ( m_sigmoidWidth );
  m_encoder.setTMISEIStartOfCodedInterva                         ( m_startOfCodedInterval );
  m_encoder.setTMISEINumPivots                                   ( m_numPivots );
  m_encoder.setTMISEICodedPivotValue                             ( m_codedPivotValue );
  m_encoder.setTMISEITargetPivotValue                            ( m_targetPivotValue );
  m_encoder.setTMISEICameraIsoSpeedIdc                           ( m_cameraIsoSpeedIdc );
  m_encoder.setTMISEICameraIsoSpeedValue                         ( m_cameraIsoSpeedValue );
  m_encoder.setTMISEIExposureIndexIdc                            ( m_exposureIndexIdc );
  m_encoder.setTMISEIExposureIndexValue                          ( m_exposureIndexValue );
  m_encoder.setTMISEIExposureCompensationValueSignFlag           ( m_exposureCompensationValueSignFlag );
  m_encoder.setTMISEIExposureCompensationValueNumerator          ( m_exposureCompensationValueNumerator );
  m_encoder.setTMISEIExposureCompensationValueDenomIdc           ( m_exposureCompensationValueDenomIdc );
  m_encoder.setTMISEIRefScreenLuminanceWhite                     ( m_refScreenLuminanceWhite );
  m_encoder.setTMISEIExtendedRangeWhiteLevel                     ( m_extendedRangeWhiteLevel );
  m_encoder.setTMISEINominalBlackLevelLumaCodeValue              ( m_nominalBlackLevelLumaCodeValue );
  m_encoder.setTMISEINominalWhiteLevelLumaCodeValue              ( m_nominalWhiteLevelLumaCodeValue );
  m_encoder.setTMISEIExtendedWhiteLevelLumaCodeValue             ( m_extendedWhiteLevelLumaCodeValue );
  m_encoder.setChromaResamplingFilterHintEnabled                 ( m_chromaResamplingFilterSEIenabled );
  m_encoder.setChromaResamplingHorFilterIdc                      ( m_chromaResamplingHorFilterIdc );
  m_encoder.setChromaResamplingVerFilterIdc                      ( m_chromaResamplingVerFilterIdc );
  m_encoder.setFramePackingArrangementSEIEnabled                 ( m_framePackingSEIEnabled );
  m_encoder.setFramePackingArrangementSEIType                    ( m_framePackingSEIType );
  m_encoder.setFramePackingArrangementSEIId                      ( m_framePackingSEIId );
  m_encoder.setFramePackingArrangementSEIQuincunx                ( m_framePackingSEIQuincunx );
  m_encoder.setFramePackingArrangementSEIInterpretation          ( m_framePackingSEIInterpretation );
  m_encoder.setSegmentedRectFramePackingArrangementSEIEnabled    ( m_segmentedRectFramePackingSEIEnabled );
  m_encoder.setSegmentedRectFramePackingArrangementSEICancel     ( m_segmentedRectFramePackingSEICancel );
  m_encoder.setSegmentedRectFramePackingArrangementSEIType       ( m_segmentedRectFramePackingSEIType );
  m_encoder.setSegmentedRectFramePackingArrangementSEIPersistence( m_segmentedRectFramePackingSEIPersistence );
  m_encoder.setDisplayOrientationSEIAngle                        ( m_displayOrientationSEIAngle );
  m_encoder.setTemporalLevel0IndexSEIEnabled                     ( m_temporalLevel0IndexSEIEnabled );
  m_encoder.setGradualDecodingRefreshInfoEnabled                 ( m_gradualDecodingRefreshInfoEnabled );
  m_encoder.setNoDisplaySEITLayer                                ( m_noDisplaySEITLayer );
  m_encoder.setDecodingUnitInfoSEIEnabled                        ( m_decodingUnitInfoSEIEnabled );
  m_encoder.setSOPDescriptionSEIEnabled                          ( m_SOPDescriptionSEIEnabled );
  m_encoder.setScalableNestingSEIEnabled                         ( m_scalableNestingSEIEnabled );
  m_encoder.setTMCTSSEIEnabled                                   ( m_tmctsSEIEnabled );
#if MCTS_ENC_CHECK
  m_encoder.setTMCTSSEITileConstraint                            ( m_tmctsSEITileConstraint );
#endif
  m_encoder.setTimeCodeSEIEnabled                                ( m_timeCodeSEIEnabled );
  m_encoder.setNumberOfTimeSets                                  ( m_timeCodeSEINumTs );
  for(Int i = 0; i < m_timeCodeSEINumTs; i++)
  {
    m_encoder.setTimeSet(m_timeSetArray[i], i);
  }
  m_encoder.setKneeSEIEnabled                                    ( m_kneeSEIEnabled );
  m_encoder.setKneeSEIId                                         ( m_kneeSEIId );
  m_encoder.setKneeSEICancelFlag                                 ( m_kneeSEICancelFlag );
  m_encoder.setKneeSEIPersistenceFlag                            ( m_kneeSEIPersistenceFlag );
  m_encoder.setKneeSEIInputDrange                                ( m_kneeSEIInputDrange );
  m_encoder.setKneeSEIInputDispLuminance                         ( m_kneeSEIInputDispLuminance );
  m_encoder.setKneeSEIOutputDrange                               ( m_kneeSEIOutputDrange );
  m_encoder.setKneeSEIOutputDispLuminance                        ( m_kneeSEIOutputDispLuminance );
  m_encoder.setKneeSEINumKneePointsMinus1                        ( m_kneeSEINumKneePointsMinus1 );
  m_encoder.setKneeSEIInputKneePoint                             ( m_kneeSEIInputKneePoint );
  m_encoder.setKneeSEIOutputKneePoint                            ( m_kneeSEIOutputKneePoint );
  m_encoder.setColourRemapInfoSEIFileRoot                        ( m_colourRemapSEIFileRoot );
  m_encoder.setMasteringDisplaySEI                               ( m_masteringDisplay );
  m_encoder.setSEIAlternativeTransferCharacteristicsSEIEnable    ( m_preferredTransferCharacteristics>=0     );
  m_encoder.setSEIPreferredTransferCharacteristics               ( UChar(m_preferredTransferCharacteristics) );
  m_encoder.setSEIGreenMetadataInfoSEIEnable                     ( m_greenMetadataType > 0 );
  m_encoder.setSEIGreenMetadataType                              ( UChar(m_greenMetadataType) );
  m_encoder.setSEIXSDMetricType                                  ( UChar(m_xsdMetricType) );

  m_encoder.setTileUniformSpacingFlag                            ( m_tileUniformSpacingFlag );
  m_encoder.setNumColumnsMinus1                                  ( m_numTileColumnsMinus1 );
  m_encoder.setNumRowsMinus1                                     ( m_numTileRowsMinus1 );
  if(!m_tileUniformSpacingFlag)
  {
    m_encoder.setColumnWidth                                     ( m_tileColumnWidth );
    m_encoder.setRowHeight                                       ( m_tileRowHeight );
  }
  m_encoder.xCheckGSParameters();
  Int uiTilesCount = (m_numTileRowsMinus1+1) * (m_numTileColumnsMinus1+1);
  if(uiTilesCount == 1)
  {
    m_bLFCrossTileBoundaryFlag = true;
  }
  m_encoder.setLFCrossTileBoundaryFlag                           ( m_bLFCrossTileBoundaryFlag );
  m_encoder.setEntropyCodingSyncEnabledFlag                      ( m_entropyCodingSyncEnabledFlag );
  m_encoder.setTMVPModeId                                        ( m_TMVPModeId );
  m_encoder.setUseScalingListId                                  ( m_useScalingListId  );
  m_encoder.setScalingListFileName                               ( m_scalingListFileName );
  m_encoder.setSignDataHidingEnabledFlag                         ( m_signDataHidingEnabledFlag);
  m_encoder.setUseRateCtrl                                       ( m_RCEnableRateControl );
  m_encoder.setTargetBitrate                                     ( m_RCTargetBitrate );
  m_encoder.setKeepHierBit                                       ( m_RCKeepHierarchicalBit );
  m_encoder.setLCULevelRC                                        ( m_RCLCULevelRC );
  m_encoder.setUseLCUSeparateModel                               ( m_RCUseLCUSeparateModel );
  m_encoder.setInitialQP                                         ( m_RCInitialQP );
  m_encoder.setForceIntraQP                                      ( m_RCForceIntraQP );
  m_encoder.setCpbSaturationEnabled                              ( m_RCCpbSaturationEnabled );
  m_encoder.setCpbSize                                           ( m_RCCpbSize );
  m_encoder.setInitialCpbFullness                                ( m_RCInitialCpbFullness );
  m_encoder.setTransquantBypassEnabledFlag                       ( m_TransquantBypassEnabledFlag );
  m_encoder.setCUTransquantBypassFlagForceValue                  ( m_CUTransquantBypassFlagForce );
  m_encoder.setCostMode                                          ( m_costMode );
  m_encoder.setUseRecalculateQPAccordingToLambda                 ( m_recalculateQPAccordingToLambda );
  m_encoder.setUseStrongIntraSmoothing                           ( m_useStrongIntraSmoothing );
  m_encoder.setActiveParameterSetsSEIEnabled                     ( m_activeParameterSetsSEIEnabled );
  m_encoder.setVuiParametersPresentFlag                          ( m_vuiParametersPresentFlag );
  m_encoder.setAspectRatioInfoPresentFlag                        ( m_aspectRatioInfoPresentFlag);
  m_encoder.setAspectRatioIdc                                    ( m_aspectRatioIdc );
  m_encoder.setSarWidth                                          ( m_sarWidth );
  m_encoder.setSarHeight                                         ( m_sarHeight );
  m_encoder.setOverscanInfoPresentFlag                           ( m_overscanInfoPresentFlag );
  m_encoder.setOverscanAppropriateFlag                           ( m_overscanAppropriateFlag );
  m_encoder.setVideoSignalTypePresentFlag                        ( m_videoSignalTypePresentFlag );
  m_encoder.setVideoFormat                                       ( m_videoFormat );
  m_encoder.setVideoFullRangeFlag                                ( m_videoFullRangeFlag );
  m_encoder.setColourDescriptionPresentFlag                      ( m_colourDescriptionPresentFlag );
  m_encoder.setColourPrimaries                                   ( m_colourPrimaries );
  m_encoder.setTransferCharacteristics                           ( m_transferCharacteristics );
  m_encoder.setMatrixCoefficients                                ( m_matrixCoefficients );
  m_encoder.setChromaLocInfoPresentFlag                          ( m_chromaLocInfoPresentFlag );
  m_encoder.setChromaSampleLocTypeTopField                       ( m_chromaSampleLocTypeTopField );
  m_encoder.setChromaSampleLocTypeBottomField                    ( m_chromaSampleLocTypeBottomField );
  m_encoder.setNeutralChromaIndicationFlag                       ( m_neutralChromaIndicationFlag );
  m_encoder.setDefaultDisplayWindow                              ( m_defDispWinLeftOffset, m_defDispWinRightOffset, m_defDispWinTopOffset, m_defDispWinBottomOffset );
  m_encoder.setFrameFieldInfoPresentFlag                         ( m_frameFieldInfoPresentFlag );
  m_encoder.setPocProportionalToTimingFlag                       ( m_pocProportionalToTimingFlag );
  m_encoder.setNumTicksPocDiffOneMinus1                          ( m_numTicksPocDiffOneMinus1    );
  m_encoder.setBitstreamRestrictionFlag                          ( m_bitstreamRestrictionFlag );
  m_encoder.setTilesFixedStructureFlag                           ( m_tilesFixedStructureFlag );
  m_encoder.setMotionVectorsOverPicBoundariesFlag                ( m_motionVectorsOverPicBoundariesFlag );
  m_encoder.setMinSpatialSegmentationIdc                         ( m_minSpatialSegmentationIdc );
  m_encoder.setMaxBytesPerPicDenom                               ( m_maxBytesPerPicDenom );
  m_encoder.setMaxBitsPerMinCuDenom                              ( m_maxBitsPerMinCuDenom );
  m_encoder.setLog2MaxMvLengthHorizontal                         ( m_log2MaxMvLengthHorizontal );
  m_encoder.setLog2MaxMvLengthVertical                           ( m_log2MaxMvLengthVertical );
  m_encoder.setEfficientFieldIRAPEnabled                         ( m_bEfficientFieldIRAPEnabled );
  m_encoder.setHarmonizeGopFirstFieldCoupleEnabled               ( m_bHarmonizeGopFirstFieldCoupleEnabled );

  m_encoder.setSummaryOutFilename                                ( m_summaryOutFilename );
  m_encoder.setSummaryPicFilenameBase                            ( m_summaryPicFilenameBase );
  m_encoder.setSummaryVerboseness                                ( m_summaryVerboseness );*/
}


/**
 * Writes reconstructed frames in the decoded picture buffer to the output
 *   bitstream
 *
 * \param dpb  Decoded picture buffer, sorted in increasing POC order
 */
Void TAppTraTop::xDisplayDecodedFrames(TComList<TComPic*>* dpb) {
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
Void TAppTraTop::xFlushPictureBuffer(TComList<TComPic*>* dpb) {
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
Void TAppTraTop::xWriteFrameToOutput(TComPic* frame) {
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
Void TAppTraTop::xWriteFrameToOutput(TComPic* field1, TComPic* field2) {
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


/**
 * Encodes a decoded NAL unit
 */
Void TAppTraTop::xEncodeUnit(const InputNALUnit& nalu, list<AccessUnit>& encodedAUs) {
  switch (nalu.m_nalUnitType) {
    case NAL_UNIT_VPS:
      m_encoder.encode(nalu, *m_decoder.getVPS(), encodedAUs);
      break;

    case NAL_UNIT_SPS:
      m_encoder.encode(nalu, *m_decoder.getSPS(), encodedAUs);
      break;

    case NAL_UNIT_PPS:
      m_encoder.encode(nalu, *m_decoder.getPPS(), encodedAUs);
      break;

    case NAL_UNIT_PREFIX_SEI:
    case NAL_UNIT_SUFFIX_SEI:
      m_encoder.encode(nalu, encodedAUs);
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
      m_encoder.encode(nalu, *m_decoder.getCurSlice(), encodedAUs);
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
      m_encoder.encode(nalu, encodedAUs);
      break;

    default:
      assert(0);
      break;
  }
}


/**
 * Writes a list of access units to an ofstream
 */
Void TAppTraTop::xWriteOutput(ofstream& stream, const list<AccessUnit>& outputAUs) const {
  for (auto it = outputAUs.begin(); it != outputAUs.end(); it++) {
    const AccessUnit& au(*it);
    writeAnnexB(stream, au);
  }
}


//! \}
