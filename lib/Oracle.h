#ifndef __ORACLE_H__
#define __ORACLE_H__

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <iostream>

class BasisCalculator;

class Oracle {
 protected:
  std::random_device rd;
  std::default_random_engine re;

  std::vector<std::vector<int>> *objInp;
  std::vector<std::vector<int>> *attrInp;
  std::vector<boost::dynamic_bitset<unsigned long>> *objInpBS;

  std::discrete_distribution<int> distribution;
  virtual boost::dynamic_bitset<unsigned long> generateRandomSubsetBS(
      boost::dynamic_bitset<unsigned long> &set) = 0;

 public:
  virtual boost::dynamic_bitset<unsigned long> generate() = 0;

  Oracle(BasisCalculator *calc);
};

std::shared_ptr<Oracle> createOracle(const std::string &type,
                                     BasisCalculator *calc);

#endif  // __ORACLE_H__
