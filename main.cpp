#include "lib/BasisCalculator.h"

#include <time.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <thread>
#include <set>
#include <chrono>
#include <cmath>
#include <unordered_set>

using namespace std;

int main(int argc, char **argv)
{
	auto startTime = chrono::high_resolution_clock::now();
	srand(time(NULL));
  BasisCalculator calc(argc, argv);
	
	calc.fillPotentialCounterExamples();
	vector<structs::implication> basis = calc.generateImplicationBasis();

	auto endTime = chrono::high_resolution_clock::now();
	double totalExecTime = (chrono::duration_cast<chrono::microseconds>(endTime - startTime)).count();

  calc.printResults(totalExecTime);
	return 0;
}
