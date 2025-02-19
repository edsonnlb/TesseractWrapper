/**********************************************************************
 * File:        polyblob.cpp  (Formerly blob.c)
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

#include "mfcpch.h"
#include          "params.h"
#include          "ocrrow.h"
#include          "polyblob.h"
//#include                                                      "lapoly.h"
#include          "polyaprx.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define EXTERN

ELISTIZE (PBLOB)
/**********************************************************************
 * position_outline
 *
 * Position the outline in the given list at the relevant place
 * according to its nesting.
 **********************************************************************/
static void position_outline(                        //put in place
                             OUTLINE *outline,       //thing to place
                             OUTLINE_LIST *destlist  //desstination list
                            ) {
  OUTLINE *dest_outline;         //outline from dest list
  OUTLINE_IT it = destlist;      //iterator
                                 //iterator on children
  OUTLINE_IT child_it = outline->child ();

  if (!it.empty ()) {
    do {
      dest_outline = it.data (); //get destination
                                 //encloses dest
      if (*dest_outline < *outline) {
                                 //take off list
        dest_outline = it.extract ();
                                 //put this in place
        it.add_after_then_move (outline);
                                 //make it a child
        child_it.add_to_end (dest_outline);
        while (!it.at_last ()) {
          it.forward ();         //do rest of list
                                 //check for other children
          dest_outline = it.data ();
          if (*dest_outline < *outline) {
                                 //take off list
            dest_outline = it.extract ();
            child_it.add_to_end (dest_outline);
            //make it a child
            if (it.empty ())
              break;
          }
        }
        return;                  //finished
      }
                                 //enclosed by dest
      else if (*outline < *dest_outline) {
        position_outline (outline, dest_outline->child ());
        //place in child list
        return;                  //finished
      }
      it.forward ();
    }
    while (!it.at_first ());
  }
  it.add_to_end (outline);       //at outer level
}


/**********************************************************************
 * plot_outline_list
 *
 * Draw a list of outlines in the given colour and their children
 * in the child colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
static void plot_outline_list(                     //draw outlines
                              OUTLINE_LIST *list,  //outline to draw
                              ScrollView* window,       //window to draw in
                              ScrollView::Color colour,       //colour to use
                              ScrollView::Color child_colour  //colour of children
                             ) {
  OUTLINE *outline;              //current outline
  OUTLINE_IT it = list;          //iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
                                 //draw it
    outline->plot (window, colour);
    if (!outline->child ()->empty ())
      plot_outline_list (outline->child (), window,
        child_colour, child_colour);
  }
}
#endif


/**********************************************************************
 * PBLOB::PBLOB
 *
 * Constructor to build a PBLOB from a list of OUTLINEs.
 * The OUTLINEs are not copied so the source list is emptied.
 * The OUTLINEs are nested correctly in the blob.
 **********************************************************************/

PBLOB::PBLOB(                            //constructor
             OUTLINE_LIST *outline_list  //in random order
            ) {
  OUTLINE *outline;              //current outline
  OUTLINE_IT it = outline_list;  //iterator

  while (!it.empty ()) {         //grab the list
    outline = it.extract ();     //get off the list
                                 //put it in place
    position_outline(outline, &outlines);
    if (!it.empty ())
      it.forward ();
  }
}


/**********************************************************************
 * approximate_outline_list
 *
 * Convert a list of chain-coded outlines (srclist) to polygonal form.
 **********************************************************************/

static void approximate_outline_list(C_OUTLINE_LIST *srclist,
                                     OUTLINE_LIST *destlist) {
  C_OUTLINE *src_outline;         // outline from src list
  OUTLINE *dest_outline;          // result
  C_OUTLINE_IT src_it = srclist;  // source iterator
  OUTLINE_IT dest_it = destlist;  // iterator

  do {
    src_outline = src_it.data ();
    dest_outline = tesspoly_outline(src_outline);
    if (dest_outline != NULL) {
      dest_it.add_after_then_move(dest_outline);
      if (!src_outline->child()->empty())
        // do child list
        approximate_outline_list(src_outline->child(), dest_outline->child());
    }
    src_it.forward();
  }
  while (!src_it.at_first());
}


/**********************************************************************
 * PBLOB::PBLOB
 *
 * Constructor to build a PBLOB (polygonal blob) from a C_BLOB
 * (chain-coded blob) by polygonal approximation.
 **********************************************************************/

PBLOB::PBLOB(C_BLOB *cblob) {
  if (!cblob->out_list ()->empty ()) {
    approximate_outline_list (cblob->out_list (), &outlines);
  }
}


/**********************************************************************
 * PBLOB::bounding_box
 *
 * Return the bounding box of the blob.
 **********************************************************************/

TBOX PBLOB::bounding_box() {  //bounding box
  OUTLINE *outline;              //current outline
  OUTLINE_IT it = &outlines;     //outlines of blob
  TBOX box;                       //bounding box

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    box += outline->bounding_box ();
  }
  return box;
}


/**********************************************************************
 * PBLOB::area
 *
 * Return the area of the blob.
 **********************************************************************/

float PBLOB::area() {  //area
  OUTLINE *outline;              //current outline
  OUTLINE_IT it = &outlines;     //outlines of blob
  float total;                   //total area

  total = 0.0f;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->area ();
  }
  return total;
}


/**********************************************************************
 * PBLOB::move
 *
 * Move PBLOB by vector
 **********************************************************************/

void PBLOB::move(                  // reposition blob
                 const FCOORD vec  // by vector
                ) {
  OUTLINE_IT it(&outlines);  // iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->move (vec);      // move each outline
}


/**********************************************************************
 * PBLOB::scale
 *
 * Scale PBLOB by float multiplier
 **********************************************************************/

void PBLOB::scale(               // scale blob
                  const float f  // by multiplier
                 ) {
  OUTLINE_IT it(&outlines);  // iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->scale (f);       // scale each outline
}


/**********************************************************************
 * PBLOB::scale
 *
 * Scale PBLOB by float multiplier
 **********************************************************************/

void PBLOB::scale(                  // scale blob
                  const FCOORD vec  // by multiplier
                 ) {
  OUTLINE_IT it(&outlines);  // iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->scale (vec);     // scale each outline
}

/**********************************************************************
 * PBLOB::rotate
 *
 * Rotate PBLOB 90 deg anticlockwise about the origin.
 **********************************************************************/

void PBLOB::rotate() {  // Rotate 90 deg anti
  rotate(FCOORD(0.0f, 1.0f));
}

/**********************************************************************
 * PBLOB::rotate
 *
 * Rotate PBLOB by the given rotation about the origin.
 * The rotation is defined to be (cos a, sin a) where a is the anticlockwise
 * rotation angle (in units appropriate for cos, sin).
 * Alternatively think of multiplication by the complex number
 * rotation = z = (x + iy), with |z| = 1.
 **********************************************************************/
void PBLOB::rotate(const FCOORD& rotation) {  // Rotate by given rotation.
  OUTLINE_IT it(&outlines);

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->rotate(rotation);     // Rotate each outline.
  }
}

/**********************************************************************
 * PBLOB::plot
 *
 * Draw the PBLOB in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void PBLOB::plot(                     //draw it
                 ScrollView* window,       //window to draw in
                 ScrollView::Color blob_colour,  //main colour
                 ScrollView::Color child_colour  //for holes
                ) {
  plot_outline_list(&outlines, window, blob_colour, child_colour);
}
#endif
