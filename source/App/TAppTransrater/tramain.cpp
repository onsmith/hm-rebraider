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
 * \file     tramain.cpp
 * \project  TAppTransrater
 * \brief    Transrater application main
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "TAppTraTop.h"


using std::printf;


//! \ingroup TAppTransrater
//! \{


int main(int argc, char* argv[]) {
  // Output version information
  fprintf(stdout, "\n");
  fprintf(stdout, "HM software: Transrater Version [%s] (including RExt)", NV_VERSION);
  fprintf(stdout, NVM_ONOS );
  fprintf(stdout, NVM_COMPILEDBY);
  fprintf(stdout, NVM_BITS);
  fprintf(stdout, "\n");

  // Create transrater
  TAppDbrTop transrater;

  // Parse configuration
  if (!transrater.parseCfg(argc, argv)) {
    return EXIT_FAILURE;
  }

  // Capture starting time
  clock_t startTime = clock();

  // Transrate video
  transrater.transrate();

  // Capture ending time
  clock_t endTime = clock();

  // Output timing information
  Double elapsedTime = (Double) (endTime - startTime) / CLOCKS_PER_SEC;
  printf("\n Total Time: %12.3f sec.\n", elapsedTime);

  // Report errors
  Int returnCode = EXIT_SUCCESS;
  if (transrater.numDecodingErrorsDetected() != 0) {
    printf("\n\n***ERROR*** A decoding mismatch occured: signalled md5sum does not match\n");
    return EXIT_FAILURE;
  }

  // Terminate
  return returnCode;
}


//! \}
