#ifndef _DYNAMIC_ENCODER_H_
#define _DYNAMIC_ENCODER_H_

#include "encoder.h"

#include "len_freq_div.h"
#include "huffman.h"

#include <cmath>
using namespace std;

class DynamicEncoder : public Encoder
{
public:
  DynamicEncoder(obstream *obs, uint64_t *nums, int len);

  virtual void write_next() override;
  ~DynamicEncoder()
  {
    delete codebook;
  }

private:
  Huffman::Codebook<int> *codebook;
  int pos = 0;
};

#endif

