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

// void getSupportOfImplications()
// {
// 	vector<int> supports;

// 	for (int i = 0; i < ansBasisBS.size(); i++)
// 	{
// 		int support = 0;

// 		for (int j = 0; j < objInpBS.size(); j++)
// 		{
// 			if (ansBasisBS[i].lhs.is_subset_of(objInpBS[j]))
// 				support++;
// 		}

// 		supports.push_back(support);
// 	}

// 	sort(supports.rbegin(), supports.rend());
// 	double meanSupport = accumulate(supports.begin(), supports.end(), 0);
// 	meanSupport /= supports.size();
// 	double p10, p50, p90, p95;
// 	p10 = supports[0.1 * supports.size()];
// 	p50 = supports[0.5 * supports.size()];
// 	p90 = supports[0.9 * supports.size()];
// 	p95 = supports[0.95 * supports.size()];
// 	cout << 100 * meanSupport / objInpBS.size() <<";";
// 	cout << 100 * p10 / objInpBS.size() <<";";
// 	cout << 100 * p50 / objInpBS.size() <<";";
// 	cout << 100 * p90 / objInpBS.size() <<";";
// 	cout << 100 * p95 / objInpBS.size() <<"\n";
// 	return;
// }

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

	// if (implicationSupport)
	// {
	// 	getSupportOfImplications();
	// 	return 0;
	// }

	// cout << allContextClosures() << "," << flush;
	// cout << allImplicationClosures() << endl;

	// for (auto x : basis) {
	// 	// //cout << "Implication\n";
	// 	printVector(x.lhs);
	// 	printVector(x.rhs);
	// }
	return 0;
}
