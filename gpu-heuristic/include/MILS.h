#ifndef MILS_H
#define MILS_h


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

        Problem * startLocalSearch(Problem * p){
            bool improvement = true;
            while(improvement){
                // cout << "LOOP START" << endl;
                improvement = false;
                bool lsImprovement = false;
                for(int i = 0; i < p->alloc.size(); i++){ // RELOCATE LOOP START
                    for (int j = 0; j < p->alloc.size(); j++){
                        if(i == j) continue;
                        Problem * backup = new Problem(*p);
                        // bool done = p->realocate(i, j);
                        // delete backup;
                        bool done = false;
                        if(done){
                            
                            if(!p->checkFeasible()){
                                cout << "booom Relocate" << endl;
                                cin.get();
                            }
                            double solValue = p->calculateMakespam();
                            // cout << "A Move was Done! i: " << i << " j: " << j << endl;
                            // cout << "oldValue: " << backup->calculateMakespam() << " newValue: " << solValue << endl;
                            // cin.get();
                            if(solValue >= backup->calculateMakespam()){
                                delete p;
                                p = backup;
                            } else{
                                // cout << "RELOCATE IMPROV! Cost: " << p->calculateMakespam()  << endl;
                                // cin.get();
                                delete backup;
                                lsImprovement = true;
                                break;
                            }                                
                        } else{
                            delete backup;
                        }
                    }
                    if(lsImprovement){
                        break;
                    }
                } // RELOCATE LOOP END

                if(lsImprovement){
                    // cout << "MELHOROU COM A BL1" << endl;
                    // cin.get();
                    improvement = true;
                    continue;
                } 
                lsImprovement = false;
                // cout << "EXAUSTED RELOCATE! p->cost was: " << p->calculateMakespam() << endl;
                for(int i = 0; i < p->alloc.size(); i++){ // SWAP MACHINE LOOP START
                    Problem * backup = new Problem(*p);
                    // lsImprovement = p->swapMachine(i);
                    if(!p->checkFeasible()){
                        cout << "booom Swap MAchine" << endl;
                        cin.get();
                    }
                    if(lsImprovement){
                        // cout << "SWAP IMPROV! Cost: " << p->calculateMakespam()  << endl;
                        // cin.get();
                        delete backup;
                        break;
                    } else{
                        delete p;
                        p = backup;
                    }
                } // END SWAP MACHINE LOOP

                if(lsImprovement){
                    // cout << "MELHOROU COM A BL2" << endl;
                    // cin.get();
                    improvement = true;
                    continue;
                }
                lsImprovement = false;
                // cout << "EXAUSTED SWAP MACHINE! p->cost was: " << p->calculateMakespam() << endl;
                for(int i = 0; i < p->alloc.size(); i++){ // SWAP MACHINE LOOP START
                    for (int j = 0; j < p->alloc.size(); j++){
                        Problem * backup = new Problem(*p);
                        // lsImprovement = p->swapMachineWrite(i);
                        if(!p->checkFeasible()){
                            cout << "booom Swap Write" << endl;
                            cin.get();
                        }
                        if(lsImprovement){
                            delete backup;
                            break;
                        } else{
                            delete p;
                            p = backup;
                        }
                    }
                } // END SWAP MACHINE LOOP
                // cout << "EXAUSTED SWAP Write!" << endl;
                if(lsImprovement){
                    // cout << "MELHOROU COM A BL3" << endl;
                    // cin.get();
                    improvement = true;
                    continue;
                }
                lsImprovement = false;
            }  
            // cout << "LOOP FINISHED" << endl;
            // cin.get();
            return p;
        }

        Problem * startPerturbation(Problem * p, double perturbationPercentage, int * machine, int * write){
            int totalPerturbations = p->alloc.size() * perturbationPercentage;
            for(int i = 0; i < totalPerturbations; i++){
                int pChooser = rand() % 2;
                int pos = rand() % p->alloc.size();
                if(pChooser == 0){
                    // p->perturbateMachine(pos);
                    *machine = *machine + 1;
                } else if(pChooser == 1){
                    // p->perturbateWriteTo(pos);
                    *write = *write + 1;
                }
            }

            if(!p->checkFeasible()){
                cout << "booom Start Perturbation" << endl;
                cin.get();
            }
            return p;
        }

        Problem * start(clock_t begin, double max_time){
            // cout << "Begin: " << begin << " max_time: " << max_time << endl;
            Problem * p = new Problem(*this->blankProblem);           
            double bestSolValue = 9999999999.0;
            Problem * bestSolution = new Problem(*this->blankProblem);
            string bestSolOrigin = "";
            int machine = 0;
            int write = 0;
            int grasp_best = -1;
            int ils_best = -1;
            // bestSolution->createSolution(this->alpha);
            bool run = true;
            int iter = -1;
            while(run){
                iter++;
            // for(int iter = 0; iter < 10; iter++){
                if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time){ 
                    cout << grasp_best << " " << ils_best << " ";
                    return bestSolution;
                }
                // if(iter % 10 == 0){
                //     cout << "Iter: " << iter << endl;
                // }
                p = new Problem(*this->blankProblem);
                p->createSolution(this->alpha);
                // p->printAlloc();
                if(!p->checkFeasible()){
                    cout << "booom create Sol" << endl;
                    cin.get();
                }
                p = startLocalSearch(p);
                if (p->calculateMakespam() < bestSolValue){
                    delete bestSolution;
                    bestSolution = new Problem(*p);
                    bestSolValue = p->calculateMakespam();
                    bestSolOrigin = "LocalSearch1";
                }
                
                // cout << "Starting ILS" << endl;
                // cin.get();
                double lastPvalue = p->calculateMakespam();
                for(int mov = 0; mov < 10; mov++){
                    if(double(clock() - begin) / CLOCKS_PER_SEC >= max_time) { 
                        cout << grasp_best << " " << ils_best << " ";
                        return bestSolution;
                    }
                    // if(mov % 10 == 0){
                    //     cout << "Mov: " << mov << endl;
                    // }
                    Problem * backupP = new Problem(*p);
                    p = startPerturbation(p, this->perturbationPercentage, &machine, &write);    
                    p = startLocalSearch(p);
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
                }
                if(!p->checkFeasible()){
                    cout << "booom create ILS" << endl;
                    cin.get();
                }

                
                delete p;
                // cout << "ITERATION: " << iter << " CURRENT BEST SOL VALUE: " << bestSolValue << endl;
                // cin.get();
            }
            // exit(1);
            // cin.get();
            cout << grasp_best << " " << ils_best << " ";
            return bestSolution;
        }
};

#endif