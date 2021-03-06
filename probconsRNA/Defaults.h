/////////////////////////////////////////////////////////////////
// Defaults.h
//
// Default constants for use in PROBCONS.  The emission
// probabilities were computed using the program used to build
// the BLOSUM62 matrix from the BLOCKS 5.0 dataset.  Transition
// parameters were obtained via unsupervised EM training on the
// BALIBASE 2.0 benchmark alignment database.
/////////////////////////////////////////////////////////////////

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <string>

using namespace std;

float initDistrib1Default[] = { 0.9588437676f, 0.0205782652f, 0.0205782652f };
float gapOpen1Default[] = { 0.0190259293f, 0.0190259293f };
float gapExtend1Default[] = { 0.3269913495f, 0.3269913495f };

float initDistrib2Default[] = { 0.9615409374f, 0.0000004538f, 0.0000004538f, 0.0192291681f, 0.0192291681f };
float gapOpen2Default[] = { 0.0082473317f, 0.0082473317f, 0.0107844425f, 0.0107844425f };
float gapExtend2Default[] = { 0.3210460842f, 0.3210460842f, 0.3298229277f, 0.3298229277f };

string alphabetDefault = "ACGUTN";
float emitSingleDefault[6] = {
  0.2270790040f, 0.2422080040f, 0.2839320004f, 0.2464679927f, 0.2464679927f, 0.0003124650f 
};

float emitPairsDefault[6][6] = {
  { 0.1487240046f, 0.0184142999f, 0.0361397006f, 0.0238473993f, 0.0238473993f, 0.0000375308f },
  { 0.0184142999f, 0.1583919972f, 0.0275536999f, 0.0389291011f, 0.0389291011f, 0.0000815823f },
  { 0.0361397006f, 0.0275536999f, 0.1979320049f, 0.0244289003f, 0.0244289003f, 0.0000824765f },
  { 0.0238473993f, 0.0389291011f, 0.0244289003f, 0.1557479948f, 0.1557479948f, 0.0000743985f },
  { 0.0238473993f, 0.0389291011f, 0.0244289003f, 0.1557479948f, 0.1557479948f, 0.0000743985f },
  { 0.0000375308f, 0.0000815823f, 0.0000824765f, 0.0000743985f, 0.0000743985f, 0.0000263252f }
};

#endif
