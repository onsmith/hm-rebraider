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
 *  \file     TDbrCabac.h
 *  \project  TAppDebraider
 *  \brief    Cabac encoder class header
 */


#pragma once


#include "TLibEncoder/TEncBinCoder.h"


//! \ingroup TAppDebraider
//! \{


class TDbrCabac : public TEncBinIf {
private:
  // Bitstream to receive encoded output
  TComBitIf* bitstream;


public:
  // Default constructor
  TDbrCabac() = default;

  // Default destructor
  ~TDbrCabac() = default;

  // Sets the underlying bitstream
  Void init(TComBitIf* pcTComBitIf);

  // Removes the underlying bitstream
  Void uninit();

  // Initializes the cabac internal state
  // Note: This method does nothing since this class is not actually a cabac
  Void start();

  // Writes any buffered bits to the output stream, injecting bits as necessary
  //   to unambiguously specify the correct arithmetic code interval
  // Note: This method does nothing since this class is not actually a cabac
  Void finish();

  // Copies internal cabac state from another cabac encoder class
  // Note: This method does nothing since this class is not actually a cabac
  Void copyState(const TEncBinIf* pcTEncBinIf);

  // Writes a terminating character to the bitstream, finishes the stream, and
  //   resets the stream
  // Note: This method does nothing since this class is not actually a cabac
  Void flush();

  // Resets the cabac internal state
  Void resetBac();

  // Encodes PCM alignment zero bits
  // Note: This method does nothing since this class is not actually a cabac
  Void encodePCMAlignBits();

  // Writes a PCM code
  Void xWritePCMCode(UInt uiCode, UInt uiLength);

  // Resets the internal bit buffer
  // Note: This method does nothing since this class is not actually a cabac
  Void resetBits();

  // Gets the number of bits written to the bitstream
  UInt getNumWrittenBits();

  // Encodes a single bit
  Void encodeBin(UInt uiBin, ContextModel& rcCtxModel);

  // Encodes a single bit such that a zero and a one are equiprobable (ep)
  Void encodeBinEP(UInt uiBin);

  // Encodes multiple bits such that zeros and ones are equiprobable (ep)
  Void encodeBinsEP(UInt uiBins, Int numBins);

  // Encodes a terminating character to the bitstream
  // Note: This method does nothing since this class is not actually a cabac
  Void encodeBinTrm(UInt uiBin);

  // Aligns the cabac interval range
  // Note: This method does nothing since this class is not actually a cabac
  Void align();
};


//! \}
