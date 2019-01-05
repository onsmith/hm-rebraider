#include "TDbrXmlReader.h"
#include <regex>

using std::string;
using std::streampos;
using std::getline;
using std::size_t;
using std::regex;
using std::smatch;
using std::regex_search;
using std::stoi;


TDbrXmlReader::TDbrXmlReader(std::istream& stream) :
  stream(&stream) {
}


std::istream* TDbrXmlReader::getStream() {
  return stream;
}


Void TDbrXmlReader::setStream(std::istream* stream) {
  this->stream = stream;
}


static Void removeLineEnding(string& str) {
  if (str.back() == '\r') {
    str.erase(str.length() - 1);
  }
}


string TDbrXmlReader::readOpenTag() {
  string line;
  getline(*stream, line);
  removeLineEnding(line);
  assert(isOpenTag(line));
  return line;
}


string TDbrXmlReader::readOpenTag(const string& expectedTagName) {
  string line;
  getline(*stream, line);
  removeLineEnding(line);
  assert(isOpenTag(line, expectedTagName));
  return line;
}


string TDbrXmlReader::readCloseTag() {
  string line;
  getline(*stream, line);
  removeLineEnding(line);
  assert(isCloseTag(line));
  return line;
}


string TDbrXmlReader::readCloseTag(const string& expectedTagName) {
  string line;
  getline(*stream, line);
  removeLineEnding(line);
  assert(isCloseTag(line, expectedTagName));
  return line;
}


static const regex typeIdRegex("type=\"(\\d+)\"");
static const regex layerIdRegex("layer=\"(\\d+)\"");
static const regex temporalIdRegex("time=\"(\\d+)\"");


string TDbrXmlReader::readOpenTag(NALUnit& nalu) {
  string line = readOpenTag("nalu");
  smatch m;

  regex_search(line, m, typeIdRegex);
  assert(m.size() == 2);
  nalu.m_nalUnitType = static_cast<NalUnitType>(stoi(m[1]));

  regex_search(line, m, layerIdRegex);
  assert(m.size() == 2);
  nalu.m_nuhLayerId = stoi(m[1]);

  regex_search(line, m, temporalIdRegex);
  assert(m.size() == 2);
  nalu.m_temporalId = stoi(m[1]);

  return line;
}


Int TDbrXmlReader::readValueTag() {
  string line;
  getline(*stream, line);
  removeLineEnding(line);
  
  size_t beg = line.find('>', 0);
  assert(beg != string::npos);
  size_t end = line.find('<', beg + 1);
  assert(end != string::npos);
  return stoi(line.substr(beg + 1, end - beg - 1));
}


Int TDbrXmlReader::readValueTag(const std::string& /*expectedTagName*/) {
  // TODO: Make this assert() that the tag name is correct
  return readValueTag();
}


Bool TDbrXmlReader::isOpenTag(const std::string& line, const std::string& expectedTagName) {
  return (
    isOpenTag(line) &&
    line.compare(1, expectedTagName.length(), expectedTagName) == 0
  );
}


Bool TDbrXmlReader::isCloseTag(const std::string& line, const std::string& expectedTagName) {
  return (
    isCloseTag(line) &&
    line.compare(2, expectedTagName.length(), expectedTagName) == 0
  );
}


Bool TDbrXmlReader::isTag(const std::string& line) {
  return (
    line.length() >= 3 &&
    line.front() == '<' &&
    line.back()  == '>'
  );
}


Bool TDbrXmlReader::isOpenTag(const std::string& line) {
  return (
    isTag(line) &&
    line.at(1) != '/'
  );
}


Bool TDbrXmlReader::isCloseTag(const std::string& line) {
  return (
    line.length() >= 4 &&
    line.front() == '<' &&
    line.back()  == '>' &&
    line.at(1) == '/'
  );
}


string TDbrXmlReader::extractTagName(const string& line) {
  assert(isTag(line));
  size_t tagNameEndPos = line.find_first_of(" >");
  if (isOpenTag(line)) {
    return line.substr(1, tagNameEndPos - 1);
  } else if (isCloseTag(line)) {
    return line.substr(2, tagNameEndPos - 2);
  } else {
    // Shouldn't be able to get here, since we already know it's a tag
    assert(0);
  }
}


std::string TDbrXmlReader::peekNextCompleteTag(const string& tagName) {
  const streampos startpos = stream->tellg();
  string contents;
  string line;
  do {
    getline(*stream, line);
    removeLineEnding(line);
    contents += line;
  } while (!isCloseTag(line) || extractTagName(line) != tagName);
  stream->seekg(startpos);
  return contents;
}


Void TDbrXmlReader::consumeEmptyLines() {
  streampos startpos = stream->tellg();
  string line;
  while (getline(*stream, line) && line.size() == 0) {
    startpos = stream->tellg();
  }
  if (!stream->fail()) {
    stream->seekg(startpos);
  }
}
