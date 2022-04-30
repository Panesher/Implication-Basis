#ifndef __ORACLE_H__
#define __ORACLE_H__

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Structs.h"

class Oracle {
 public:
  virtual boost::dynamic_bitset<unsigned long> generate() = 0;
};

std::shared_ptr<Oracle> createOracle(const std::string &type,
                                     structs::Table *calc);

#endif  // __ORACLE_H__
