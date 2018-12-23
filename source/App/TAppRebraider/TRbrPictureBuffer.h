#pragma once


#include <list>

#include "TLibCommon/TComPic.h"


class TRbrPictureBuffer {
protected:
  // Internal list of pictures
  std::list<TComPic*> m_list;


public:
  // Default constructor
  TRbrPictureBuffer();

  // Virtual destructor
  virtual ~TRbrPictureBuffer();

  // Get a reference to the internal list
        std::list<TComPic*>& list();

  // Get a const reference to the internal list
  const std::list<TComPic*>& list() const;

  // Find an existing TComPic by POC
  TComPic* find(Int poc);

  // Find an existing const TComPic by POC
  const TComPic* find(Int poc) const;

  // Get an unused entry from the picture buffer
  TComPic*& getUnusedEntry();

  // Empty the picture buffer
  Void clear();

  // Sort the picture buffer by POC
  //   Note: Uses insertion sort
  //   Note: May reset the current slice pointer of some pictures to 0
  Void sort();

  // Extract a TComPic by POC, removing all occurances from the buffer without
  //   destroying them and returning the first one
  TComPic* extract(Int poc);

  // Remove a TComPic by POC, removing all occurances from the buffer and
  //   destroying them
  Void remove(Int poc);
};
