#ifndef MILS_H
#define MILS_H


#include <string>
#include <iostream>
#include <algorithm>
#include "Problem.h"
#include "LocalSearch.h"


using namespace std;

class Mils{

public:
	double alpha;
	double perturbationPercentage;
	Problem * blankProblem;
	double max_time = double(INT_MAX);
	double maxIter, maxILS;
	bool useTime;
	vector<LocalSearch *> localSearch;
	//virtual ~Chromosome();
	virtual ~Mils() {
		// TODO Auto-generated destructor stub
	}

	Mils(Problem * problem, double alpha, double perturbationPercentage){
		this->blankProblem = problem;
		this->alpha = alpha;
		this->perturbationPercentage = perturbationPercentage;
	}

	Problem * startLocalSearch(Problem * p, clock_t begin){
		// cout << "Started Local Search" << endl;

		bool improvement = true;
		while(improvement){
			if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time && useTime){
				// cout << iter << " " << ils << " ";
				// cout << grasp_best << " " << ils_best << " ";
				return p;
			}
			// cout << "LOOP START Cost: " << p->calculateMakespam() << " Elapsed Time: " << double(clock() - begin) / CLOCKS_PER_SEC << endl;
			// p->print();
			// cin.get();
			improvement = false;
			bool lsImprovement = false;
			double moveCost = 0.0;
			// cout << "FileAlloc" << endl;
			moveCost = p->test_swapFileAllocation();
			// cout << "ExitFileAlloc" << endl;
			if(moveCost >= 0){
				lsImprovement = true;
				// cout << "MEELHOROU COM A NOVA BL!: " << moveCost << endl;
				// p->print();
				// cin.get();
			}
			if(!p->checkFeasible()){
				cout << "booom Swap File Allocation" << endl;
				p->print();
				cin.get();
			}
			if(lsImprovement){
				improvement = true;
				continue;
			}
			// cout << "Realloc" << endl;
			// p->print();
			// moveCost = p->test_reallocate();
			// // p->print();
			// if(moveCost >= 0){
			// 	lsImprovement = true;
			// 	// cout << "MEELHOROU COM A NOVA BL!: " << moveCost << endl;
			// 	// p->print();
			// 	// cin.get();
			// }
			// if(!p->checkFeasible()){
			// 	cout << "booom Relocate!" << endl;
			// 	p->print();
			// 	cin.get();
			// }
			// if(lsImprovement){
			// 	improvement = true;
			// 	continue;
			// }
		
			// // cout << "machinePair" << endl;
			// // p->print();
			moveCost = p->test_swapMachinePair();
			// p->print();
			if(moveCost >= 0){
				lsImprovement = true;
			}
			if(!p->checkFeasible()){
				cout << "booom Swap Machine Pair" << endl;
				p->print();
				p->printAlloc();
				cin.get();
			}
			if(lsImprovement){
				improvement = true;
				continue;
			}

			// // cout << "machine" << endl;
			// // p->print();
			// moveCost = p->test_swapMachine();
			// // p->print();
			// if(moveCost > 0){
			// 	lsImprovement = true;
			// }

			// if(!p->checkFeasible()){
			// 	cout << "booom Swap MAchine" << endl;
			// 	p->print();
			// 	cin.get();
			// }

			// if(lsImprovement){
			// 	improvement = true;
			// 	continue;
			// }
		}
		// cout << "LOOP FINISHED" << endl;
		// cin.get();
		return p;
	}

	Problem * startPerturbation(Problem * p, double perturbationPercentage, int * machine, int * write){
		// cout << "Started perturbation!" << endl;
		// p->print();
		int totalPerturbations = p->alloc.size() * perturbationPercentage;
		for(int i = 0; i < totalPerturbations; i++){
			int pChooser = rand() % 2;
			// int pChooser = 2;
			int pos = rand() % p->alloc.size();
			if(pChooser == 0){
				p->perturbateMachine(pos);
				*machine = *machine + 1;
			} else if(pChooser == 1){
				p->perturbateWriteTo(pos);
				*write = *write + 1;
			}
		}

		if(!p->checkFeasible()){
			cout << "booom Start Perturbation" << endl;
			p->print();
			cin.get();
		}
		// cout << "Exited perturbation" << endl;
		// p->print();
		return p;
	}

	Problem * start(){
		// cout << "max_time: " << max_time << endl;
		Problem * p = new Problem(*this->blankProblem);
		double bestSolValue = 9999999999.0;
		Problem * bestSolution = new Problem(*this->blankProblem);
		string bestSolOrigin = "";
		int machine = 0;
		int write = 0;
		int grasp_best = -1;
		int ils_best = -1;
		bool run = true;
		int iter = -1;
		clock_t begin = clock();
		for(int iter = 0; iter < maxIter; iter++){
			if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time && useTime){
				cout << iter << " 0 ";
				// cout << grasp_best << " " << ils_best << " ";
				return bestSolution;
			}
			// if(iter % 10 == 0){
			// cout << "Iter: " << iter << endl;
			// }
			p = new Problem(*this->blankProblem);
			p->createSolution(this->alpha);
			// return p;
			// cout << "Solution created!" << endl;
			// cout << p->calculateFO() << " " << p->calculateMakespam() << " " << p->calculateCost() << endl;
			// p->print();
			// cin.get();

			// p->printAlloc();
			if(!p->checkFeasible()){
				cout << "booom create Sol" << endl;
				cin.get();
			}
			// cout << "Starting first LS!" << endl;
			p = startLocalSearch(p, begin);
			// cout << "Finished first LS!" << endl;
			if (p->calculateMakespam() < bestSolValue){
				delete bestSolution;
				bestSolution = new Problem(*p);
				bestSolValue = p->calculateMakespam();
				bestSolOrigin = "LocalSearch1";
			}

			if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time && useTime){
				cout << iter << "0 ";
				// cout << grasp_best << " " << ils_best << " ";
				return bestSolution;
			}

			// cout << "After First LocalSearch!" << endl;
			// cout << p->calculateFO() << " " << p->calculateMakespam() << " " << p->calculateCost() << endl;

			// cout << "Starting ILS" << endl;
			// cin.get();
			double lastPvalue = p->calculateMakespam();
			for(int mov = 0; mov < 130; mov++){
				break;
				if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time && useTime) {
					cout << iter << " " << mov << " ";
					// cout << grasp_best << " " << ils_best << " ";
					return bestSolution;
				}
				// if(mov % 10 == 0){
				// cout << "Mov: " << mov << endl;
				// }
				Problem * backupP = new Problem(*p);
				p = startPerturbation(p, this->perturbationPercentage, &machine, &write);
				p = startLocalSearch(p, begin);

				if(p->calculateMakespam() < lastPvalue){
					lastPvalue = p->calculateMakespam();
					delete backupP;
					backupP = new Problem(*p);
					if (p->calculateMakespam() < bestSolValue){
						grasp_best = iter;
						ils_best = mov;
						// cout << "BestSol value after localsearch: " << p->calculateMakespam() << endl;
						delete bestSolution;
						bestSolution = new Problem(*p);
						bestSolValue = p->calculateMakespam();
						bestSolOrigin = "perturbation";
					}
				} else{
					delete p;
					p = backupP;
				}
				if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time && useTime){
					// cout << grasp_best << " " << ils_best << " ";
					cout << iter << " " << mov << " ";
					return bestSolution;
				}
			}
			if(!p->checkFeasible()){
				cout << "booom create ILS" << endl;
				cin.get();
			}


			delete p;
			// cout << "ITERATION: " << iter << " CURRENT BEST SOL VALUE: " << bestSolValue << endl;
			// cin.get();
		}
		cout << iter << " " << maxILS << " ";
		// exit(1);
		// cin.get();
		// cout << grasp_best << " " << ils_best << " ";
		// p->print();
		return bestSolution;
	}
};

#endif
