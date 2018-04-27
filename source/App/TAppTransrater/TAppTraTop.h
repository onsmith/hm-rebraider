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
 *  \file     TAppTraTop.h
 *  \project  TAppTransrater
 *  \brief    Transrater application class header
 */


#ifndef __TAPPTRATOP__
#define __TAPPTRATOP__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "TLibVideoIO/TVideoIOYuv.h"
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPicYuv.h"
#include "TLibDecoder/TDecTop.h"
#include "TLibEncoder/TEncTop.h"
#include "TAppTraCfg.h"


//! \ingroup TAppTransrater
//! \{


class TAppTraTop : public TAppTraCfg {
private:
  // Internal encoder and decoder objects
  TEncTop m_encoder;
  TDecTop m_decoder;

  // Output YUV bitstream (DEPRECATED)
  TVideoIOYuv m_cTVideoIOYuvReconFile;

  // output control
  Int           m_iPOCLastDisplay;      // last POC in display order
  std::ofstream m_seiMessageFileStream; // Used for outputing SEI messages.

  SEIColourRemappingInfo* m_pcSeiColourRemappingInfoPrevious;


public:
  // Default constructor
  TAppTraTop();

  // Destructor
  virtual ~TAppTraTop();

  // Main transrating function
  Void transrate();

  // Retrieves the number of decoding errors encountered
  UInt getNumberOfChecksumErrorsDetected() const;


protected:
  // Initializes internal decoder object
  Void xCreateDecoder();
  
  // Destroys internal decoder object
  Void xDestroyDecoder();

  Void xWriteOutput(TComList<TComPic*>* pcListPic , UInt tId); //< write YUV to file
  Void xFlushOutput(TComList<TComPic*>* pcListPic);            //< flush all remaining decoded pictures to file

  // Checks whether a given layerId should be decoded
  Bool xWillDecodeLayer(Int layerId) const;

  // Checks whether all layerIds should be decoded
  Bool xWillDecodeAllLayers() const;

  // Opens an output stream for reporting decoded SEI messages.
  Void xOpenSEIOutputStream();

  // Sets the output bit depths
  Void xSetOutputBitDepths(const BitDepths& bitDepths);

  // Writes a frame to the output bitstream
  Void xWriteFrameToOutput(TComPic* pic);

  // Writes two fields to the output bitstream
  Void xWriteFieldToOutput(TComPic* field1, TComPic* field2);

private:
  Void applyColourRemapping(const TComPicYuv& pic, SEIColourRemappingInfo& pCriSEI, const TComSPS &activeSPS);
  Void xOutputColourRemapPic(TComPic* pcPic);
};


//! \}


#endif
