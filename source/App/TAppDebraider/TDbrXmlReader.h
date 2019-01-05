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
 *  \file     TDbrXmlReader.h
 *  \project  TAppDebraider
 *  \brief    Debraider XML reader class header
 */


#pragma once

#include <string>
#include <fstream>

#include "TLibCommon/CommonDef.h"
#include "TLibDecoder/NALread.h"


using std::string;
using std::istream;


//! \ingroup TAppDebraider
//! \{


class TDbrXmlReader {
private:
  // Stores an istream for reading xml
  std::istream* stream;


public:
  // Constructors
  TDbrXmlReader() = default;
  TDbrXmlReader(istream& stream);


  // Stream management
  istream* getStream();
  Void setStream(istream* stream);


  // Generic tag input
  string readOpenTag();
  string readOpenTag(const std::string& expectedTagName);

  string readCloseTag();
  string readCloseTag(const std::string& expectedTagName);


  // Nalu tag input
  string readOpenTag(NALUnit& nalu);


  // Value input
  Int readValueTag();
  Int readValueTag(const std::string& expectedTagName);


  // Checks if a string represents a tag
  static Bool isTag(const string& line);
  static Bool isOpenTag(const string& line, const string& expectedTagName);
  static Bool isCloseTag(const string& line, const string& expectedTagName);
  static Bool isOpenTag(const string& line);
  static Bool isCloseTag(const string& line);


  // Extracts a tag name
  static string extractTagName(const string& line);


  // Extracts the next complete tag as a string without moving the stream cursor
  string peekNextCompleteTag(const string& tagName);


  // Skips empty line(s) in the stream
  Void consumeEmptyLines();
};


//! \}
