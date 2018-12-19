#include "TDbrEntropy.h"




/**
 * Helper methods to write codes to the output bitstreams
 */
Void TDbrEntropy::xWriteCode(TComBitIf& bitstream, UInt uiCode, UInt uiLength, const string& /* name */) {
  assert (uiLength > 0);
  bitstream.write(uiCode, uiLength);
}


Void TDbrEntropy::xWriteUvlc(TComBitIf& bitstream, UInt uiCode, const string& /* name */) {
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


Void TDbrEntropy::xWriteFlag(TComBitIf& bitstream, UInt uiCode, const string& /* name*/ ) {
  bitstream.write(uiCode, 1);
}


Void TDbrEntropy::xWriteRbspTrailingBits(TComBitIf& bitstream) {
  xWriteFlag(bitstream, 1, "rbsp_stop_one_bit");

  Int cnt = 0;
  while (bitstream.getNumBitsUntilByteAligned()) {
    xWriteFlag(bitstream, 0, "rbsp_alignment_zero_bit");
    cnt++;
  }

  assert(cnt < 8);
}


UInt TDbrEntropy::xConvertToUInt(Int iValue) const {
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
 * Codes profile tier level (PTL) info (for VPS)
 */
Void TDbrEntropy::xCodePTL(TComBitIf& bitstream, const TComPTL* pcPTL, Bool profilePresentFlag, Int maxNumSubLayersMinus1) {
  if (profilePresentFlag) {
    xCodeProfileTier(bitstream, pcPTL->getGeneralPTL(), false);
  }

  xWriteCode(bitstream, Int(pcPTL->getGeneralPTL()->getLevelIdc()), 8, "general_level_idc");

  for (Int i = 0; i < maxNumSubLayersMinus1; i++) {
    xWriteFlag(bitstream, pcPTL->getSubLayerProfilePresentFlag(i), "sub_layer_profile_present_flag[i]");
    xWriteFlag(bitstream, pcPTL->getSubLayerLevelPresentFlag(i),   "sub_layer_level_present_flag[i]");
  }

  if (maxNumSubLayersMinus1 > 0) {
    for (Int i = maxNumSubLayersMinus1; i < 8; i++) {
      xWriteCode(bitstream, 0, 2, "reserved_zero_2bits");
    }
  }

  for (Int i = 0; i < maxNumSubLayersMinus1; i++) {
    if (pcPTL->getSubLayerProfilePresentFlag(i)) {
      xCodeProfileTier(bitstream, pcPTL->getSubLayerPTL(i), true);
    }

    if (pcPTL->getSubLayerLevelPresentFlag(i)) {
      xWriteCode(bitstream, Int(pcPTL->getSubLayerPTL(i)->getLevelIdc()), 8, "sub_layer_level_idc[i]");
    }
  }
}



/**
 * Profile tier level (PTL) helper
 */
Void TDbrEntropy::xCodeProfileTier(TComBitIf& bitstream, const ProfileTierLevel* ptl, const Bool /* bIsSubLayer */) {
  xWriteCode(bitstream, ptl->getProfileSpace(), 2 , "profile_space");
  xWriteFlag(bitstream, ptl->getTierFlag()==Level::HIGH, "tier_flag");
  xWriteCode(bitstream, Int(ptl->getProfileIdc()), 5 , "profile_idc");

  for(Int j = 0; j < 32; j++) {
    xWriteFlag(bitstream, ptl->getProfileCompatibilityFlag(j), "profile_compatibility_flag[][j]");
  }

  xWriteFlag(bitstream, ptl->getProgressiveSourceFlag(), "progressive_source_flag");
  xWriteFlag(bitstream, ptl->getInterlacedSourceFlag(), "interlaced_source_flag");
  xWriteFlag(bitstream, ptl->getNonPackedConstraintFlag(), "non_packed_constraint_flag");
  xWriteFlag(bitstream, ptl->getFrameOnlyConstraintFlag(), "frame_only_constraint_flag");

  if (ptl->getProfileIdc() == Profile::MAINREXT || ptl->getProfileIdc() == Profile::HIGHTHROUGHPUTREXT) {
    const UInt bitDepthConstraint = ptl->getBitDepthConstraint();
    xWriteFlag(bitstream, bitDepthConstraint <= 12, "max_12bit_constraint_flag");
    xWriteFlag(bitstream, bitDepthConstraint <= 10, "max_10bit_constraint_flag");
    xWriteFlag(bitstream, bitDepthConstraint <= 8, "max_8bit_constraint_flag");

    const ChromaFormat chromaFmtConstraint = ptl->getChromaFormatConstraint();
    xWriteFlag(bitstream, chromaFmtConstraint == CHROMA_422 || chromaFmtConstraint == CHROMA_420 || chromaFmtConstraint == CHROMA_400, "max_422chroma_constraint_flag");
    xWriteFlag(bitstream, chromaFmtConstraint == CHROMA_420 || chromaFmtConstraint == CHROMA_400, "max_420chroma_constraint_flag");
    xWriteFlag(bitstream, chromaFmtConstraint == CHROMA_400, "max_monochrome_constraint_flag");

    xWriteFlag(bitstream, ptl->getIntraConstraintFlag(), "intra_constraint_flag");
    xWriteFlag(bitstream, ptl->getOnePictureOnlyConstraintFlag(), "one_picture_only_constraint_flag");
    xWriteFlag(bitstream, ptl->getLowerBitRateConstraintFlag(), "lower_bit_rate_constraint_flag");
    xWriteCode(bitstream, 0, 16, "reserved_zero_34bits[0..15]");
    xWriteCode(bitstream, 0, 16, "reserved_zero_34bits[16..31]");
    xWriteCode(bitstream, 0,  2, "reserved_zero_34bits[32..33]");
  } else if (ptl->getProfileIdc() == Profile::MAIN10) {
    xWriteCode(bitstream, 0x00, 7, "reserved_zero_7bits");
    xWriteFlag(bitstream, ptl->getOnePictureOnlyConstraintFlag(), "one_picture_only_constraint_flag");
    xWriteCode(bitstream, 0x0000, 16, "reserved_zero_35bits[0..15]");
    xWriteCode(bitstream, 0x0000, 16, "reserved_zero_35bits[16..31]");
    xWriteCode(bitstream, 0x0, 3, "reserved_zero_35bits[32..34]");
  } else {
    xWriteCode(bitstream, 0x0000, 16, "reserved_zero_43bits[0..15]");
    xWriteCode(bitstream, 0x0000, 16, "reserved_zero_43bits[16..31]");
    xWriteCode(bitstream, 0x000, 11, "reserved_zero_43bits[32..42]");
  }

  xWriteFlag(bitstream, false, "inbld_flag");
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

  xCodePTL(bs, pcVPS->getPTL(), true, pcVPS->getMaxTLayers() - 1);
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
        xCodeHrdParameters(bs, pcVPS->getHrdParameters(i), pcVPS->getCprmsPresentFlag(i), pcVPS->getMaxTLayers() - 1);
      }
    }
  }

  xWriteFlag(bs, 0, "vps_extension_flag");

  // Future extensions here..

  xWriteRbspTrailingBits(bs);
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
