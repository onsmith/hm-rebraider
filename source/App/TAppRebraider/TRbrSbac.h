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
 *  \file     TRbrSbac.h
 *  \project  TAppRebraider
 *  \brief    Rebraider SBAC entropy class header
 */


#pragma once

#include "TRbrBinVLC.h"
#include "TRbrStreamSet.h"

#include "TLibDecoder/TDecSbac.h"


//! \ingroup TAppRebraider
//! \{


class TRbrSbac : public TDecSbac {
protected:
  // Stores a set of debraided input bitstreams as a TRbrStreamSet object
  TRbrStreamSet* bitstreams;

  // Stores a binary reader to read non-arithmetic-coded bits
  TRbrBinVLC vlcReader;



public:
  // Default constructor
  TRbrSbac() = default;

  // Default destructor
  ~TRbrSbac() = default;


  // Bitstream set management
  Void setBitstreams(TRbrStreamSet* bitstreams);
  TRbrStreamSet* getBitstreams();


  // Override TDecEntropyIf virtual methods
  Void parseVPS(TComVPS* pcVPS);
  Void parseSPS(TComSPS* pcSPS);
  Void parsePPS(TComPPS* pcPPS);
  Void parseSliceHeader(TComSlice* pcSlice, ParameterSetManager* parameterSetManager, const Int prevTid0POC);
  Void parseTerminatingBit(UInt& ruilsLast);
  Void parseRemainingBytes(Bool noTrailingBytesExpected);
  Void parseMVPIdx(Int& riMVPIdx);
  Void parseSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx);
  Void parseMergeIndex(TComDataCU* pcCU, UInt& ruiMergeIndex);
  Void parsePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parsePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseInterDir(TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx);
  Void parseRefFrmIdx(TComDataCU* pcCU, Int& riRefFrmIdx, RefPicList eRefList);
  Void parseMvd(TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList);
  Void parseCrossComponentPrediction(TComTU& rTu, ComponentID compID);
  Void parseTransformSubdivFlag(UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize);
  Void parseQtCbf(TComTU& rTu, const ComponentID compID, const Bool lowestLevel);
  Void parseQtRootCbf(UInt uiAbsPartIdx, UInt& uiQtRootCbf);
  Void parseDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void parseCoeffNxN(TComTU& rTu, ComponentID compID);
  Void parseTransformSkipFlags(TComTU& rTu, ComponentID component);
  Void parseExplicitRdpcmMode(TComTU& rTu, ComponentID compID);
};


//! \}
