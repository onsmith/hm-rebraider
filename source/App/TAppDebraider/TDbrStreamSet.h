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
 *  \file     TDbrStreamSet.h
 *  \project  TAppDebraider
 *  \brief    Debraider stream set class header
 */


#pragma once

#include <vector>
#include <fstream>

#include "TLibCommon/TComBitStream.h"
#include "TLibDecoder/NALread.h"


//! \ingroup TAppDebraider
//! \{


class TDbrStreamSet {
public:
  // Number of streams in the set
  static const UInt NUM_STREAMS = 33;

  // Stream index
  enum class STREAM {
    CHUNKS = 0,        // Bits for encoding number of bytes in each stream for each nal unit
    NALU,              // Bits for encoding NAL unit header values
    VPS,               // Bits for encoding video parameter sets (vps)
    SPS,               // Bits for encoding sequence parameter sets (sps)
    PPS,               // Bits for encoding picture parameter sets (pps)
    SLICE,             // Bits for encoding metadata of slice, including:
                       //   - Slice headers
                       //   - Slice endings
                       //   - WPP tile entry points
    DQP,               // Bits for encoding delta quality parameter (dqp)
    COEFF,             // Bits for encoding quantized dct coefficients
    MVP_INDEX,         // Bits for encoding motion vector prediction (mvp) index
    TQ_BYPASS_FLAG,    // Bits for encoding transquant bypass
    SKIP_FLAG,         // Bits for encoding inter prediction cu skip flag
    MERGE_FLAG,        // Bits for encoding inter prediction cu merge flag
    MERGE_INDEX,       // Bits for encoding inter prediction cu merge index
    SPLIT_FLAG,        // Bits for encoding cu quadtree split flag
    PART_SIZE,         // Bits for encoding partition size
    PRED_MODE,         // Bits for encoding prediction mode (inter vs intra)
    IPCM,              // Bits for encoding intra pulse code modulation (ipcm) info
    TU_SUBDIV_FLAG,    // Bits for encoding transform subdivision flag
    QT_CBF,            // Bits for encoding quadtree (qt) coded block flag (cbf)
    INTRA_DIR_LUMA,    // Bits for encoding intra prediction mode for luma samples
    INTRA_DIR_CHROMA,  // Bits for encoding intra prediction mode for chroma samples
    INTER_DIR,         // Bits for encoding inter prediction direction
    REF_FRAME_INDEX,   // Bits for encoding reference frame index
    MVD,               // Bits for encoding motion vector deltas (mvd)
    CROSS_COMP_PRED,   // Bits for encoding cross component prediction
    CHROMA_QP_ADJ,     // Bits for encoding chroma qp adjust
    TU_SKIP_FLAG,      // Bits for encoding transform skip flag
    SAO_PARAMS,        // Bits for encoding various sample adaptive offset (sao) in-loop filter params
    SAO_BLOCK_PARAMS,  // Bits for encoding sample adaptive offset (sao) in-loop filter block params
    LAST_SIG_XY,       // Bits for encoding last significant (x, y) coordinate for dct coefficients
    SCALING_LIST,      // Bits for encoding scaling list
    RDPCM,             // Bits for encoding residual differential pulse code modulation (rdpcm)
    OTHER              // Bits for encoding undecoded NAL units (SEI, etc)
  };

  // Stream name strings
  static const std::string STREAM_NAMES[NUM_STREAMS];


private:
  // Stores the bitstreams in a vector
  std::vector<TComInputBitstream> bitstreams;

  // Stores corresponding ifstreams in a vector
  std::vector<std::ifstream> ifstreams;


public:
  // Default constructor
  TDbrStreamSet();

  // Opens all bitstreams
  Void open(const std::string& basename);

  // Reads the next nal unit into the bitstreams
  Void readNextNalUnit(InputNALUnit& nalu);

  // Bitstream access
        TComInputBitstream& getBitstream(STREAM i);
  const TComInputBitstream& getBitstream(STREAM i) const;
        TComInputBitstream& getBitstream(Int i);
  const TComInputBitstream& getBitstream(Int i) const;

  // Ofstream access
        std::ifstream& getIfstream(STREAM i);
  const std::ifstream& getIfstream(STREAM i) const;
        std::ifstream& getIfstream(Int i);
  const std::ifstream& getIfstream(Int i) const;

  // Returns true if there is another nal unit in the hevc stream
  Bool hasAnotherNalUnit() const;

  // Returns true if one or more ifstreams has failed
  Bool fail() const;

  // Rewinds the bitstream
  Void rewind();


private:
  // Reads a nal unit header from the special NALU stream
  Void xReadNextNalUnitHeader(InputNALUnit& nalu);

  // Read the next stream chunk length from the special CHUNKS stream
  UInt xReadNextChunkLength();

  // Reads a specified number of bytes from an ifstream into the corresponding
  //   bitstream
  Void xReadBytesIntoBitstream(STREAM stream, UInt numBytes);
};


//! \}
