#include "TDbrEntropy.h"




/**
 * Helper methods to write codes to the output bitstreams
 */
Void TDbrEntropy::xWriteCode(TComBitIf& bitstream, UInt uiCode, UInt uiLength, const string& name) {
  assert (uiLength > 0);
  bitstream.write(uiCode, uiLength);
}


Void TDbrEntropy::xWriteUvlc(TComBitIf& bitstream, UInt uiCode, const string& name) {
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;

  assert(uiTemp);

  while (1 != uiTemp) {
    uiTemp >>= 1;
    uiLength += 2;
  }

  // Take care of cases where uiLength > 32
  bitstream.write(0, uiLength >> 1);
  bitstream.write(uiCode, (uiLength + 1) >> 1);
}


Void TDbrEntropy::xWriteSvlc(TComBitIf& bitstream, Int iCode, const string& name) {
  xWriteUvlc(bitstream, xConvertToUInt(iCode), name);
}


Void TDbrEntropy::xWriteFlag(TComBitIf& bitstream, UInt uiCode, const string& name) {
  bitstream.write(uiCode, 1);
}


UInt TDbrEntropy::xConvertToUInt(Int iValue) {
  return (iValue <= 0) ? -iValue << 1 : (iValue << 1) - 1;
}




/**
  * Bitstream control methods
  */
Void TDbrEntropy::resetEntropy(const TComSlice *pSlice) {

}


SliceType TDbrEntropy::determineCabacInitIdx(const TComSlice *pSlice) {

}


Void TDbrEntropy::setBitstream(TComBitIf* p) {

}


Void TDbrEntropy::resetBits() {

}


UInt TDbrEntropy::getNumberOfWrittenBits() {

}




/**
 * Hard parameters
 */
Void TDbrEntropy::xCodeHrdParameters(TComBitIf& bitstream, const TComHRD *hrd, Bool commonInfPresentFlag, UInt maxNumSubLayersMinus1) {
  if(commonInfPresentFlag) {
    xWriteFlag(bitstream, hrd->getNalHrdParametersPresentFlag() ? 1 : 0, "nal_hrd_parameters_present_flag");
    xWriteFlag(bitstream, hrd->getVclHrdParametersPresentFlag() ? 1 : 0, "vcl_hrd_parameters_present_flag");
    if (hrd->getNalHrdParametersPresentFlag() || hrd->getVclHrdParametersPresentFlag()) {
      xWriteFlag(bitstream, hrd->getSubPicCpbParamsPresentFlag() ? 1 : 0, "sub_pic_hrd_params_present_flag");
      if (hrd->getSubPicCpbParamsPresentFlag()) {
        xWriteCode(bitstream, hrd->getTickDivisorMinus2(), 8, "tick_divisor_minus2" );
        xWriteCode(bitstream, hrd->getDuCpbRemovalDelayLengthMinus1(), 5, "du_cpb_removal_delay_increment_length_minus1");
        xWriteFlag(bitstream, hrd->getSubPicCpbParamsInPicTimingSEIFlag() ? 1 : 0, "sub_pic_cpb_params_in_pic_timing_sei_flag");
        xWriteCode(bitstream, hrd->getDpbOutputDelayDuLengthMinus1(), 5, "dpb_output_delay_du_length_minus1");
      }

      xWriteCode(bitstream, hrd->getBitRateScale(), 4, "bit_rate_scale");
      xWriteCode(bitstream, hrd->getCpbSizeScale(), 4, "cpb_size_scale");
      if (hrd->getSubPicCpbParamsPresentFlag()) {
        xWriteCode(bitstream, hrd->getDuCpbSizeScale(), 4, "du_cpb_size_scale");
      }
      xWriteCode(bitstream, hrd->getInitialCpbRemovalDelayLengthMinus1(), 5, "initial_cpb_removal_delay_length_minus1");
      xWriteCode(bitstream, hrd->getCpbRemovalDelayLengthMinus1(), 5, "au_cpb_removal_delay_length_minus1");
      xWriteCode(bitstream, hrd->getDpbOutputDelayLengthMinus1(), 5, "dpb_output_delay_length_minus1");
    }
  }
  Int i, j, nalOrVcl;
  for(i = 0; i <= maxNumSubLayersMinus1; i++) {
    xWriteFlag(bitstream, hrd->getFixedPicRateFlag(i) ? 1 : 0, "fixed_pic_rate_general_flag");
    Bool fixedPixRateWithinCvsFlag = true;

    if (!hrd->getFixedPicRateFlag(i)) {
      fixedPixRateWithinCvsFlag = hrd->getFixedPicRateWithinCvsFlag(i);
      xWriteFlag(bitstream, hrd->getFixedPicRateWithinCvsFlag(i) ? 1 : 0, "fixed_pic_rate_within_cvs_flag");
    }

    if (fixedPixRateWithinCvsFlag) {
      xWriteUvlc(bitstream, hrd->getPicDurationInTcMinus1(i), "elemental_duration_in_tc_minus1");
    } else {
      xWriteFlag(bitstream, hrd->getLowDelayHrdFlag(i) ? 1 : 0, "low_delay_hrd_flag");
    }

    if (!hrd->getLowDelayHrdFlag(i)) {
      xWriteUvlc(bitstream, hrd->getCpbCntMinus1(i), "cpb_cnt_minus1");
    }

    for (nalOrVcl = 0; nalOrVcl < 2; nalOrVcl++) {
      if (nalOrVcl == 0 && hrd->getNalHrdParametersPresentFlag() ||
          nalOrVcl == 1 && hrd->getVclHrdParametersPresentFlag()) {
        for (j = 0; j <= hrd->getCpbCntMinus1(i); j++) {
          xWriteUvlc(bitstream, hrd->getBitRateValueMinus1(i, j, nalOrVcl), "bit_rate_value_minus1");
          xWriteUvlc(bitstream, hrd->getCpbSizeValueMinus1(i, j, nalOrVcl), "cpb_size_value_minus1");
          if (hrd->getSubPicCpbParamsPresentFlag()) {
            xWriteUvlc(bitstream, hrd->getDuCpbSizeValueMinus1(i, j, nalOrVcl), "cpb_size_du_value_minus1");
            xWriteUvlc(bitstream, hrd->getDuBitRateValueMinus1(i, j, nalOrVcl), "bit_rate_du_value_minus1");
          }
          xWriteFlag(bitstream, hrd->getCbrFlag(i, j, nalOrVcl) ? 1 : 0, "cbr_flag");
        }
      }
    }
  }
}




/**
  * Parameter sets and headers
  */
Void TDbrEntropy::codeVPS(const TComVPS* pcVPS) {
  TComBitIf& bs = *vps_bitstream;

  xWriteCode(bs, pcVPS->getVPSId(), 4, "vps_video_parameter_set_id");
  xWriteFlag(bs, 1, "vps_base_layer_internal_flag");
  xWriteFlag(bs, 1, "vps_base_layer_available_flag");
  xWriteCode(bs, 0, 6, "vps_max_layers_minus1");
  xWriteCode(bs, pcVPS->getMaxTLayers() - 1, 3, "vps_max_sub_layers_minus1");
  xWriteFlag(bs, pcVPS->getTemporalNestingFlag(), "vps_temporal_id_nesting_flag");

  assert (pcVPS->getMaxTLayers()>1 || pcVPS->getTemporalNestingFlag());

  xWriteCode(bs, 0xffff, 16, "vps_reserved_0xffff_16bits");

  codePTL(pcVPS->getPTL(), true, pcVPS->getMaxTLayers() - 1);
  const Bool subLayerOrderingInfoPresentFlag = 1;
  xWriteFlag(bs, subLayerOrderingInfoPresentFlag, "vps_sub_layer_ordering_info_present_flag");

  for (UInt i = 0; i <= pcVPS->getMaxTLayers() - 1; i++) {
    xWriteUvlc(bs, pcVPS->getMaxDecPicBuffering(i) - 1, "vps_max_dec_pic_buffering_minus1[i]");
    xWriteUvlc(bs, pcVPS->getNumReorderPics(i), "vps_max_num_reorder_pics[i]");
    xWriteUvlc(bs, pcVPS->getMaxLatencyIncrease(i), "vps_max_latency_increase_plus1[i]");

    if (!subLayerOrderingInfoPresentFlag) {
      break;
    }
  }

  assert(pcVPS->getNumHrdParameters() <= MAX_VPS_NUM_HRD_PARAMETERS);
  assert(pcVPS->getMaxNuhReservedZeroLayerId() < MAX_VPS_NUH_RESERVED_ZERO_LAYER_ID_PLUS1);

  xWriteCode(bs, pcVPS->getMaxNuhReservedZeroLayerId(), 6, "vps_max_layer_id");
  xWriteUvlc(bs, pcVPS->getMaxOpSets() - 1, "vps_num_layer_sets_minus1");

  for(UInt opsIdx = 1; opsIdx <= pcVPS->getMaxOpSets() - 1; opsIdx++) {
    // Operation point set
    for(UInt i = 0; i <= pcVPS->getMaxNuhReservedZeroLayerId(); i++) {
      // Only applicable for version 1
      // pcVPS->setLayerIdIncludedFlag(true, opsIdx, i);
      xWriteFlag(bs, pcVPS->getLayerIdIncludedFlag(opsIdx, i) ? 1 : 0, "layer_id_included_flag[opsIdx][i]");
    }
  }
  const TimingInfo *timingInfo = pcVPS->getTimingInfo();

  xWriteFlag(bs, timingInfo->getTimingInfoPresentFlag(), "vps_timing_info_present_flag");

  if (timingInfo->getTimingInfoPresentFlag()) {
    xWriteCode(bs, timingInfo->getNumUnitsInTick(), 32, "vps_num_units_in_tick");
    xWriteCode(bs, timingInfo->getTimeScale(), 32, "vps_time_scale");
    xWriteFlag(bs, timingInfo->getPocProportionalToTimingFlag(), "vps_poc_proportional_to_timing_flag");

    if (timingInfo->getPocProportionalToTimingFlag()) {
      xWriteUvlc(bs, timingInfo->getNumTicksPocDiffOneMinus1(), "vps_num_ticks_poc_diff_one_minus1");
    }

    xWriteUvlc(bs, pcVPS->getNumHrdParameters(), "vps_num_hrd_parameters");

    if (pcVPS->getNumHrdParameters() > 0) {
      for (UInt i = 0; i < pcVPS->getNumHrdParameters(); i++) {
        // Only applicable for version 1
        xWriteUvlc(bs, pcVPS->getHrdOpSetIdx(i), "hrd_layer_set_idx");
        if (i > 0) {
          xWriteFlag(bs, pcVPS->getCprmsPresentFlag(i) ? 1 : 0, "cprms_present_flag[i]");
        }
        codeHrdParameters(pcVPS->getHrdParameters(i), pcVPS->getCprmsPresentFlag(i), pcVPS->getMaxTLayers() - 1);
      }
    }
  }

  xWriteFlag(bs, 0, "vps_extension_flag");

  //future extensions here..
  xWriteRbspTrailingBits();
}


Void TDbrEntropy::codeSPS(const TComSPS* pcSPS) {

}


Void TDbrEntropy::codePPS(const TComPPS* pcPPS) {

}


Void TDbrEntropy::codeSliceHeader(TComSlice* pcSlice) {

}




/**
  * 
  */
Void TDbrEntropy::codeTilesWPPEntryPoint(TComSlice* pSlice) {

}


Void TDbrEntropy::codeTerminatingBit(UInt uilsLast) {

}


Void TDbrEntropy::codeSliceFinish() {

}


Void TDbrEntropy::codeMVPIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {

}




/**
  * 
  */
Void TDbrEntropy::codeCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeSkipFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeMergeFlag(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeMergeIndex(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeSplitFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {

}




/**
  *
  */
Void TDbrEntropy::codePartSize(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {

}


Void TDbrEntropy::codePredMode(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}




/**
  *
  */
Void TDbrEntropy::codeIPCMInfo(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}




/**
  *
  */
Void TDbrEntropy::codeTransformSubdivFlag(UInt uiSymbol, UInt uiCtx) {

}


Void TDbrEntropy::codeQtCbf(TComTU &rTu, const ComponentID compID, const Bool lowestLevel) {

}


Void TDbrEntropy::codeQtRootCbf(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeQtCbfZero(TComTU &rTu, const ChannelType chType) {

}


Void TDbrEntropy::codeQtRootCbfZero() {

}


Void TDbrEntropy::codeIntraDirLumaAng(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool isMultiplePU) {

}




/**
  *
  */
Void TDbrEntropy::codeIntraDirChroma(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeInterDir(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeRefFrmIdx(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {

}


Void TDbrEntropy::codeMvd(TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList) {

}




/**
  *
  */
Void TDbrEntropy::codeCrossComponentPrediction(TComTU &rTu, ComponentID compID) {

}




/**
  *
  */
Void TDbrEntropy::codeDeltaQP(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeChromaQpAdjustment(TComDataCU* pcCU, UInt uiAbsPartIdx) {

}


Void TDbrEntropy::codeCoeffNxN(TComTU &rTu, TCoeff* pcCoef, const ComponentID compID) {

}


Void TDbrEntropy::codeTransformSkipFlags(TComTU &rTu, ComponentID component) {

}


Void TDbrEntropy::codeSAOBlkParam(SAOBlkParam& saoBlkParam, const BitDepths &bitDepths, Bool* sliceEnabled, Bool leftMergeAvail, Bool aboveMergeAvail, Bool onlyEstMergeInfo = false) {

}


Void TDbrEntropy::estBit(estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType, COEFF_SCAN_TYPE scanType) {

}


Void TDbrEntropy::codeExplicitRdpcmMode(TComTU &rTu, const ComponentID compID) {

}
