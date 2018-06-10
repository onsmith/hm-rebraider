#include "TTraPictureBuffer.h"


/**
 * Constructor with initial capacity
 */
TTraPictureBuffer::TTraPictureBuffer() {
}


/**
 * Virtual destructor
 */
TTraPictureBuffer::~TTraPictureBuffer() {
  clear();
}


/**
 * Get a reference to the internal list
 */
std::list<TComPic*>& TTraPictureBuffer::list() {
  return m_list;
}


/**
 * Get a const reference to the internal list
 */
const std::list<TComPic*>& TTraPictureBuffer::list() const {
  return m_list;
}


/**
 * Find TComPic by POC
 */
TComPic* TTraPictureBuffer::findByPOC(Int poc) {
  for (auto it = m_list.begin(); it != m_list.end(); it++) {
    TComPic* pPic = *it;
    if (pPic != nullptr && pPic->getPOC() == poc) {
      return pPic;
    }
  }
  return nullptr;
}



/**
 * Get or make a TComPic from the picture buffer
 */
TComPic& TTraPictureBuffer::getUnusedPicture() {
  
}


/**
 * Clear picture buffer
 */
Void TTraPictureBuffer::clear() {
  for (auto it = m_list.begin(); it != m_list.end(); it++) {
    TComPic* pPic = *it;
    if (pPic != nullptr) {
      pPic->destroy();
      delete pPic;
      pPic = nullptr;
    }
  }
  m_list.clear();
}
