#include "Oracle.h"

#include <map>

#include "Utils.h"

namespace {

inline int randBound(int upperBound) { return rand() % (upperBound + 1); }

}  // namespace

class BaseOracle : public Oracle {
 protected:
  std::random_device rd;
  std::default_random_engine re;
  structs::Table *table;

  std::discrete_distribution<int> distribution;
  virtual boost::dynamic_bitset<unsigned long> generateRandomSubsetBS(
      boost::dynamic_bitset<unsigned long> &set) = 0;

 public:
  BaseOracle(structs::Table *table) : rd(), re(rd()), table(table) {}
};

class UniformSubsetOracle : public BaseOracle {
 protected:
  boost::dynamic_bitset<unsigned long> generateRandomSubsetBS(
      boost::dynamic_bitset<unsigned long> &set) override {
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
  boost::dynamic_bitset<unsigned long> reservoirSampleBS(
      const boost::dynamic_bitset<unsigned long> &set,
      const size_t subsetSize) {
    int i = 0;
    std::vector<int> resultMap(subsetSize);
    for (size_t setI = 0; setI < set.size(); setI++) {
      if (set.test(setI)) {
        if (i < subsetSize) {
          resultMap[i] = setI;
        } else {
          int j = randBound(i);
          if (j < subsetSize) {
            resultMap[j] = setI;
          }
        }
        ++i;
      }
    }

    return utils::attrVectorToAttrBS(resultMap, set.size());
  }

  size_t getRandomSubsetSize(const size_t size) {
    std::vector<long double> w(size + 1);
    w[1] = size;
    for (size_t i = 2; i <= size; i++) {
      w[i] = w[i - 1] * (size - i + 1) / (i - 1);
    }

    return std::discrete_distribution<int>(w.begin(), w.end())(re);
  }

  boost::dynamic_bitset<unsigned long> generateRandomSubsetBS(
      boost::dynamic_bitset<unsigned long> &set) override {
    return reservoirSampleBS(set, getRandomSubsetSize(set.count()));
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
    return generateRandomSubsetBS(table->objInpBS[distribution(re)]);
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

 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS(objIntersectionBS[distribution(re)]);
  }

  SquaredFrequencyOracle(structs::Table *table) : UniformSubsetOracle(table) {
    std::map<boost::dynamic_bitset<unsigned long>, long double>
        objIntersectionWeight;
    // TODO: D1, D1
    for (int i = 0; i < table->objInpBS.size(); i++) {
      for (int j = i + 1; j < table->objInpBS.size(); j++) {
        auto intersectionBS = table->objInpBS[i] & table->objInpBS[j];
        auto weight = (long double)pow((long double)2,
                                       (long double)intersectionBS.count());
        if (auto it = objIntersectionWeight.find(intersectionBS);
            it != objIntersectionWeight.end()) {
          it->second += weight;
        } else {
          objIntersectionBS.push_back(intersectionBS);
          objIntersectionWeight[intersectionBS] = weight;
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
                                     structs::Table *table) {
  if (type == std::string("frequent")) {
    return std::make_shared<FrequentAttributeOracle>(table);
  } else if (type == std::string("area-based")) {
    return std::make_shared<AreaBasedOracle>(table);
  } else if (type == std::string("squared-frequency")) {
    return std::make_shared<SquaredFrequencyOracle>(table);
  } else {
    return std::make_shared<UniformOracle>(table);
  }
}
