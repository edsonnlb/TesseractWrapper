/**********************************************************************
 * File:        params.cpp
 * Description: Initialization and setting of Tesseract parameters.
 * Author:      Ray Smith
 * Created:     Fri Feb 22 16:22:34 GMT 1991
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

#include          "mfcpch.h"     //precompiled headers

#include          <stdio.h>
#include          <string.h>
#include          <stdlib.h>

#include          "genericvector.h"
#include          "scanutils.h"
#include          "tprintf.h"
#include          "params.h"

#define PLUS          '+'        //flag states
#define MINUS         '-'
#define EQUAL         '='

tesseract::ParamsVectors *GlobalParams() {
  static tesseract::ParamsVectors *global_params =
    new tesseract::ParamsVectors();
  return global_params;
}

namespace tesseract {

bool ParamUtils::ReadParamsFile(const char *file, bool init_only,
                                ParamsVectors *member_params) {
  char flag;                     // file flag
  inT16 nameoffset;              // offset for real name
  FILE *fp;                      // file pointer
                                 // iterators

  if (*file == PLUS) {
    flag = PLUS;                 // file has flag
    nameoffset = 1;
  } else if (*file == MINUS) {
    flag = MINUS;
    nameoffset = 1;
  } else {
    flag = EQUAL;
    nameoffset = 0;
  }

  fp = fopen(file + nameoffset, "r");
  if (fp == NULL) {
    tprintf("read_params_file: Can't open %s\n", file + nameoffset);
    return true;
  }
  return ReadParamsFromFp(fp, -1, init_only, member_params);
  fclose(fp);
}

bool ParamUtils::ReadParamsFromFp(FILE *fp, inT64 end_offset, bool init_only,
                                  ParamsVectors *member_params) {
  char line[MAX_PATH];           // input line
  bool anyerr = false;           // true if any error
  bool foundit;                  // found parameter
  inT16 length;                  // length of line
  char *valptr;                  // value field

  while ((end_offset < 0 || ftell(fp) < end_offset) &&
         fgets(line, MAX_PATH, fp)) {
    if (line[0] != '\n' && line[0] != '#') {
      length = strlen (line);
      if (line[length - 1] == '\n')
        line[length - 1] = '\0';  // cut newline
      for (valptr = line; *valptr && *valptr != ' ' && *valptr != '\t';
        valptr++);
      if (*valptr) {             // found blank
        *valptr = '\0';          // make name a string
        do
          valptr++;              // find end of blanks
        while (*valptr == ' ' || *valptr == '\t');
      }
      foundit = SetParam(line, valptr, init_only, member_params);

      if (!foundit) {
        anyerr = true;         // had an error
        tprintf("read_params_file: parameter not found: %s\n", line);
        exit(1);
      }
    }
  }
  return anyerr;
}

bool ParamUtils::SetParam(const char *name, const char* value,
                          bool init_only, ParamsVectors *member_params) {
  // Look for the parameter among string parameters.
  StringParam *sp = FindParam<StringParam>(name, GlobalParams()->string_params,
                                           member_params->string_params);
  if (sp != NULL && (!init_only || sp->is_init())) sp->set_value(value);
  if (*value == '\0') return (sp != NULL);

  // Look for the parameter among int parameters.
  int intval;
  IntParam *ip = FindParam<IntParam>(name, GlobalParams()->int_params,
                                     member_params->int_params);
  if (ip && (!init_only || ip->is_init()) &&
      sscanf(value, INT32FORMAT, &intval) == 1) ip->set_value(intval);

  // Look for the parameter among bool parameters.
  BoolParam *bp = FindParam<BoolParam>(name, GlobalParams()->bool_params,
                                       member_params->bool_params);
  if (bp != NULL && (!init_only || bp->is_init())) {
    if (*value == 'T' || *value == 't' ||
        *value == 'Y' || *value == 'y' || *value == '1') {
      bp->set_value(true);
    } else if (*value == 'F' || *value == 'f' ||
                *value == 'N' || *value == 'n' || *value == '0') {
      bp->set_value(false);
    }
  }

  // Look for the parameter among double parameters.
  double doubleval;
  DoubleParam *dp = FindParam<DoubleParam>(name, GlobalParams()->double_params,
                                           member_params->double_params);
  if (dp != NULL && (!init_only || dp->is_init())) {
#ifdef EMBEDDED
      doubleval = strtofloat(value);
#else
      if (sscanf(value, "%lf", &doubleval) == 1)
#endif
      dp->set_value(doubleval);
  }
  return (sp || ip || bp || dp);
}

bool ParamUtils::GetParamAsString(const char *name,
                                  const ParamsVectors* member_params,
                                  STRING *value) {
  // Look for the parameter among string parameters.
  StringParam *sp = FindParam<StringParam>(name, GlobalParams()->string_params,
                                           member_params->string_params);
  if (sp) {
    *value = sp->string();
    return true;
  }
  // Look for the parameter among int parameters.
  IntParam *ip = FindParam<IntParam>(name, GlobalParams()->int_params,
                                     member_params->int_params);
  if (ip) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%d", inT32(*ip));
    *value = buf;
    return true;
  }
  // Look for the parameter among bool parameters.
  BoolParam *bp = FindParam<BoolParam>(name, GlobalParams()->bool_params,
                                       member_params->bool_params);
  if (bp != NULL) {
    *value = BOOL8(*bp) ? "1": "0";
    return true;
  }
  // Look for the parameter among double parameters.
  DoubleParam *dp = FindParam<DoubleParam>(name, GlobalParams()->double_params,
                                           member_params->double_params);
  if (dp != NULL) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%g", double(*dp));
    *value = buf;
    return true;
  }
  return false;
}

void ParamUtils::PrintParams(FILE *fp, const ParamsVectors *member_params) {
  int v, i;
  int num_iterations = (member_params == NULL) ? 1 : 2;
  for (v = 0; v < num_iterations; ++v) {
    const ParamsVectors *vec = (v == 0) ? GlobalParams() : member_params;
    for (i = 0; i < vec->int_params.size(); ++i) {
      fprintf(fp, "%s\t%d\n", vec->int_params[i]->name_str(),
              (inT32)(*vec->int_params[i]));
    }
    for (i = 0; i < vec->bool_params.size(); ++i) {
      fprintf(fp, "%s\t%d\n", vec->bool_params[i]->name_str(),
              (BOOL8)(*vec->bool_params[i]));
    }
    for (int i = 0; i < vec->string_params.size(); ++i) {
      fprintf(fp, "%s\t%s\n", vec->string_params[i]->name_str(),
              vec->string_params[i]->string());
    }
    for (int i = 0; i < vec->double_params.size(); ++i) {
      fprintf(fp, "%s\t%g\n", vec->double_params[i]->name_str(),
              (double)(*vec->double_params[i]));
    }
  }
}

}  // namespace tesseract
