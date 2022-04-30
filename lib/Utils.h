#ifndef __UTILS_H__
#define __UTILS_H__

#include <boost/dynamic_bitset.hpp>
#include <string>
#include <vector>

namespace utils {

void printUsageAndExit();
void printVector(std::vector<int> &A);
void printReadbleResult(const std::vector<std::string> &printingResults);
void printResultAsCSV(const std::vector<std::string> &printingResults);
void printCSVHeader();

std::vector<int> attrBSToAttrVector(
    boost::dynamic_bitset<unsigned long> &attrBS);

boost::dynamic_bitset<unsigned long> attrVectorToAttrBS(
    std::vector<int> &attrVec, size_t size);

}  // namespace utils

#endif  // __UTILS_H__
