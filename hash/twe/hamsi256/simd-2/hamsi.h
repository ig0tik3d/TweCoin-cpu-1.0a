#ifndef __HAMSI_H__
#define __HAMSI_H__

#include <stdint.h>

typedef unsigned char BitSequence;
typedef unsigned long long DataLength;
typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHBITLEN = 2 } HashReturn;

typedef struct {
  uint32_t cv[8] __attribute__ ((aligned (32)));
  uint8_t buffer[4];
  uint64_t databitlen;
  int hashbitlen;
} hashStateHM;

HashReturn InitHM
  (hashStateHM* state, int hashbitlen);

HashReturn UpdateHM
  (hashStateHM* state, const BitSequence* data, DataLength databitlen);

HashReturn FinalHM
  (hashStateHM* state, BitSequence* hashval);

HashReturn HashHM
  (int hashbitlen, const BitSequence* data, DataLength databitlen,
   BitSequence *hashval);

#endif
