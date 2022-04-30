#include "Oracle.h"

#include <map>

#include "BasisCalculator.h"

namespace {

inline int randBound(int upperBound) { return rand() % (upperBound + 1); }

}  // namespace

Oracle::Oracle(BasisCalculator *calc)
    : rd(),
      re(rd()),
      objInpBS(&calc->table.objInpBS),
      objInp(&calc->table.objInp),
      attrInp(&calc->table.attrInp) {}

class BaseOracle : public Oracle {
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
  BaseOracle(BasisCalculator *calc) : Oracle(calc) {}
};

class UniformOracle : public BaseOracle {
 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    boost::dynamic_bitset<unsigned long> ans(attrInp->size());
    ans.set();
    ans[0] = false;
    return generateRandomSubsetBS(ans);
  }

  UniformOracle(BasisCalculator *calc) : BaseOracle(calc) {}
};

class FrequentAttributeOracle : public BaseOracle {
 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS((*objInpBS)[distribution(re)]);
  }

  FrequentAttributeOracle(BasisCalculator *calc) : BaseOracle(calc) {
    std::vector<long double> attrSetWeight(objInp->size());

    for (int i = 0; i < objInp->size(); i++) {
      attrSetWeight[i] =
          (long double)pow((long double)2, (long double)(*objInp)[i].size());
    }

    distribution = std::discrete_distribution<int>(attrSetWeight.begin(),
                                                   attrSetWeight.end());
  }
};

class AreaBasedOracle : public Oracle {
 private:
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

    boost::dynamic_bitset<unsigned long> resultBitset(set.size());
    for (const auto &setI : resultMap) {
      resultBitset.set(setI);
    }

    return resultBitset;
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
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS((*objInpBS)[distribution(re)]);
  }

  AreaBasedOracle(BasisCalculator *calc) : Oracle(calc) {
    std::vector<long double> attrSetWeight(objInp->size());

    for (int i = 0; i < objInp->size(); i++) {
      attrSetWeight[i] =
          (long double)pow((long double)2,
                           (long double)(*objInp)[i].size() - 1) *
          (*objInp)[i].size();
    }

    distribution = std::discrete_distribution<int>(attrSetWeight.begin(),
                                                   attrSetWeight.end());
  }
};

class SquaredFrequencyOracle : public BaseOracle {
 private:
  std::vector<boost::dynamic_bitset<unsigned long>> objIntersectionBS;

 public:
  boost::dynamic_bitset<unsigned long> generate() override {
    return generateRandomSubsetBS(objIntersectionBS[distribution(re)]);
  }

  SquaredFrequencyOracle(BasisCalculator *calc) : BaseOracle(calc) {
    std::map<boost::dynamic_bitset<unsigned long>, long double>
        objIntersectionWeight;
    for (int i = 0; i < objInpBS->size(); i++) {
      for (int j = i + 1; j < objInpBS->size(); j++) {
        auto intersectionBS = (*objInpBS)[i] & (*objInpBS)[j];
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
                                     BasisCalculator *calc) {
  if (type == std::string("frequent")) {
    return std::make_shared<FrequentAttributeOracle>(calc);
  } else if (type == std::string("area-based")) {
    return std::make_shared<AreaBasedOracle>(calc);
  } else if (type == std::string("squared-frequency")) {
    return std::make_shared<SquaredFrequencyOracle>(calc);
  } else {
    return std::make_shared<UniformOracle>(calc);
  }
}
