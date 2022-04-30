#ifndef __BASIS_CALCUCLATOR_H__
#define __BASIS_CALCUCLATOR_H__

#include <boost/dynamic_bitset.hpp>
#include <mutex>
#include <random>

#include "Structs.h"
#include "ThreadPool.h"
#include "Oracle.h"

#define TIMEPRINT(X) (((double)X) / ((double)1000000))

class BasisCalculator {
 private:
  friend class Oracle;
  structs::TimeStatistic timeStatistic;
  structs::Statistic statistic;
  structs::Table table;
  structs::PrintFormat printFormat = structs::kReadble;

  std::mutex mtx;

  boost::dynamic_bitset<unsigned long> counterExampleBS;
  std::vector<boost::dynamic_bitset<unsigned long>> potentialCounterExamplesBS;

  structs::implicationBS updatedImplication;
  std::vector<structs::implicationBS> basisBS;
  std::vector<structs::implication> basis;

  std::shared_ptr<Oracle> oracle;
  char **argv;

  void initFrequencyOrderedAttributes();
  void initializeObjInpBS();
  void readFormalContext1(const std::string &fileName);
  void readFormalContext2(const std::string &fileName);
  void initFromArgs(int argc, char **argv);

  void getLoopCount();
  void setNumThreads();
  boost::dynamic_bitset<unsigned long> contextClosureBS(
      boost::dynamic_bitset<unsigned long> &aset);
  boost::dynamic_bitset<unsigned long> closureBS(
      std::vector<structs::implicationBS> &basis, boost::dynamic_bitset<unsigned long> X);
  void tryPotentialCounterExamples(std::vector<structs::implicationBS> &basis);
  bool isSetEqualToImpCLosure(std::vector<structs::implicationBS> &basis,
                              boost::dynamic_bitset<unsigned long> &X);
  std::vector<structs::implication> BSBasisToVectorBasis(std::vector<structs::implicationBS> ansBS);
  bool isLectGreater(boost::dynamic_bitset<unsigned long> &closedSet,
                     int lectInd);
  boost::dynamic_bitset<unsigned long> nextContextClosure(
      boost::dynamic_bitset<unsigned long> A,
      boost::dynamic_bitset<unsigned long> finalClosedSet);
  boost::dynamic_bitset<unsigned long> nextImplicationClosure(
      boost::dynamic_bitset<unsigned long> A,
      boost::dynamic_bitset<unsigned long> finalClosedSet);
  void getCounterExample(std::vector<structs::implicationBS> &basis, int s);
  void tryToUpdateImplicationBasis(std::vector<structs::implicationBS> &basis);
  std::vector<std::string> getPrintResults(double totalExecTime = -1);

  // Static views for binding thread
  static void getCounterExampleView(BasisCalculator *calc,
                                    std::vector<structs::implicationBS> &basis, int s);
  static void tryToUpdateImplicationBasisView(BasisCalculator *calc,
                                              std::vector<structs::implicationBS> &basis);

 public:
  BasisCalculator(int argc, char **argv);
  void fillPotentialCounterExamples();
  std::vector<structs::implication> generateImplicationBasis();
  int allContextClosures();
  int allImplicationClosures();
  void printResults(double totalExecTime);
  int getMaxTheadCount();
};

#endif  // __BASIS_CALCUCLATOR_H__
