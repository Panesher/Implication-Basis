#include "BasisCalculator.h"

#include <fstream>
#include <map>
#include <random>
#include <sstream>

#include "Utils.h"

using namespace std;
using namespace structs;

void BasisCalculator::tryToUpdateImplicationBasisView(
    BasisCalculator *calc, vector<implicationBS> &basis) {
  calc->tryToUpdateImplicationBasis(basis);
}

void BasisCalculator::getCounterExampleView(BasisCalculator *calc,
                                            vector<implicationBS> &basis,
                                            int s) {
  calc->getCounterExample(basis, s);
}

void BasisCalculator::readFormalContext1(const string &fileName) {
  ifstream inFile(fileName);
  string line;
  while (getline(inFile, line)) {
    vector<int> cur;
    istringstream iss(line);
    int x;
    while (iss >> x) {
      if (x >= table.attrInp.size()) table.attrInp.resize(x + 1);
      table.attrInp[x].push_back(table.objInp.size());
      cur.push_back(x);
    }
    if (cur.size() != 0) table.objInp.push_back(cur);
  }

  inFile.close();
}

void BasisCalculator::readFormalContext2(const string &fileName) {
  ifstream inFile(fileName);
  int obj, attr;
  inFile >> obj >> attr;
  table.objInp.resize(obj);
  table.attrInp.resize(attr);
  for (int i = 0; i < obj; i++) {
    int x;
    for (int j = 0; j < attr; j++) {
      inFile >> x;
      if (x == 1) {
        table.objInp[i].push_back(j);
        table.attrInp[j].push_back(i);
      }
    }
  }
  inFile.close();
}

void BasisCalculator::initializeObjInpBS() {
  table.objInpBS.resize(table.objInp.size());

  for (int i = 0; i < table.objInp.size(); i++) {
    table.objInpBS[i] =
        utils::attrVectorToAttrBS(table.objInp[i], table.attrInp.size());
  }
}

void BasisCalculator::initFrequencyOrderedAttributes() {
  vector<int> freqAttr(table.attrInp.size(), 0);

  for (int i = 0; i < table.objInp.size(); i++) {
    for (int j = 0; j < table.objInp[i].size(); j++)
      freqAttr[table.objInp[i][j]]++;
  }

  vector<pair<int, int>> freqPairs;

  for (int i = 1; i < table.attrInp.size(); i++) {
    freqPairs.push_back({freqAttr[i], i});
  }

  sort(freqPairs.begin(), freqPairs.end());
  table.frequencyOrderedAttributes.push_back(0);

  for (int i = 0; i < freqPairs.size(); i++)
    table.frequencyOrderedAttributes.push_back(freqPairs[i].second);
}

void BasisCalculator::initFromArgs(int argc, char **argv) {
  if (argc == 2 && argv[1] == string("header")) {
    utils::printCSVHeader();
    exit(0);
  }

  if (argc < 8) {
    utils::printUsageAndExit();
  }

  readFormalContext1(argv[1]);
  initializeObjInpBS();
  initFrequencyOrderedAttributes();
  statistic.epsilon = atof(argv[2]);
  statistic.del = atof(argv[3]);
  if (string(argv[4]) == string("strong")) statistic.epsilonStrong = true;

  oracle = createOracle(string(argv[5]), &table);

  statistic.maxThreads = atoi(argv[6]);
  statistic.numThreads = 1;
  if (string(argv[7]) == string("support")) {
    statistic.implicationSupport = true;
  }

  if (argc > 8 && argv[8] == string("csv-with-header")) {
    printFormat = kCSV;
    utils::printCSVHeader();
  } else if (argc > 8 && argv[8] == string("csv")) {
    printFormat = kCSV;
  }
}

void BasisCalculator::getLoopCount() {
  double loopCount =
      log(statistic.del /
          ((double)(statistic.gCounter * (statistic.gCounter + 1))));
  loopCount = loopCount / log(1 - statistic.epsilon);
  statistic.maxTries = (int)ceil(loopCount);
}

BasisCalculator::BasisCalculator(int argc, char **argv) : argv(argv) {
  initFromArgs(argc, argv);
}

boost::dynamic_bitset<unsigned long> BasisCalculator::contextClosureBS(
    boost::dynamic_bitset<unsigned long> &aset) {
  statistic.totUpDownComputes++;
  boost::dynamic_bitset<unsigned long> aBS = aset, ansBS(table.attrInp.size());
  ansBS.set();
  ansBS[0] = false;

  int aid = -1;
  int osize = table.objInp.size() + 1;

  for (int i = 0; i < aset.size(); i++) {
    if (aset[i] && (table.attrInp[i].size() < osize)) {
      osize = table.attrInp[i].size();
      aid = i;
    }
  }

  if (aid != -1) {
    for (int i = 0; i < table.attrInp[aid].size(); i++) {
      int cObj = table.attrInp[aid][i];

      if (aBS.is_subset_of(table.objInpBS[cObj])) {
        ansBS &= table.objInpBS[cObj];
      }

      // if(ansBS.count() == aBS.count())
      // 	return ansBS;
    }
  }

  else {
    statistic.emptySetClosureComputes++;

    if (statistic.emptySetClosureComputed) return table.emptySetClosure;

    for (int i = 0; i < table.objInp.size(); i++) {
      int cObj = i;
      ansBS &= table.objInpBS[cObj];
    }

    table.emptySetClosure = ansBS;
    statistic.emptySetClosureComputed = true;
  }

  return ansBS;
}

boost::dynamic_bitset<unsigned long> BasisCalculator::closureBS(
    vector<implicationBS> &basis, boost::dynamic_bitset<unsigned long> X) {
  statistic.totClosureComputations++;
  if (basis.size() == 0) return X;
  vector<bool> cons;
  for (int i = 0; i <= basis.size(); i++) cons.push_back(false);
  bool changed = true;

  while (changed) {
    changed = false;

    for (int i = 0; i < basis.size(); i++) {
      if (cons[i] == true) continue;

      if (basis[i].lhs.is_subset_of(X)) {
        cons[i] = true;

        if (!basis[i].rhs.is_subset_of(X)) {
          X |= basis[i].rhs;
          changed = true;
          break;
        }
      }
    }
  }

  return X;
}

void BasisCalculator::tryPotentialCounterExamples(
    vector<implicationBS> &basis) {
  while (!potentialCounterExamplesBS.empty()) {
    boost::dynamic_bitset<unsigned long> X = potentialCounterExamplesBS.back();
    potentialCounterExamplesBS.pop_back();
    boost::dynamic_bitset<unsigned long> cX = contextClosureBS(X);
    if (X.count() == cX.count()) continue;
    boost::dynamic_bitset<unsigned long> cL = closureBS(basis, X);

    if (statistic.epsilonStrong) {
      if (cL.count() != cX.count()) {
        counterExampleBS = cL;
        statistic.globalFlag = false;
        return;
      }
    }

    else {
      if (cL.count() == X.count()) {
        counterExampleBS = cL;
        statistic.globalFlag = false;
        return;
      }
    }
  }
}

void BasisCalculator::setNumThreads() {
  double temp = (statistic.prevThreads * timeStatistic.prevIter) /
                timeStatistic.threadOverhead;
  temp -= (statistic.prevThreads * statistic.prevThreads);

  if (temp < 0) {
    statistic.numThreads = 1;
    return;
  }

  temp = sqrt(temp);
  statistic.numThreads = max((int)temp, 1);
  statistic.numThreads = min((int)statistic.numThreads, statistic.maxThreads);
}

void BasisCalculator::getCounterExample(vector<implicationBS> &basis, int s) {
  double threadContextClosureTime = 0, threadImplicationClosureTime = 0;
  std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
  int threadTries = 0;
  boost::dynamic_bitset<unsigned long> X;

  for (int i = s; i < statistic.maxTries && statistic.globalFlag;
       i +=
       statistic
           .numThreads) {  // Each thread handles an equal number of iterations.
    threadTries++;

    X = oracle->generate();

    auto start = chrono::high_resolution_clock::now();
    boost::dynamic_bitset<unsigned long> cX = contextClosureBS(X);
    auto end = chrono::high_resolution_clock::now();
    threadContextClosureTime +=
        (chrono::duration_cast<chrono::microseconds>(end - start)).count();

    if (X.count() == cX.count())
      continue;  // It is sufficient to compare sizes since closure does not
                 // remove elements.

    if (statistic.epsilonStrong) {
      start = chrono::high_resolution_clock::now();
      boost::dynamic_bitset<unsigned long> cL = closureBS(basis, X);
      end = chrono::high_resolution_clock::now();
      threadImplicationClosureTime +=
          (chrono::duration_cast<chrono::microseconds>(end - start)).count();

      if (cX.count() != cL.count()) {
        lck.lock();

        // statistic.globalFlag is false until a counterexample has been found
        //  if(statistic.globalFlag) // If this line is not commented then
        //  quality increases with threads
        {
          statistic.globalFlag = false;
          counterExampleBS = cL;
        }

        lck.unlock();
        break;
      }
    }

    else {
      if (isSetEqualToImpCLosure(basis, X)) {
        lck.lock();

        // statistic.globalFlag is false until a counterexample has been found
        //  if(statistic.globalFlag) // If this line is not commented then
        //  quality increases with threads
        {
          statistic.globalFlag = false;
          counterExampleBS = X;
        }

        lck.unlock();
        break;
      }
    }
  }

  lck.lock();

  statistic.totTries += threadTries;

  if (threadContextClosureTime > timeStatistic.thisIterMaxContextClosure)
    timeStatistic.thisIterMaxContextClosure = threadContextClosureTime;

  if (threadImplicationClosureTime >
      timeStatistic.thisIterMaxImplicationClosure)
    timeStatistic.thisIterMaxImplicationClosure = threadImplicationClosureTime;

  lck.unlock();
}

bool BasisCalculator::isSetEqualToImpCLosure(
    vector<implicationBS> &basis, boost::dynamic_bitset<unsigned long> &X) {
  for (int i = 0; i < basis.size(); i++) {
    if (basis[i].lhs.is_subset_of(X) && (!basis[i].rhs.is_subset_of(X)))
      return false;
  }

  return true;
}

void BasisCalculator::fillPotentialCounterExamples() {
  // Singleton
  for (int i = 1; i < table.attrInp.size(); i++) {
    vector<int> cVec = {i};
    potentialCounterExamplesBS.push_back(
        utils::attrVectorToAttrBS(cVec, table.attrInp.size()));
  }
}

boost::dynamic_bitset<unsigned long> BasisCalculator::nextContextClosure(
    boost::dynamic_bitset<unsigned long> A,
    boost::dynamic_bitset<unsigned long> finalClosedSet) {
  int nAttr = table.attrInp.size() - 1;

  for (int i = nAttr; i > 0; i--) {
    if (A[table.frequencyOrderedAttributes[i]])
      A[table.frequencyOrderedAttributes[i]] = false;
    else {
      boost::dynamic_bitset<unsigned long> B, temp = A;
      temp[table.frequencyOrderedAttributes[i]] = true;
      B = contextClosureBS(temp);

      bool flag = true;

      for (int j = 1; j < i; j++) {
        if (B[table.frequencyOrderedAttributes[j]] &
            (!A[table.frequencyOrderedAttributes[j]])) {
          flag = false;
          break;
        }
      }

      if (flag) return B;
    }
  }

  return finalClosedSet;
}

int BasisCalculator::allContextClosures() {
  int totalClosedSets = 1;
  boost::dynamic_bitset<unsigned long> currentClosedSet,
      finalClosedSet(table.attrInp.size()), emptySet(table.attrInp.size());
  currentClosedSet = contextClosureBS(emptySet);
  finalClosedSet.set();
  finalClosedSet[0] = false;
  int nattr = table.attrInp.size();
  int lectInd = max(1, ((3 * nattr) / 4)), lectLessClosures;
  bool lectDone = false;
  auto timeStart = chrono::high_resolution_clock::now();
  auto timePrev = chrono::high_resolution_clock::now();

  while (currentClosedSet != finalClosedSet) {
    currentClosedSet = nextContextClosure(currentClosedSet, finalClosedSet);
    totalClosedSets++;
    auto timeNow = chrono::high_resolution_clock::now();
    double duration =
        (chrono::duration_cast<chrono::microseconds>(timeNow - timePrev))
            .count();

    if (duration > 60000000) {
      timePrev = timeNow;
    }

    if ((!lectDone) && isLectGreater(currentClosedSet, lectInd)) {
      lectLessClosures = totalClosedSets;
      lectDone = true;
    }

    duration =
        (chrono::duration_cast<chrono::microseconds>(timeNow - timeStart))
            .count();

    if (lectDone && (duration > 6000000)) {
      return lectLessClosures;
    }
  }

  return lectLessClosures;
}

boost::dynamic_bitset<unsigned long> BasisCalculator::nextImplicationClosure(
    boost::dynamic_bitset<unsigned long> A,
    boost::dynamic_bitset<unsigned long> finalClosedSet) {
  int nAttr = table.attrInp.size() - 1;

  for (int i = nAttr; i > 0; i--) {
    if (A[table.frequencyOrderedAttributes[i]])
      A[table.frequencyOrderedAttributes[i]] = false;
    else {
      boost::dynamic_bitset<unsigned long> B, temp = A;
      temp[table.frequencyOrderedAttributes[i]] = true;
      B = closureBS(basisBS, temp);

      bool flag = true;

      for (int j = 1; j < i; j++) {
        if (B[table.frequencyOrderedAttributes[j]] &
            (!A[table.frequencyOrderedAttributes[j]])) {
          flag = false;
          break;
        }
      }

      if (flag) return B;
    }
  }

  return finalClosedSet;
}

int BasisCalculator::allImplicationClosures() {
  int totalClosedSets = 1;
  boost::dynamic_bitset<unsigned long> currentClosedSet,
      finalClosedSet(table.attrInp.size()), emptySet(table.attrInp.size());
  currentClosedSet = closureBS(basisBS, emptySet);
  finalClosedSet.set();
  finalClosedSet[0] = false;

  int nattr = table.attrInp.size();
  int lectInd = max(1, ((3 * nattr) / 4)), lectLessClosures;
  bool lectDone = false;
  auto timeStart = chrono::high_resolution_clock::now();
  auto timePrev = chrono::high_resolution_clock::now();

  while (currentClosedSet != finalClosedSet) {
    currentClosedSet = nextImplicationClosure(currentClosedSet, finalClosedSet);
    totalClosedSets++;

    auto timeNow = chrono::high_resolution_clock::now();
    double duration =
        (chrono::duration_cast<chrono::microseconds>(timeNow - timePrev))
            .count();

    if (duration > 60000000) {
      timePrev = timeNow;
    }

    if ((!lectDone) && isLectGreater(currentClosedSet, lectInd)) {
      lectLessClosures = totalClosedSets;
      lectDone = true;
    }

    duration =
        (chrono::duration_cast<chrono::microseconds>(timeNow - timeStart))
            .count();

    if (lectDone && (duration > 6000000)) {
      return lectLessClosures;
    }
  }

  return lectLessClosures;
}

bool BasisCalculator::isLectGreater(
    boost::dynamic_bitset<unsigned long> &closedSet, int lectInd) {
  for (int i = 0; i <= lectInd; i++)
    if (closedSet[table.frequencyOrderedAttributes[i]]) return true;

  return false;
}

vector<implication> BasisCalculator::BSBasisToVectorBasis(
    vector<implicationBS> ansBS) {
  vector<implication> ans;

  for (int i = 0; i < ansBS.size(); i++) {
    ans.push_back(implication{utils::attrBSToAttrVector(ansBS[i].lhs),
                              utils::attrBSToAttrVector(ansBS[i].rhs)});
  }

  return ans;
}

void BasisCalculator::tryToUpdateImplicationBasis(
    vector<implicationBS> &basis) {
  std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
  double threadContextClosureTime = 0;
  lck.lock();

  while ((statistic.implicationsSeen < basis.size()) &&
         (!statistic.basisUpdate)) {
    boost::dynamic_bitset<unsigned long> A =
        basis[statistic.implicationsSeen].lhs;
    boost::dynamic_bitset<unsigned long> B =
        basis[statistic.implicationsSeen].rhs;
    int curIndex = statistic.implicationsSeen;
    statistic.implicationsSeen++;
    lck.unlock();
    boost::dynamic_bitset<unsigned long> C = A & counterExampleBS;
    statistic.aEqualToCCount++;

    if (A != C) {
      statistic.aEqualToCCount--;
      auto durBegin = chrono::high_resolution_clock::now();
      boost::dynamic_bitset<unsigned long> cC = contextClosureBS(C);
      auto durEnd = chrono::high_resolution_clock::now();
      threadContextClosureTime +=
          (chrono::duration_cast<chrono::microseconds>(durEnd - durBegin))
              .count();

      if (C == cC) {
        lck.lock();
        continue;
      }

      lck.lock();

      if (!statistic.basisUpdate) {
        statistic.basisUpdate = true;
        statistic.indexOfUpdatedImplication = curIndex;
        updatedImplication.lhs = C;
        updatedImplication.rhs = cC;
      }

      else if (statistic.basisUpdate &&
               (curIndex < statistic.indexOfUpdatedImplication)) {
        statistic.indexOfUpdatedImplication = curIndex;
        updatedImplication.lhs = C;
        updatedImplication.rhs = cC;
      }

      continue;
    }

    lck.lock();
  }

  if (threadContextClosureTime > timeStatistic.thisIterMaxContextClosure)
    timeStatistic.thisIterMaxContextClosure = threadContextClosureTime;

  lck.unlock();
}

vector<implication> BasisCalculator::generateImplicationBasis() {
  ThreadPool threadPool(statistic.maxThreads - 1);
  double prevIterTime1 = 0, prevIterTime2 = 0;
  double prevThreads1 = 1, prevThreads2 = 1;

  while (true) {
    auto start = chrono::high_resolution_clock::now();
    statistic.gCounter++;
    statistic.totTries = 0;
    getLoopCount();
    statistic.globalFlag = true;
    counterExampleBS.clear();
    timeStatistic.thisIterMaxContextClosure = 0;
    timeStatistic.thisIterMaxImplicationClosure = 0;

    if (!potentialCounterExamplesBS.empty()) {
      tryPotentialCounterExamples(basisBS);
      statistic.gCounter = 0;
    }

    if (statistic.globalFlag) {
      statistic.prevThreads = prevThreads1;
      timeStatistic.prevIter = prevIterTime1;
      setNumThreads();
      vector<std::future<void>> taskVector;

      for (int i = 1; i < statistic.numThreads; i++) {
        taskVector.emplace_back(threadPool.enqueue(
            BasisCalculator::getCounterExampleView, this, ref(basisBS), i));
      }
      //
      // This is important. If we don't write the next statement,
      // the main thread will simply keep waiting without doing anything.
      // This initially caused quite a bit of confusion, as a program without
      // multi-threading was running faster due to the main thread sitting idle.
      //
      getCounterExample(basisBS, 0);

      for (int i = 0; i < taskVector.size(); i++) {
        taskVector[i].get();
      }
    }

    timeStatistic.updown += timeStatistic.thisIterMaxContextClosure;
    timeStatistic.totalClosure += timeStatistic.thisIterMaxImplicationClosure;

    statistic.sumTotTries += statistic.totTries;
    if (statistic.globalFlag) break;

    boost::dynamic_bitset<unsigned long> X = counterExampleBS;
    statistic.totCounterExamples++;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    prevThreads1 = statistic.numThreads;
    prevIterTime1 = duration.count();
    timeStatistic.total += duration.count();
    bool found = false;
    start = chrono::high_resolution_clock::now();
    statistic.basisUpdate = false;
    statistic.implicationsSeen = 0;
    timeStatistic.thisIterMaxContextClosure = 0;

    // The algorithm implemented as-is.
    statistic.prevThreads = prevThreads2;
    timeStatistic.prevIter = prevIterTime2;
    setNumThreads();

    vector<std::future<void>> taskVector;

    for (int i = 1; i < statistic.numThreads; i++)
      taskVector.emplace_back(
          threadPool.enqueue(BasisCalculator::tryToUpdateImplicationBasisView,
                             this, ref(basisBS)));

    tryToUpdateImplicationBasis(basisBS);

    for (int i = 0; i < taskVector.size(); i++) {
      taskVector[i].get();
    }

    timeStatistic.updown += timeStatistic.thisIterMaxContextClosure;

    if (!statistic.basisUpdate)
      basisBS.push_back(implicationBS{X, contextClosureBS(X)});
    else
      basisBS[statistic.indexOfUpdatedImplication] = updatedImplication;

    end = std::chrono::high_resolution_clock::now();
    timeStatistic.totalExec2 +=
        (chrono::duration_cast<chrono::microseconds>(end - start)).count();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    prevThreads2 = statistic.numThreads;
    prevIterTime2 = duration.count();
  }

  return basis = BSBasisToVectorBasis(basisBS);
}

void BasisCalculator::printResults(double totalExecTime) {
  switch (printFormat) {
    case kCSV:
      utils::printResultAsCSV(getPrintResults(totalExecTime));
      break;
    case kReadble:
    default:
      utils::printReadbleResult(getPrintResults(totalExecTime));
      break;
  }
}

vector<string> BasisCalculator::getPrintResults(double totalExecTime) {
  vector<string> printingResults;
  for (int i = 1; i < 7; i++) {
    printingResults.push_back(argv[i]);
  }
  vector<string> results(
      {to_string(TIMEPRINT(totalExecTime)),
       to_string(TIMEPRINT(timeStatistic.total)),
       to_string(TIMEPRINT(timeStatistic.totalExec2)),
       to_string(TIMEPRINT(timeStatistic.totalClosure)),
       to_string(TIMEPRINT(timeStatistic.updown)),
       to_string(statistic.totClosureComputations),
       to_string(statistic.totUpDownComputes), to_string(basis.size()),
       to_string(statistic.totCounterExamples),
       to_string(statistic.sumTotTries), to_string(statistic.aEqualToCCount),
       to_string(statistic.emptySetClosureComputes)});
  printingResults.insert(printingResults.end(), results.begin(), results.end());
  auto supportResult = getSupportOfImplications();
  printingResults.insert(printingResults.end(), supportResult.begin(), supportResult.end());
  return printingResults;
}

vector<string> BasisCalculator::getSupportOfImplications() {
  vector<int> supports;
  for (int i = 0; i < basisBS.size(); i++) {
    int support = 0;
    for (int j = 0; j < table.objInpBS.size(); j++) {
      if (basisBS[i].lhs.is_subset_of(table.objInpBS[j])) support++;
    }
    supports.push_back(support);
  }

  sort(supports.rbegin(), supports.rend());
  double meanSupport = accumulate(supports.begin(), supports.end(), 0);
  meanSupport /= supports.size();

  return {
    to_string(100. * meanSupport / table.objInpBS.size()),
    to_string(100. * supports[0.1 * supports.size()] / table.objInpBS.size()),
    to_string(100. * supports[0.5 * supports.size()] / table.objInpBS.size()),
    to_string(100. * supports[0.9 * supports.size()] / table.objInpBS.size()),
    to_string(100. * supports[0.95 * supports.size()] / table.objInpBS.size()),
  };
}
