#include "TTraPictureBuffer.h"


/**
 * Default constructor
 */
TTraPictureBuffer::TTraPictureBuffer() :
  m_list() {
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
 * Find an existing TComPic by POC
 */
TComPic* TTraPictureBuffer::find(Int poc) {
  return const_cast<TComPic*>(
    static_cast<const TTraPictureBuffer*>(this)->find(poc)
  );
}


/**
 * Find an existing const TComPic by POC
 */
const TComPic* TTraPictureBuffer::find(Int poc) const {
  for (auto it = m_list.begin(); it != m_list.end(); it++) {
    TComPic* pPic = *it;
    if (pPic != nullptr && pPic->getPOC() == poc) {
      return pPic;
    }
  }
  return nullptr;
}



TComPic*& TTraPictureBuffer::getUnusedEntry() {
  // Linearly search the buffer for an existing unused entry
  for (auto it = m_list.begin(); it != m_list.end(); it++) {
    TComPic*& pPic = *it;

    if (pPic != nullptr && !pPic->getOutputMark()) {
      if (!pPic->getReconMark()) {
        return pPic;

      // TODO: Is this necessary?
      } else if (!pPic->getSlice(0)->isReferenced()) {
        pPic->setReconMark(false);
        pPic->getPicYuvRec()->setBorderExtension(false);
        return pPic;
      }
    }
  }

  // All entries are used, so make a new entry at the back of the list
  m_list.push_back(nullptr);
  return m_list.back();
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


/**
 * Sort the picture buffer by POC using insertion sort
 */
Void TTraPictureBuffer::sort() {
  for (Int i = 1; i < (Int)(m_list.size()); i++) {
    auto iterPicExtract = m_list.begin();
    for (Int j = 0; j < i; j++) {
      iterPicExtract++;
    }
    TComPic* pcPicExtract = *iterPicExtract;
    pcPicExtract->setCurrSliceIdx(0);

    auto iterPicInsert = m_list.begin();
    while (iterPicInsert != iterPicExtract) {
      TComPic* pcPicInsert = *iterPicInsert;
      pcPicInsert->setCurrSliceIdx(0);
      if (pcPicInsert->getPOC() >= pcPicExtract->getPOC()) {
        break;
      }

      iterPicInsert++;
    }

    auto iterPicExtract_1 = iterPicExtract;
    iterPicExtract_1++;

    //  swap iterPicExtract and iterPicInsert, iterPicExtract = curr. / iterPicInsert = insertion position
    m_list.insert(iterPicInsert, iterPicExtract, iterPicExtract_1);
    m_list.erase(iterPicExtract);
  }
}


/**
 * Extract a TComPic by POC, removing it from the buffer without deconstruction
 */
TComPic* TTraPictureBuffer::extract(Int poc) {
  TComPic* pPic = nullptr;
  for (auto it = m_list.cbegin(); it != m_list.cend(); it++) {
    TComPic* itPic = *it;
    if (itPic != nullptr && itPic->getPOC() == poc) {
      m_list.erase(it);
      it--;
      if (pPic == nullptr) {
        pPic = itPic;
      }
    }
  }
  return pPic;
}


/**
 * Remove a TComPic by POC, removing it from the buffer and destroying it
 */
Void TTraPictureBuffer::remove(Int poc) {
  for (auto it = m_list.cbegin(); it != m_list.cend(); it++) {
    TComPic* pic = *it;
    if (pic != nullptr && pic->getPOC() == poc) {
      pic->destroy();
      delete pic;
      m_list.erase(it);
      it--;
    }
  }
}
