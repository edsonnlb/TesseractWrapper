///////////////////////////////////////////////////////////////////////
// File:        publictypes.h
// Description: Types used in both the API and internally
// Author:      Ray Smith
// Created:     Wed Mar 03 09:22:53 PST 2010
//
// (C) Copyright 2010, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCSTRUCT_PUBLICTYPES_H__
#define TESSERACT_CCSTRUCT_PUBLICTYPES_H__

// This file contains types that are used both by the API and internally
// to Tesseract. In order to decouple the API from Tesseract and prevent cyclic
// dependencies, THIS FILE SHOULD NOT DEPEND ON ANY OTHER PART OF TESSERACT.
// Restated: It is OK for low-level Tesseract files to include publictypes.h,
// but not for the low-level tesseract code to include top-level API code.
// This file should not use other Tesseract types, as that would drag
// their includes into the API-level.
// API-level code should include apitypes.h in preference to this file.

// Number of printers' points in an inch. The unit of the pointsize return.
const int kPointsPerInch = 72;

// Possible types for a POLY_BLOCK or ColPartition.
// Must be kept in sync with kPBColors in polyblk.cpp and PTIs*Type functions
// below, as well as kPolyBlockNames in publictypes.cpp.
// Used extensively by ColPartition, and POLY_BLOCK.
enum PolyBlockType {
  PT_UNKNOWN,        // Type is not yet known. Keep as the first element.
  PT_FLOWING_TEXT,   // Text that lives inside a column.
  PT_HEADING_TEXT,   // Text that spans more than one column.
  PT_PULLOUT_TEXT,   // Text that is in a cross-column pull-out region.
  PT_TABLE,          // Partition belonging to a table region.
  PT_VERTICAL_TEXT,  // Text-line runs vertically.
  PT_CAPTION_TEXT,   // Text that belongs to an image.
  PT_FLOWING_IMAGE,  // Image that lives inside a column.
  PT_HEADING_IMAGE,  // Image that spans more than one column.
  PT_PULLOUT_IMAGE,  // Image that is in a cross-column pull-out region.
  PT_HORZ_LINE,      // Horizontal Line.
  PT_VERT_LINE,      // Vertical Line.
  PT_NOISE,          // Lies outside of any column.
  PT_COUNT
};


// Returns true if PolyBlockType is of horizontal line type
inline bool PTIsLineType(PolyBlockType type) {
  return type == PT_HORZ_LINE || type == PT_VERT_LINE;
}
// Returns true if PolyBlockType is of image type
inline bool PTIsImageType(PolyBlockType type) {
  return type == PT_FLOWING_IMAGE || type == PT_HEADING_IMAGE ||
         type == PT_PULLOUT_IMAGE;
}
// Returns true if PolyBlockType is of text type
inline bool PTIsTextType(PolyBlockType type) {
  return type == PT_FLOWING_TEXT || type == PT_HEADING_TEXT ||
         type == PT_PULLOUT_TEXT || type == PT_TABLE ||
         type == PT_VERTICAL_TEXT || type == PT_CAPTION_TEXT;
}

// String name for each block type. Keep in sync with PolyBlockType.
extern const char* kPolyBlockNames[];

namespace tesseract {

// Possible modes for page layout analysis. These *must* be kept in order
// of decreasing amount of layout analysis to be done, except for OSD_ONLY,
// so that the inequality test macros below work.
enum PageSegMode {
  PSM_OSD_ONLY,       ///< Orientation and script detection only.
  PSM_AUTO_OSD,       ///< Automatic page segmentation with orientation and
                      ///< script detection. (OSD)
  PSM_AUTO_ONLY,      ///< Automatic page segmentation, but no OSD, or OCR.
  PSM_AUTO,           ///< Fully automatic page segmentation, but no OSD.
  PSM_SINGLE_COLUMN,  ///< Assume a single column of text of variable sizes.
  PSM_SINGLE_BLOCK_VERT_TEXT,  ///< Assume a single uniform block of vertically
                               ///< aligned text.
  PSM_SINGLE_BLOCK,   ///< Assume a single uniform block of text. (Default.)
  PSM_SINGLE_LINE,    ///< Treat the image as a single text line.
  PSM_SINGLE_WORD,    ///< Treat the image as a single word.
  PSM_CIRCLE_WORD,    ///< Treat the image as a single word in a circle.
  PSM_SINGLE_CHAR,    ///< Treat the image as a single character.

  PSM_COUNT           ///< Number of enum entries.
};

// Macros that act on a PageSegMode to determine whether components of
// layout analysis are enabled.
// *Depend critically on the order of elements of PageSegMode.*
#define PSM_OSD_ENABLED(pageseg_mode) ((pageseg_mode) <= PSM_AUTO_OSD)
#define PSM_COL_FIND_ENABLED(pageseg_mode) \
  ((pageseg_mode) >= PSM_AUTO_OSD && (pageseg_mode) <= PSM_AUTO)
#define PSM_BLOCK_FIND_ENABLED(pageseg_mode) \
  ((pageseg_mode) >= PSM_AUTO_OSD && (pageseg_mode) <= PSM_SINGLE_COLUMN)
#define PSM_LINE_FIND_ENABLED(pageseg_mode) \
  ((pageseg_mode) >= PSM_AUTO_OSD && (pageseg_mode) <= PSM_SINGLE_BLOCK)
#define PSM_WORD_FIND_ENABLED(pageseg_mode) \
  ((pageseg_mode) >= PSM_AUTO_OSD && (pageseg_mode) <= PSM_SINGLE_LINE)

// enum of the elements of the page hierarchy, used in ResultIterator
// to provide functions that operate on each level without having to
// have 5x as many functions.
// NOTE: At present RIL_PARA and RIL_BLOCK are equivalent as there is
// no paragraph internally yet.
// TODO(rays) Add paragraph detection.
enum PageIteratorLevel {
  RIL_BLOCK,     // Block of text/image/separator line.
  RIL_PARA,      // Paragraph within a block.
  RIL_TEXTLINE,  // Line within a paragraph.
  RIL_WORD,      // Word within a textline.
  RIL_SYMBOL     // Symbol/character within a word.
};

// When Tesseract/Cube is initialized we can choose to instantiate/load/run
// only the Tesseract part, only the Cube part or both along with the combiner.
// The preference of which engine to use is stored in tessedit_ocr_engine_mode.
//
// ATTENTION: When modifying this enum, please make sure to make the
// appropriate changes to all the enums mirroring it (e.g. OCREngine in
// cityblock/workflow/detection/detection_storage.proto). Such enums will
// mention the connection to OcrEngineMode in the comments.
enum OcrEngineMode {
  OEM_TESSERACT_ONLY,           // Run Tesseract only - fastest
  OEM_CUBE_ONLY,                // Run Cube only - better accuracy, but slower
  OEM_TESSERACT_CUBE_COMBINED,  // Run both and combine results - best accuracy
  OEM_DEFAULT                   // Specify this mode when calling init_*(),
                                // to indicate that any of the above modes
                                // should be automatically inferred from the
                                // variables in the language-specific config,
                                // command-line configs, or if not specified
                                // in any of the above should be set to the
                                // default OEM_TESSERACT_ONLY.
};

}  // namespace tesseract.

#endif  // TESSERACT_CCSTRUCT_PUBLICTYPES_H__
