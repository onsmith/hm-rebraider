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
 *  \file     TTraTop.h
 *  \project  TAppTransrater
 *  \brief    Transrater class header
 */


#pragma once 


#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComTU.h"

#include "TLibDecoder/NALread.h"

#include "TLibEncoder/NALwrite.h"
#include "TLibEncoder/TEncTop.h"


//! \ingroup TAppTransrater
//! \{


class TTraTop : public TEncTop {
public:
  // Default constructor
  TTraTop();

  // Virtual destructor
  virtual ~TTraTop() = default;
  
  // Transcode a NAL unit without decoding
  Void transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu);
  
  // Transcode a decoded VPS NAL unit
  Void transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComVPS& vps);
  
  // Transcode a decoded SPS NAL unit
  Void transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSPS& sps);
  
  // Transcode a decoded PPS NAL unit
  Void transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComPPS& pps);
  
  // Transcode a decoded slice NAL unit
  Void transcode(const InputNALUnit& inputNalu, OutputNALUnit& outputNalu, const TComSlice& slice);


protected:
  /**
   * Encoder/decoder synchronization
   */
  // Copy a TComSlice to a TComPic, returning a reference to the new TComSlice
  TComSlice& xCopySliceToPic(const TComSlice& srcSlice, TComPic& dstPic);
  
  // Set up an encoder TComPic by copying relevant configuration from a
  //   corresponding decoded TComPic
  Void xCopyDecPic(const TComPic& srcPic, TComPic& dstPic);


  /**
   * Picture buffer management
   */
  // Resolve a TComSlice into its corresponding encoder TComPic
  TComPic& xGetEncPicBySlice(const TComSlice& slice);

  // Find an existing const TComPic by POC
  TComPic* xGetEncPicByPoc(Int poc);
  
  // Get an unused entry from the picture buffer
  TComPic*& xGetUnusedEntry();


  /**
   * Entropy coding
   */
 // Parameter set encode and write to bitstream
  Void xEncodeVPS(const TComVPS& vps, TComOutputBitstream& bitstream); // video parameter set encoding
  Void xEncodeSPS(const TComSPS& sps, TComOutputBitstream& bitstream); // sequence parameter set encoding
  Void xEncodePPS(const TComPPS& pps, TComOutputBitstream& bitstream); // picture parameter set encoding

  // Encode slice and write to bitstream
  Void xEncodeSlice(TComSlice& slice, TComOutputBitstream& bitstream);


  /**
   * Requantization
   */
  // Requantize a given slice by altering residual qp
  Void xRequantizeSlice(TComSlice& slice);

  // Recursively requantize a ctu by altering residual qp
  Void xRequantizeCtu(TComDataCU& ctu, UInt cuPartAddr = 0, UInt cuDepth = 0);


  /**
   * Prediction
   */
  // Requantizes an inter-predicted cu
  Void xRequantizeInterCu(TComDataCU& cu, TComYuv& predBuff, TComYuv& resiBuff, TComYuv& recoBuff);
  
  // Recursively requantizes an inter-predicted tu
  Void xRequantizeInterTu(TComTURecurse& tu, ComponentID component, TComYuv& resiBuff);

  // Requantizes an intra-predicted cu
  Void xRequantizeIntraCu(TComDataCU& cu, TComYuv& predBuff, TComYuv& resiBuff, TComYuv& recoBuff);

  //
  Void xPredictIntraTu(TComTURecurse& tu, TComYuv& predictionBuffer, ChannelType channelType);
};


//! \}
