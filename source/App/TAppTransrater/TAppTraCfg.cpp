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
 *  \file     TAppTraCfg.cpp
 *  \project  TAppTransrater
 *  \brief    Transrater configuration class implementation
 */


#include <cstdio>
#include <cstring>
#include <string>
#include "TAppTraCfg.h"
#include "TAppCommon/program_options_lite.h"
#include "TLibCommon/TComChromaFormat.h"


#ifdef WIN32
#define strdup _strdup
#endif


using namespace std;
namespace po = df::program_options_lite;


//! \ingroup TAppTransrater
//! \{


/**
 * Default constructor
 */
TAppTraCfg::TAppTraCfg() :
  m_bitstreamFileName(),
  m_reconFileName(),
  m_iSkipFrame(0),
  m_outputColourSpaceConvert(IPCOLOURSPACE_UNCHANGED),
  m_iMaxTemporalLayer(-1),
  m_decodedPictureHashSEIEnabled(0),
  m_decodedNoDisplaySEIEnabled(false),
  m_colourRemapSEIFileName(),
  m_targetDecLayerIdSet(),
  m_respectDefDispWindow(0),
  m_outputDecodedSEIMessagesFilename(),
#if O0043_BEST_EFFORT_DECODING
  m_forceDecodeBitDepth(0),
#endif
#if MCTS_ENC_CHECK
  m_tmctsCheck(false),
#endif
  m_bClipOutputVideoToRec709Range(false) {
  for (UInt c = 0; c < MAX_NUM_CHANNEL_TYPE; c++) {
    m_outputBitDepth[c] = 0;
  }
}


/**
 * Destructor
 */
TAppTraCfg::~TAppTraCfg() {
}


/**
 * Parses command line arguments
 *
 * \param argc  number of command line arguments
 * \param argv  array of command line arguments
 */
Bool TAppTraCfg::parseCfg(Int argc, TChar* argv[]) {
  Bool   do_help = false;
  Int    warnUnknownParameter = 0;
  string cfg_TargetDecLayerIdSetFile;
  string outputColourSpaceConvert;

  // Initialize options structure
  po::Options opts;

  // Specify permitted command line arguments in options structure
  opts.addOptions()
    ("help",                             do_help,                               false,      "this help text")
    ("BitstreamFile,b",                  m_bitstreamFileName,                   string(""), "bitstream input file name")
    ("ReconFile,o",                      m_reconFileName,                       string(""), "reconstructed YUV output file name\n"
                                                                                            "YUV writing is skipped if omitted")
    ("WarnUnknownParameter,w",           warnUnknownParameter,                  0,          "warn for unknown configuration parameters instead of failing")
    ("SkipFrames,s",                     m_iSkipFrame,                          0,          "number of frames to skip before random access")
    ("OutputBitDepth,d",                 m_outputBitDepth[CHANNEL_TYPE_LUMA],   0,          "bit depth of YUV output luma component (default: use 0 for native depth)")
    ("OutputBitDepthC,d",                m_outputBitDepth[CHANNEL_TYPE_CHROMA], 0,          "bit depth of YUV output chroma component (default: use 0 for native depth)")
    ("OutputColourSpaceConvert",         outputColourSpaceConvert,              string(""), "Colour space conversion to apply to input 444 video. Permitted values are (empty string=UNCHANGED) " + getListOfColourSpaceConverts(false))
    ("MaxTemporalLayer,t",               m_iMaxTemporalLayer,                   -1,         "Maximum Temporal Layer to be decoded. -1 to decode all layers")
    ("SEIDecodedPictureHash",            m_decodedPictureHashSEIEnabled,        1,          "Control handling of decoded picture hash SEI messages\n"
                                                                                            "\t1: check hash in SEI messages if available in the bitstream\n"
                                                                                            "\t0: ignore SEI message")
    ("SEINoDisplay",                     m_decodedNoDisplaySEIEnabled,          true,       "Control handling of decoded no display SEI messages")
    ("TarDecLayerIdSetFile,l",           cfg_TargetDecLayerIdSetFile,           string(""), "targetDecLayerIdSet file name. The file should include white space separated LayerId values to be decoded. Omitting the option or a value of -1 in the file decodes all layers.")
    ("RespectDefDispWindow,w",           m_respectDefDispWindow,                0,          "Only output content inside the default display window\n")
    ("SEIColourRemappingInfoFilename",   m_colourRemapSEIFileName,              string(""), "Colour Remapping YUV output file name. If empty, no remapping is applied (ignore SEI message)\n")
#if O0043_BEST_EFFORT_DECODING
    ("ForceDecodeBitDepth",              m_forceDecodeBitDepth,                 0U,         "Force the decoder to operate at a particular bit-depth (best effort decoding)")
#endif
    ("OutputDecodedSEIMessagesFilename", m_outputDecodedSEIMessagesFilename,    string(""), "When non empty, output decoded SEI messages to the indicated file. If file is '-', then output to stdout\n")
    ("ClipOutputVideoToRec709Range",     m_bClipOutputVideoToRec709Range,       false,      "If true then clip output video to the Rec. 709 Range on saving")
#if MCTS_ENC_CHECK
    ("TMCTSCheck",                       m_tmctsCheck,                          false,      "If enabled, the decoder checks for violations of mc_exact_sample_value_match_flag in Temporal MCTS ")
#endif
  ;

  po::setDefaults(opts);
  po::ErrorReporter err;

  // Scan for unhandled arguments
  {
    const list<const TChar*>& argv_unhandled =
      po::scanArgv(opts, argc, (const TChar**) argv, err);

    for (auto it = argv_unhandled.begin(); it != argv_unhandled.end(); it++) {
      fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
    }
  }

  // do_help
  if (argc == 1 || do_help) {
    po::doHelp(cout, opts);
    return false;
  }

  // Terminate early if unknown parameters were found
  if (err.is_errored && !warnUnknownParameter) {
    return false;
  }

  // m_outputColourSpaceConvert
  {
    m_outputColourSpaceConvert =
      stringToInputColourSpaceConvert(outputColourSpaceConvert, false);

    if (m_outputColourSpaceConvert >= NUMBER_INPUT_COLOUR_SPACE_CONVERSIONS) {
      fprintf(stderr, "Bad output colour space conversion string\n");
      return false;
    }
  }

  // m_bitstreamFileName
  if (m_bitstreamFileName.empty()) {
    fprintf(stderr, "No input file specified, aborting\n");
    return false;
  }

  // cfg_TargetDecLayerIdSetFile
  if (!cfg_TargetDecLayerIdSetFile.empty()) {
    FILE* layerIdConfigFile = fopen(cfg_TargetDecLayerIdSetFile.c_str(), "r");

    if (layerIdConfigFile) {
      Bool isLayerIdZeroIncluded = false;
      while (!feof(layerIdConfigFile)) {
        // Read next layerId from bitstream
        Int layerId = 0;
        if (fscanf(layerIdConfigFile, "%d ", &layerId) != 1) {
          if (m_targetDecLayerIdSet.size() == 0) {
            fprintf(
              stderr,
              "No LayerId could be parsed in file %s. Decoding all LayerIds as default.\n",
              cfg_TargetDecLayerIdSetFile.c_str()
            );
          }
          break;
        }

        // If layerId is -1, then all layerIds should be decoded
        if (layerId == -1) {
          m_targetDecLayerIdSet.clear();
          break;
        }

        // Ensure the layerId is in bounds
        if (layerId < 0 || layerId >= MAX_NUM_LAYER_IDS) {
          fprintf(
            stderr,
            "Warning! Parsed LayerId %d is not within allowed range [0,%d]. Ignoring this value.\n",
            layerId,
            MAX_NUM_LAYER_IDS - 1
          );
          continue;
        }

        // Add the layerId to the decode list
        m_targetDecLayerIdSet.push_back(layerId);
        isLayerIdZeroIncluded = isLayerIdZeroIncluded || layerId == 0;
      }

      fclose(layerIdConfigFile);

      // Ensure layerId 0 was included for decoding
      if (m_targetDecLayerIdSet.size() > 0 && !isLayerIdZeroIncluded) {
        fprintf(stderr, "TargetDecLayerIdSet must contain LayerId=0, aborting");
        return false;
      }
    } else {
      fprintf(
        stderr,
        "File %s could not be opened. Using all LayerIds as default.\n",
        cfg_TargetDecLayerIdSetFile.c_str()
      );
    }
  }

  // Successful parse
  return true;
}


//! \}
