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
 *  \file     TDbrEntropy.h
 *  \project  TAppDebraider
 *  \brief    Debraider entropy class header
 */


#pragma once

#include "TDbrCabac.h"

#include "TLibEncoder/TEncEntropy.h"
#include "TLibEncoder/TEncCavlc.h"
#include "TLibEncoder/TEncSbac.h"


//! \ingroup TAppDebraider
//! \{


class TDbrEntropy : public TEncEntropyIf {
protected:
  /**
   * Decorates a context-adaptive variable-length code (cavlc) TEncEntropyIf
   *   object as well as a syntax-based binary arithmetic code (sbac) object.
   *   Delegates to these objects for all methods required by the TEncEntropyIf
   *   abstract class.
   */
  TEncCavlc cavlc;
  TEncSbac  sbac;


  /**
   * Encapsulates a context-adaptive binary arithmetic code (cabac) object which
   *   works as the middle man between the sbac object and the bitstream.
   *   Usually, this object holds the cabac contexts and internal state, but
   *   since this impelmentation just uses vlc for everything, there is no
   *   arithmetic coding applied.
   */
  TDbrCabac cabac;


  /**
   * The syntax elements used in hevc are grouped into classes for this class.
   *   Each group gets its own bitstream and therefore its own output file.
   */
  // Bits for encoding video parameter sets (vps)
  TComBitIf* vps_bitstream;

  // Bits for encoding sequence parameter sets (sps)
  TComBitIf* sps_bitstream;

  // Bits for encoding picture parameter sets (pps)
  TComBitIf* pps_bitstream;

  // Bits representing metadata of slice, including:
  //   - Slice headers
  //   - Slice endings
  //   - WPP tile entry points
  TComBitIf* slice_bitstream;

  // Bits for encoding delta quality parameter (dqp)
  TComBitIf* dqp_bitstream;

  // Bits for encoding quantized dct coefficients
  TComBitIf* coeff_bitstream;

  // Bits for encoding motion vector prediction (mvp) index
  TComBitIf* mvp_index_bitstream;

  // Bits for encoding transquant bypass
  TComBitIf* tq_bypass_bitstream;

  // Bits for encoding inter prediction cu skip flag
  TComBitIf* skip_flag_bitstream;

  // Bits for encoding inter prediction cu merge flag
  TComBitIf* merge_flag_bitstream;

  // Bits for encoding inter prediction cu merge index
  TComBitIf* merge_index_bitstream;

  // Bits for encoding cu quadtree split flag
  TComBitIf* split_flag_bitstream;

  // Bits for encoding partition size
  TComBitIf* part_size_bitstream;

  // Bits for encoding prediction mode (inter vs intra)
  TComBitIf* pred_mode_bitstream;

  // Bits for encoding intra pulse code modulation (ipcm) info
  TComBitIf* ipcm_bitstream;

  // Bits for encoding transform subdivision flag
  TComBitIf* tu_subdiv_flag_bitstream;

  // Bits for encoding quadtree (qt) coded block flag (cbf)
  TComBitIf* qt_cbf_bitstream;

  // Bits for encoding intra prediction mode for luma samples
  TComBitIf* intra_mode_luma_bitstream;

  // Bits for encoding intra prediction mode for chroma samples
  TComBitIf* intra_mode_chroma_bitstream;

  // Bits for encoding inter prediction direction
  TComBitIf* inter_dir_bitstream;

  // Bits for encoding reference frame index
  TComBitIf* ref_frame_index_bitstream;

  // Bits for encoding motion vector deltas (mvd)
  TComBitIf* mvd_bitstream;

  // Bits for encoding cross component prediction
  TComBitIf* cross_comp_pred_bitstream;

  // Bits for encoding chroma qp adjust
  TComBitIf* chroma_qp_adj_bitstream;

  // Bits for encoding transform skip flag
  TComBitIf* tu_skip_flag_bitstream;

  // Bits for encoding sample adaptive offset (sao) in-loop filter block params
  TComBitIf* sao_blk_param_bitstream;

  // Bits for encoding residual differential pulse code modulation (rdpcm)
  TComBitIf* rdpcm_bitstream;



public:
  // Default constructor passes the cabac object to the sbac object
  TDbrEntropy();

  // Default destructor
  ~TDbrEntropy() = default;


  /**
   * Interface bitstream control methods
   * Note: These methods don't really do anything; they're just here to fulfill
   *   the requirements of TEncEntropyIf.
   */
  Void resetEntropy(const TComSlice *pSlice);
  SliceType determineCabacInitIdx(const TComSlice *pSlice);
  Void setBitstream(TComBitIf* p);
  Void resetBits();
  UInt getNumberOfWrittenBits();


  /**
   * Parameter sets
   */
  // Video parameter set
  Void codeVPS(const TComVPS* pcVPS);

  // Sequence parameter set
  Void codeSPS(const TComSPS* pcSPS);

  // Picture parameter set
  Void codePPS(const TComPPS* pcPPS);


  /**
   * Slice information
   */
  // Slice header
  Void codeSliceHeader(TComSlice* pcSlice);

  // End of slice
  Void codeSliceFinish();


  /**
   * TODO: Write comments for these
   */
  Void codeTilesWPPEntryPoint(TComSlice* pSlice);
  Void codeTerminatingBit(UInt uilsLast);
  Void codeMVPIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList);
  Void codeCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeMergeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void codePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void codePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeTransformSubdivFlag(UInt uiSymbol, UInt uiCtx);
  Void codeQtCbf(TComTU &rTu, const ComponentID compID, const Bool lowestLevel);
  Void codeQtRootCbf(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeQtCbfZero(TComTU &rTu, const ChannelType chType);
  Void codeQtRootCbfZero();
  Void codeIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool isMultiplePU);
  Void codeIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeRefFrmIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList);
  Void codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList);
  Void codeCrossComponentPrediction(TComTU &rTu, ComponentID compID);
  Void codeDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx);
  Void codeCoeffNxN(TComTU &rTu, TCoeff* pcCoef, const ComponentID compID);
  Void codeTransformSkipFlags(TComTU &rTu, ComponentID component);
  Void codeSAOBlkParam(SAOBlkParam& saoBlkParam, const BitDepths &bitDepths, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, Bool onlyEstMergeInfo = false);
  Void estBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType, COEFF_SCAN_TYPE scanType);
  Void codeExplicitRdpcmMode(TComTU &rTu, const ComponentID compID);


private:
  /**
   * Helper methods to write codes to the output bitstreams
   */
  // Writes a codeword to a bitstream
  Void xWriteCode(TComBitIf& bitstream, UInt uiCode, UInt uiLength, const string& name);

  // Writes an unsigned integer to a bitstream
  Void xWriteUvlc(TComBitIf& bitstream, UInt uiCode, const string& name);

  // Writes an signed integer to a bitstream
  Void xWriteSvlc(TComBitIf& bitstream, Int iCode, const string& name);

  // Writes a boolean flag to a bitstream
  Void xWriteFlag(TComBitIf& bitstream, UInt uiCode, const string& name);

  // Writes aligning bits to a bitstream
  Void xWriteRbspTrailingBits(TComBitIf& bitstream);

  // Converts a signed integer to an unsigned integer
  UInt xConvertToUInt(Int iValue) const;
};


//! \}
