#include "Oracle.h"

#include <algorithm>
#include <future>
#include <map>
#include <mutex>

#include "Utils.h"
#include "Log.h"

namespace {

inline int randBound(int upperBound) { return rand() % (upperBound + 1); }

}  // namespace

class BaseOracle : public Oracle {
 protected:
  std::random_device rd;
  std::default_random_engine re;
  structs::Table *table;

  std::discrete_distribution<int> distribution;

 public:
  BaseOracle(structs::Table *table) : rd(), re(rd()), table(table) {}
};

class UniformSubsetOracle : public BaseOracle {
 protected:
  boost::dynamic_bitset<unsigned long> generateRandomSubsetBS(
      boost::dynamic_bitset<unsigned long> &set) {
    int numElems = set.size(), processedElems = 0;
    boost::dynamic_bitset<unsigned long> ansSet(numElems);

    while (processedElems < numElems) {
      int bset = rand();

      for (int i = 0; i < 30; i++) {
        if ((bset & (1 << i)) && (set[processedElems])) {
          ansSet[processedElems] = true;
        }

        processedElems++;

        if (processedElems >= numElems) break;
      }
    }

    return ansSet;
  }

 public:
  UniformSubsetOracle(structs::Table *table) : BaseOracle(table) {}
};

class PowerBasedSubsetOracle : public BaseOracle {
 protected:
  size_t getRandomSubsetSize(const size_t size) {
    std::vector<long double> w(size + 1);
    w[1] = size;
    for (size_t i = 2; i <= size; i++) {
      w[i] = w[i - 1] * (size - i + 1) / (i - 1);
    }

    return std::discrete_distribution<int>(w.begin(), w.end())(re);
  }

  boost::dynamic_bitset<unsigned long> generateRandomSubsetBS(
      std::vector<int> &set) {
    size_t sampleSize = getRandomSubsetSize(set.size());
    std::vector<int> result(sampleSize);
    std::sample(set.begin(), set.end(), result.begin(), sampleSize, re);
    return utils::attrVectorToAttrBS(result, table->objInpBS.front().size());
  }

 public:
  PowerBasedSubsetOracle(structs::Table *table) : BaseOracle(table) {}
};

class UniformOracle : public UniformSubsetOracle {
 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    boost::dynamic_bitset<unsigned long> ans(table->attrInp.size());
    ans.set();
    ans[0] = false;
    return generateRandomSubsetBS(ans);
  }

  UniformOracle(structs::Table *table) : UniformSubsetOracle(table) {}
};

class FrequentAttributeOracle : public UniformSubsetOracle {
 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS(table->objInpBS[distribution(re)]);
  }

  FrequentAttributeOracle(structs::Table *table) : UniformSubsetOracle(table) {
    std::vector<long double> attrSetWeight(table->objInp.size());

    for (int i = 0; i < table->objInp.size(); i++) {
      attrSetWeight[i] = (long double)pow((long double)2,
                                          (long double)table->objInp[i].size());
    }

    distribution = std::discrete_distribution<int>(attrSetWeight.begin(),
                                                   attrSetWeight.end());
  }
};

class AreaBasedOracle : public PowerBasedSubsetOracle {
 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS(table->objInp[distribution(re)]);
  }

  AreaBasedOracle(structs::Table *table) : PowerBasedSubsetOracle(table) {
    std::vector<long double> attrSetWeight(table->objInp.size());

    for (int i = 0; i < table->objInp.size(); i++) {
      attrSetWeight[i] =
          (long double)pow((long double)2,
                           (long double)table->objInp[i].size() - 1) *
          table->objInp[i].size();
    }

    distribution = std::discrete_distribution<int>(attrSetWeight.begin(),
                                                   attrSetWeight.end());
  }
};

class SquaredFrequencyOracle : public UniformSubsetOracle {
 private:
  std::vector<boost::dynamic_bitset<unsigned long>> objIntersectionBS;
  typedef std::map<boost::dynamic_bitset<unsigned long>, long double>
      mapIntersectionType;

  mapIntersectionType loadIntersectionShard(const size_t start,
                                            const size_t increment) {
    mapIntersectionType objIntersectionWeight;
    for (int i = start; i < table->objInpBS.size(); i += increment) {
      for (int j = i; j < table->objInpBS.size(); j++) {
        auto intersectionBS = table->objInpBS[i] & table->objInpBS[j];
        auto weight = (long double)pow((long double)2,
                                       (long double)intersectionBS.count());
        if (i != j) {
          weight *= 2;
        }
        if (auto it = objIntersectionWeight.find(intersectionBS);
            it != objIntersectionWeight.end()) {
          it->second += weight;
        } else {
          objIntersectionWeight[intersectionBS] = weight;
        }
      }
    }
    return objIntersectionWeight;
  }

 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS(objIntersectionBS[distribution(re)]);
  }

  SquaredFrequencyOracle(structs::Table *table, size_t threadCount)
      : UniformSubsetOracle(table) {
    assert(threadCount != 0);
    mapIntersectionType objIntersectionWeight;
    std::vector<std::future<mapIntersectionType>> objIntersectionWeightFutures;
    for (size_t threadNumber = 1; threadNumber < threadCount; ++threadNumber) {
      objIntersectionWeightFutures.push_back(
          std::async(&SquaredFrequencyOracle::loadIntersectionShard, this, 0,
                     threadCount));
    }
    loadIntersectionShard(0, threadCount);
    for (auto &resultFuture : objIntersectionWeightFutures) {
      auto result = resultFuture.get();
      for (const auto &one_result : result) {
        if (auto it = objIntersectionWeight.find(one_result.first);
            it != objIntersectionWeight.end()) {
          it->second += one_result.second;
        } else {
          objIntersectionBS.push_back(one_result.first);
          objIntersectionWeight[one_result.first] = one_result.second;
        }
      }
    }

    std::vector<long double> intersectionWeights;
    for (const auto &intersectionBS : objIntersectionBS) {
      intersectionWeights.push_back(objIntersectionWeight[intersectionBS]);
    }

    distribution = std::discrete_distribution<int>(intersectionWeights.begin(),
                                                   intersectionWeights.end());
  }
};

std::shared_ptr<Oracle> createOracle(const std::string &type,
                                     structs::Table *table,
                                     const size_t thread_count) {
  if (type == std::string("frequent")) {
    return std::make_shared<FrequentAttributeOracle>(table);
  } else if (type == std::string("area-based")) {
    return std::make_shared<AreaBasedOracle>(table);
  } else if (type == std::string("squared-frequency")) {
    return std::make_shared<SquaredFrequencyOracle>(table, thread_count);
  } else {
    return std::make_shared<UniformOracle>(table);
  }
}
