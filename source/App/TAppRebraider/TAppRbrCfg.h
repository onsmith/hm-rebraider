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
 *  \file     TAppRbrCfg.h
 *  \project  TAppRebraider
 *  \brief    Rebraider configuration class header
 */


#ifndef __TAPPTRACFG__
#define __TAPPTRACFG__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "TLibCommon/CommonDef.h"
#include <vector>


using std::string;


//! \ingroup TAppRebraider
//! \{


class TAppRbrCfg {
protected:
  // Input compressed hevc bitstream file name
  string m_inputFileName;

  // Output compressed hevc bitstream file name
  string m_outputFileName;

  // Output reconstruction file name
  string m_reconFileName;

  // The number of frames before the random access point to skip
  Int m_iSkipFrame;

  // Bit depth for output pixel values of each channel
  Int m_outputBitDepth[MAX_NUM_CHANNEL_TYPE];

  // Desired color space conversion
  InputColourSpaceConversion m_outputColourSpaceConvert;

  // Maximum temporal layer to be decoded
  Int m_iMaxTemporalLayer;

  // Checksum(3)/CRC(2)/MD5(1)/disable(0) acting on decoded picture hash SEI
  //   message
  Int m_decodedPictureHashSEIEnabled;

  // Enable(true)/disable(false) writing only pictures that get displayed based
  //   on the no display SEI message
  Bool m_decodedNoDisplaySEIEnabled;

  // Output color remapping file name
  std::string m_colourRemapSEIFileName;

  // set of LayerIds to be included in the sub-bitstream extraction process.
  std::vector<Int> m_targetDecLayerIdSet;

  // Only output content inside the default display window
  Int m_respectDefDispWindow;

  // Filename to output decoded SEI messages to. If '-', then use stdout. If
  //   empty, do not output details.
  std::string m_outputDecodedSEIMessagesFilename;

  // If true, clip the output video to the Rec 709 range on saving.
  Bool m_bClipOutputVideoToRec709Range;

#if MCTS_ENC_CHECK
  Bool m_tmctsCheck;
#endif

#if O0043_BEST_EFFORT_DECODING
  // if non-zero, force the bit depth at the decoder (best effort decoding)
  UInt m_forceDecodeBitDepth;
#endif


public:
  // Default constructor
  TAppRbrCfg();

  // Virtual destructor
  virtual ~TAppRbrCfg();

  // Reads configurations options from command line arguments
  Bool parseCfg(Int argc, TChar* argv[]);
};


//! \}


#endif
