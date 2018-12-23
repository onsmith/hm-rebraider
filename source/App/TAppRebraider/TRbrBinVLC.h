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
 *  \file     TRbrBinVLC.h
 *  \project  TAppRebraider
 *  \brief    VLC binary value encoder class header
 */


#pragma once


#include "TLibDecoder/TDecBinCoder.h"


//! \ingroup TAppRebraider
//! \{


class TRbrBinVLC : public TDecBinIf {
private:
  // Input bitstream from which to read bits
  TComInputBitstream* bitstream;


public:
  // Default constructor
  TRbrBinVLC() = default;

  // Default destructor
  ~TRbrBinVLC() = default;

  // Sets the underlying TComInputBitstream
  Void init(TComInputBitstream* pcTComBitstream);

  // Removes the underlying TComInputBitstream
  Void uninit();

  // Initializes the internal state
  // Note: This method does nothing since there is no internal state
  Void start();

  // Writes any buffered bits to the output stream, injecting bits as necessary
  //   to unambiguously specify the correct arithmetic code interval
  // Note: This method does nothing since this class is not actually a cabac
  Void finish();

  // Reads a PCM code
  Void xReadPCMCode(UInt uiLength, UInt& ruiCode);

  // Decodes a single bit
  Void decodeBin(UInt& ruiBin, ContextModel& rcCtxModel);

  // Decodes a single bit when a zero and a one are equiprobable (ep)
  Void decodeBinEP(UInt& ruiBin);

  // Decodes multiple bits when zeros and ones are equiprobable (ep)
  Void decodeBinsEP(UInt& ruiBins, Int numBins);

  // Aligns the cabac interval range
  // Note: This method does nothing since this class is not actually a cabac
  Void align();

  // Decodes a terminating character from the bitstream
  Void decodeBinTrm(UInt& ruiBin);

  // Copies internal state from another decoder object
  // Note: This method does nothing since there is no internal state
  Void copyState(const TDecBinIf* pcTDecBinIf);

  // These methods throw an error with this implementation of TDecBinIf, since
  //   it isn't backed by a TDecBinCABAC.
        TDecBinCABAC* getTDecBinCABAC();
  const TDecBinCABAC* getTDecBinCABAC() const;
};


//! \}
