/**********************************************************************
 * File:        polyblob.h  (Formerly blob.h)
 * Description: Code for PBLOB class.
 * Author:		Ray Smith
 * Created:		Wed Oct 23 15:17:41 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef           POLYBLOB_H
#define           POLYBLOB_H

#include          "poutline.h"
#include          "rect.h"
#include          "normalis.h"
#include          "stepblob.h"

class PBLOB : public ELIST_LINK {
  public:
    PBLOB() {}

    // Create from a list of polygonal outlines.
    PBLOB(OUTLINE_LIST *outline_list);

    // Create from a chain-coded form.
    PBLOB(C_BLOB *cblob);

    OUTLINE_LIST *out_list() {  //get outline list
      return &outlines;
    }

    TBOX bounding_box();  //compute bounding box
    float area();  //get area of blob

    void plot(                       //draw one
              ScrollView* window,         //window to draw in
              ScrollView::Color blob_colour,    //for outer bits
              ScrollView::Color child_colour);  //for holes

    void move(                    // reposition blob
              const FCOORD vec);  // by FLOAT vector

    void scale(                 // scale blob
               const float f);  // by multiplier
    void scale(                    // scale blob
               const FCOORD vec);  // by FLOAT vector
    void rotate();  // Rotate 90 deg anti
    void rotate(const FCOORD& rotation);  // Rotate by given rotation.

    PBLOB& operator=(const PBLOB & source) {
      if (!outlines.empty ())
        outlines.clear ();

      outlines.deep_copy(&source.outlines, &OUTLINE::deep_copy);
      return *this;
    }

    static PBLOB* deep_copy(const PBLOB* src) {
      PBLOB* blob = new PBLOB;
      *blob = *src;
      return blob;
    }

  private:
    OUTLINE_LIST outlines;       //master elements
};

ELISTIZEH (PBLOB)
#endif
