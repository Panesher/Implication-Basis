#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace structs {

struct Table {
  // For storing which attributes are associated with which objects
  std::vector<std::vector<int>> objInp;
  // For storing which objects are associated with which attributes
  std::vector<std::vector<int>> attrInp;
  std::vector<boost::dynamic_bitset<unsigned long>> objInpBS;
  // For terminating other threads in case one thread found a counter-example
  boost::dynamic_bitset<unsigned long> emptySetClosure;
  std::vector<int> frequencyOrderedAttributes;
};

struct TimeStatistic {
  double total = 0;
  double totalExec2 = 0;
  double totalClosure = 0;
  double intersection = 0;
  double thisIterMaxImplicationClosure = 0;
  double thisIterMaxContextClosure = 0;
  double updown = 0;
  double threadOverhead = 6;
  double prevIter = 0;
};

struct Statistic {
  bool epsilonStrong = false;
  bool implicationSupport = false;
  bool emptySetClosureComputed = false;
  bool basisUpdate = false;
  bool globalFlag;
  int numThreads = 1;
  int maxThreads;
  int indexOfUpdatedImplication;
  int implicationsSeen;
  int maxTries;
  int gCounter = 0;
  int totTries = 0;
  int prevThreads = 1;
  double epsilon;
  double del;
  long long totCounterExamples = 0;
  long long totUpDownComputes = 0;
  long long sumTotTries = 0;
  long long totClosureComputations = 0;
  long long emptySetClosureComputes = 0;
  long long aEqualToCCount = 0;
};

struct implication {
  std::vector<int> lhs;
  std::vector<int> rhs;
};

struct implicationBS {
  boost::dynamic_bitset<unsigned long> lhs;
  boost::dynamic_bitset<unsigned long> rhs;
};

enum PrintFormat {
  kReadble,
  kCSV,
};

}  // namespace structs

#endif  // __STRUCTS_H__
