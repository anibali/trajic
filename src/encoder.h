#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "obstream.h"

class Encoder
{
public:
  Encoder(obstream *obs, uint64_t *nums, int len)
    : obs(obs), nums(nums), len(len) {}
  virtual void write_next() = 0;

protected:
  obstream *obs;
  uint64_t *nums;
  int len;
};

#endif

