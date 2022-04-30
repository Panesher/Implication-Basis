#include "Utils.h"

#include <iostream>

namespace utils {

namespace {

const std::vector<std::string> columnNames = {
    "Context",
    "Epsilon",
    "Delta",
    "Aproximation_type",
    "Distribution_type",
    "Threads",
    "Total_execution_time",
    "Total_time",
    "Total_exec_time_2",
    "Total_closure_time",
    "Updown_time",
    "Total_closure_computations",
    "Total_up_down_computes",
    "Basis_size",
    "Total_counter_examples",
    "Sum_total_tries",
    "Equal_to_count",
    "Empty_set_closure_computes",
};

}  // namespace

void printUsageAndExit() {
  std::cout << "Usage: ./algo.out <path/to/context.txt> <Epsilon> <Delta> "
               "<strong/weak> "
               "<uniform/frequent/area-based/squared-frequency> <number of "
               "threads> none <csv/csv-with-header/readable>\n";
  exit(0);
}

void printVector(std::vector<int> &A) {
  for (auto x : A) {
    std::cout << x << " ";
  }

  std::cout << "\n";
}

void printReadbleResult(const std::vector<std::string> &printingResults) {
  if (printingResults.size() != columnNames.size()) {
    for (const auto &printingResult : printingResults)
      std::cout << printingResult << "\n";
    std::cout << std::endl;
    return;
  }
  for (size_t i = 0; i < printingResults.size(); ++i) {
    std::cout << columnNames[i] << ": " << printingResults[i] << "\n";
  }
}

void printResultAsCSV(const std::vector<std::string> &printingResults) {
  for (const auto &printingResult : printingResults) {
    std::cout << printingResult;
    if (printingResult != printingResults.back()) {
      std::cout << ",";
    }
  }
  std::cout << std::endl;
}

void printCSVHeader() {
  for (const auto &columnName : columnNames) {
    if (columnName != columnNames.front()) {
      std::cout << ",";
    }
    std::cout << columnName;
  }
  std::cout << "\n";
}

std::vector<int> attrBSToAttrVector(
    boost::dynamic_bitset<unsigned long> &attrBS) {
  std::vector<int> ans;
  for (int i = 0; i < attrBS.size(); i++) {
    if (attrBS[i]) ans.push_back(i);
  }

  return ans;
}

boost::dynamic_bitset<unsigned long> attrVectorToAttrBS(
    std::vector<int> &attrVec, size_t size) {
  boost::dynamic_bitset<unsigned long> ans(size);

  for (int i = 0; i < attrVec.size(); i++) {
    ans[attrVec[i]] = true;
  }

  return ans;
}

}  // namespace utils