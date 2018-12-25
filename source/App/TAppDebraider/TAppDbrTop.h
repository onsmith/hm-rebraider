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
 *  \file     TAppDbrTop.h
 *  \project  TAppDebraider
 *  \brief    Debraider application class header
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
#include "TAppDbrCfg.h"

#include "TDbrTop.h"


using std::ifstream;
using std::ofstream;
using std::ostream;


//! \ingroup TAppDebraider
//! \{


class TAppDbrTop : public TAppDbrCfg {
private:
  // Internal decoder object
  TDecTop m_decoder;

  // Internal debraiding transcoder object
  TDbrTop m_transcoder;

  // The picture order count (POC) of the last frame that was output
  Int m_lastOutputPOC;

  // Output stream for reconstructed source YUV frames
  TVideoIOYuv m_decodedYUVOutputStream;

  // Holds transrated nal units for the current access unit
  AccessUnit m_currentAccessUnit;


public:
  // Default constructor
  TAppDbrTop();

  // Performs transrating
  Void debraid();

  // Gets the number of decoding errors detected
  UInt numDecodingErrorsDetected() const;


protected:
  /**
   * Configuration of decoder object
   */

  // Transfers the current configuration to the encoder object
  Void xConfigTranscoder();

  // Transfers the current configuration to the decoder object
  Void xConfigDecoder();


  /**
   * I/O stream management
   */

  // Opens an ifstream for reading the source hevc bitstream
  Void xOpenInputStream(ifstream& stream) const;

  // Opens an ofstream for writing the transrated hevc bitstream
  Void xOpenOutputStream(ofstream& stream) const;


  /**
   * Access Unit management
   */

  // Checks if the given nal unit signals the start of a new access unit
  Bool xIsFirstNalUnitOfNewAccessUnit(const NALUnit& nalu) const;

  // Writes the current access unit to the given bitstream and resets the
  //   current access unit list
  Void xFlushAccessUnit(ostream& stream);


  /**
   * Helper methods
   */

  // Checks whether a given layerId should be decoded
  Bool xWillDecodeLayer(Int layerId) const;

  // Checks whether all layerIds should be decoded
  Bool xWillDecodeAllLayers() const;

  // Overwrites the default configuration for output bit depth
  Void xSetOutputBitDepths(const BitDepths& bitDepths);

  // Re-encodes a NAL unit
  Void xEncodeUnit(const InputNALUnit& sourceNalu, OutputNALUnit& encodedNalu);

  // Directly copies a nal unit from a bitstream
  Void xCopyNaluBodyFromStream(InputNALUnit& nalu, const TComInputBitstream& bitstream) const;


  /**
   * Decoded picture buffer management
   */

  // Writes reconstructed frames in the decoded picture buffer to the output
  //   bitstream
  Void xDisplayDecodedFrames(TComList<TComPic*>* pcListPic);
  
  // Writes reconstructed frames in the decoded picture buffer to the output
  //   bitstream and flushes the buffer
  Void xFlushPictureBuffer(TComList<TComPic*>* pcListPic);

  // Writes a raw reconstructed frame to the output bitstream
  Void xWriteFrameToOutput(TComPic* frame);

  // Writes a raw reconstructed interlaced frame to the output bitstream
  Void xWriteFrameToOutput(TComPic* field1, TComPic* field2);
};


//! \}


#endif
