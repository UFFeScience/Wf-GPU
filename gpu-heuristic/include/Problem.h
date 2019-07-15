#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <iostream>
#include <vector>
#include <list>
#include <limits>
#include <algorithm>
#include <random>
#include <regex>


using namespace std;

class Item{
public:
	int id = -1;
	int name;
	double size;
	int alocated_vm_id = -1;
	vector<int> static_vms;
	bool is_static = false;
	vector<double> IOTimeCost;
	vector<double> VMsBandwidth;
	Item(){};
	Item(int name, int id, double size) : name(name), id(id), size(size), is_static(false) {}
	Item(int name, int id, double size, vector<int> static_vms) : name(name), id(id), size(size), is_static(true), static_vms(static_vms) {}
};

class Job{
public:
	string name;
	int id;
	double base_time_cpu;
	double base_time_gpu;
	bool rootJob;
	bool alocated = false;
	bool gpu = false;
	bool on_gpu = false;
	int alocated_vm_id = -1;
	vector<Item*> input, output;

	bool checkJobFeasibleOnMachine(int vm_id){
		// cout << "JobNameTested: " << this->name << endl;
		// cout << "testing feasible on Machine: " << vm_id << endl;
		if(rootJob){
		// cout << "IsRoot" << endl;
			for(unsigned int f = 0; f < input.size(); f++){
				if(input[f]->is_static){
					bool feasible = false;
					// cout << "StaticVMs( " << input[f]->static_vms.size() << "):";
					if(input[f]->alocated_vm_id < 0){
						for(unsigned int vm = 0; vm < input[f]->static_vms.size(); vm++){
							// cout << input[f]->static_vms[vm] << " "; 
							if(input[f]->static_vms[vm] == vm_id){
								feasible = true;
								break;
							}
						}
						// cout << endl;
						if(!feasible)
							return false;
					}
				}
			}
		}
		return true;
	}

	bool checkInputNeed(int item_id){
		for(unsigned int i = 0; i < input.size(); i++){
			if(input[i]->id == item_id)
				return true;
		}
		return false;
	}

	bool checkOutputNeed(int item_id){
		for(unsigned int i = 0; i < output.size(); i++){
			if(output[i]->id == item_id)
				return true;
		}
		return false;
	}

	Job(){};
};

class Machine{
public:
	string name;
	int id;
	double cpu_slowdown, gpu_slowdown, storage, usage_cost;
	vector<vector<double>> bandwidth;
	vector<Job*> timelineJobs;
	vector<double> timelineStartTime;
	vector<double> timelineFinishTime;
	double makespam = 0.0;
	double cost = 0.0;
	bool hasGpu;

	double calculateLocalspam(){
		if(timelineFinishTime.size() == 0){
			this->makespam = 0.0;
			return 0.0;
		}
		this->makespam = timelineFinishTime[timelineFinishTime.size() - 1];
		return this->makespam; 
	}

	double calculateCost(){
		this->cost = 0.0;
		for(int i = 0; i < timelineJobs.size(); i++){
			if(hasGpu) {
				this->cost += ceil((timelineJobs[i]->base_time_gpu * this->gpu_slowdown)) * usage_cost;
			}
			else {
				this->cost += ceil((timelineJobs[i]->base_time_cpu * this->cpu_slowdown)) * usage_cost;
			}
		}
		return this->cost;
	}

	bool popJob(int jobId){
		for(unsigned int i = 0; i < timelineJobs.size(); i++){
			if (timelineJobs[i]->id == jobId){
				timelineJobs[i]->alocated = false;
				timelineJobs[i]->on_gpu = false;
				for(unsigned int f = 0; f < timelineJobs[i]->output.size(); f++){
					timelineJobs[i]->output[f]->alocated_vm_id = -1;
				}
				if(timelineJobs[i]->rootJob){
					for(unsigned int f = 0; f < timelineJobs[i]->input.size(); f++){
						if(timelineJobs[i]->input[f]->is_static)
							timelineJobs[i]->input[f]->alocated_vm_id = -1;
					}	
				}
				timelineJobs.erase(timelineJobs.begin() + i);
				timelineStartTime.erase(timelineStartTime.begin() + i);
				timelineFinishTime.erase(timelineFinishTime.begin() + i);
				return true;
			}
		}
		return false;
	}

	// bool pushJob(Job * job, int write_vm_id, double minSpam, bool GPU){
	bool pushJob(Job * job, int write_vm_id, double minSpam){
		if(write_vm_id < 0){
			cout << "123123123123" << endl;
			cin.get();
		}
		// if(!job->checkJobFeasibleOnMachine(this->id)){
		// 	cout << "not feasible on machine" << endl;
		// 	return false;
		// }
		// cout << "feasible on machine" << endl;
		// cout << "MinSpam: " << minSpam << endl;
		// cout << "JobID: " << job->id << endl;
		// cin.get();
		for(unsigned int i = 0; i < job->input.size(); i++){ // checando se todos os arquivos de input que nao sao estaticos ja foram produzidos
			// cout << "inputFileID: " << job->input[i]->id << " isStatic: " << job->input[i]->is_static << endl;
			if(job->input[i]->is_static == false && job->input[i]->alocated_vm_id < 0){
				// cout << "JOB NOT AVAILABLE: FILES NOT READY" << endl; 
				return false;
			}
		}


		double startTime; // calculando tempo de inicio.
		if (timelineJobs.size() == 0)
			startTime = 0.0;
		else
			startTime = (timelineFinishTime[timelineFinishTime.size() - 1]) + 1;

		if(startTime < minSpam)
			startTime = minSpam;

		// cout << "StartTime: " << startTime << endl;
		// cin.get();
		
		
		
		double readtime = 0.0; // calculando o tempo de leitura de todos os arquivos de input necessarios caso nao estejam alocados na Maquina
		for(unsigned int i = 0; i < job->input.size(); i++){
			if(job->input[i]->alocated_vm_id == this->id){
				readtime += 1.0;
				continue;
			}
			int source;
			if(job->input[i]->is_static){
				double readTime = INFINITY;
				for(int j = 0; j < job->input[i]->static_vms.size(); j++){
					double currentReadTime;
					if(job->input[i]->static_vms[j] == this->id){
						currentReadTime = 0.0;
					} else{
						currentReadTime = ceil(job->input[i]->size / this->bandwidth[job->input[i]->static_vms[j]][this->id]);
					}
					if(currentReadTime < readTime){
						readTime = currentReadTime;
					}
				}
				readtime += readTime;
			} else{
				double readTime;
				if(this->bandwidth[job->input[i]->alocated_vm_id][this->id] == 0.0){
					readTime = 0.0;
				} else
					readTime = ceil(job->input[i]->size / this->bandwidth[job->input[i]->alocated_vm_id][this->id]);
				readtime += readTime;	
			}
		}

		// cout << "ReadTime: " << readtime << endl;
		// cout << "test " << this->id << " " << write_vm_id << endl;
		// cout << this->bandwidth[this->id][write_vm_id] << endl;
		// cout << "test2" << endl;
		// cin.get();

		double writetime = 0.0;
		if(write_vm_id != -1 && write_vm_id != this->id){
			for(unsigned int i = 0; i < job->output.size(); i++){
				// cout << "size: " << job->input[i]->size << endl;
				writetime += ceil(job->output[i]->size / this->bandwidth[this->id][write_vm_id]);
			}
		} else{
			writetime = 1.0 * job->output.size();
		}

		// if(readtime == 0){
		// 	readtime = 1;
		// }
		// if(writetime = 0){
		// 	writetime = 1;
		// }

		// cout << "12" << endl;
		double processtime;
		// if(GPU){
		// 	job->on_gpu = true;
		// 	processtime = ceil(job->base_time_gpu * this->gpu_slowdown);
		// }
		// else
		// 	processtime = ceil(job->base_time_cpu * this->cpu_slowdown);
		// // double processtime = ceil(job->base_time * this->slowdown);
		if(this->hasGpu && job->gpu){
			processtime = ceil(job->base_time_gpu * this->gpu_slowdown);
			job->on_gpu = true;
		} else if(!this->hasGpu || !job->gpu){
			processtime = ceil(job->base_time_cpu * this->cpu_slowdown);
			job->on_gpu = false;
		}
		double finishTime = readtime + writetime + processtime + startTime;

		// if(job->name == "ID00002"){
			// cout << "ProcessMachineID: " <<this->id << " ProcessMAchineName: " << this->name << " writeID: " << write_vm_id << " minSpam: " << minSpam << endl;
			// cout << "start: " << startTime << " read: " << readtime << " process: " << processtime << " write: " << writetime << " fulltime: " << finishTime << endl;
			// cin.get();
		// }

		timelineStartTime.push_back(startTime);
		timelineFinishTime.push_back(finishTime);
		timelineJobs.push_back(job);

		job->alocated = true;
		job->alocated_vm_id = this->id;
		for(unsigned int i = 0; i < job->output.size(); i++){
			if(job->output[i]->is_static) continue;
			int final_vm = this->id;
			if(write_vm_id != -1)
				final_vm = write_vm_id;
			// cout << "FinalWriteVM: " << final_vm << endl;
			if(final_vm < 0) {cin.get();}
			job->output[i]->alocated_vm_id = final_vm;
		}

		// for(unsigned int i = 0; i < job->input.size(); i++){
		// 	if(job->input[i]->is_static){
		// 		if(job->input[i]->alocated_vm_id < 0)
		// 			job->input[i]->alocated_vm_id = this->id;
		// 	}
		// }

		return true;
	}

	int jobPosOnTimeline(int id){
		for(unsigned int i = 0; i < this->timelineJobs.size(); i++){
			if(this->timelineJobs[i]->id == id)
				return i;
		}
		return -1;
	}

	bool timeLineFeasible(){
		for(unsigned int i = 1; i < this->timelineJobs.size(); i++){
			if(this->timelineFinishTime[i - 1] > this->timelineStartTime[i])
				return false;
		}
		return true;
	}

	bool checkFeasible(){
		return timeLineFeasible();
	}



	Machine(){}
};

class Allocation{
public:
	Job * job;
	Machine * vms;
	bool GPU;
	int writeTo;
};

class Problem{

public:
	vector<vector<int>> conflicts;
	vector<vector<int>> notParallelable;
	vector<Item*> files;
	vector<Job*> jobs;
	vector<Machine*> vms;
	vector<Allocation*> alloc;
	vector<string> name_map;
	vector<int> name_map_code;
	double maxTime, maxCost;
	double ponderation = 0.5;

	void printAlloc(){
		// cout << "Allocation order that created solution: " << endl;
		// for(unsigned int i = 0; i < alloc.size(); i++){
		// 	cout << "JobID: " << alloc[i]->job->name << " to MachineID: " << alloc[i]->vms->id << " Writing to MachineID: " << alloc[i]->writeTo << endl;
		// }
		for(unsigned int i = 0; i < alloc.size(); i++){
			cout << alloc[i]->job->name << ", ";
		}
		cout << endl;
	}

	Problem(const Problem &p){
		this->conflicts = p.conflicts;
		this->notParallelable = p.notParallelable;
		this->name_map = p.name_map;
		this->name_map_code = p.name_map_code;

		for(unsigned int i = 0; i < p.files.size(); i++){			
				Item * copiedItem = p.files[i];
				Item * newItem = new Item();
				newItem->id = copiedItem->id;
				newItem->name = copiedItem->name;
				newItem->size = copiedItem->size;
				newItem->alocated_vm_id = copiedItem->alocated_vm_id;
				newItem->static_vms = copiedItem->static_vms;
				newItem->is_static = copiedItem->is_static;
				newItem->IOTimeCost = copiedItem->IOTimeCost;
				newItem->VMsBandwidth = copiedItem->VMsBandwidth;
				this->files.push_back(newItem);
		}

		for(unsigned int i = 0; i < p.jobs.size(); i++){
			Job * copiedJob = p.jobs[i];
			Job * newJob = new Job();
			newJob->name = copiedJob->name;
			newJob->id = copiedJob->id;
			newJob->base_time_cpu = copiedJob->base_time_cpu;
			newJob->base_time_gpu = copiedJob->base_time_gpu;
			newJob->rootJob = copiedJob->rootJob;
			newJob->alocated = copiedJob->alocated;
			newJob->alocated_vm_id = copiedJob->alocated_vm_id;
			for(unsigned int input = 0; input < copiedJob->input.size(); input++){
				for(unsigned int item = 0; item < this->files.size(); item++){
					if(this->files[item]->id == copiedJob->input[input]->id){
						newJob->input.push_back(this->files[item]);
						break;
					}
				}
			}
			for(unsigned int output = 0; output < copiedJob->output.size(); output++){
				for(unsigned int item = 0; item < this->files.size(); item++){
					if(this->files[item]->id == copiedJob->output[output]->id){
						newJob->output.push_back(this->files[item]);
						break;
					}
				}
			}
			this->jobs.push_back(newJob);
		}

		for(unsigned int i = 0; i < p.vms.size(); i++){
			Machine * copiedMachine = p.vms[i];
			Machine * newMachine = new Machine();
			newMachine->name = copiedMachine->name;
			newMachine->id = copiedMachine->id;
			newMachine->gpu_slowdown = copiedMachine->gpu_slowdown;
			newMachine->cpu_slowdown = copiedMachine->cpu_slowdown;
			newMachine->storage = copiedMachine->storage;
			newMachine->cost = copiedMachine->cost;
			newMachine->usage_cost = copiedMachine->usage_cost;
			newMachine->hasGpu = copiedMachine->hasGpu;
			newMachine->bandwidth = copiedMachine->bandwidth;
			newMachine->timelineStartTime = copiedMachine->timelineStartTime;
			newMachine->timelineFinishTime = copiedMachine->timelineFinishTime;
			newMachine->makespam = copiedMachine->makespam;
			for(unsigned int job = 0; job < copiedMachine->timelineJobs.size(); job++){
				for(unsigned int pos = 0; pos < this->jobs.size(); pos++){
					if(this->jobs[pos]->id == copiedMachine->timelineJobs[job]->id){
						newMachine->timelineJobs.push_back(this->jobs[pos]);
						break;
					}
				}
			}
			this->vms.push_back(newMachine);
		}

		for(unsigned int i = 0; i < p.alloc.size(); i++){
			Allocation * copiedAllocation = p.alloc[i];
			Allocation * newAllocation = new Allocation();
			newAllocation->writeTo = copiedAllocation->writeTo;
			int copiedJobId = copiedAllocation->job->id;
			newAllocation->job = this->jobs[copiedJobId];
			int copiedVmsId = copiedAllocation->vms->id;
			newAllocation->vms = this->vms[copiedVmsId];
			newAllocation->GPU = copiedAllocation->GPU;
			this->alloc.push_back(newAllocation);
		}

	}

	double createSolution(double alpha){
		// cout << "Create Solution..." << endl;
		int totalJobs = jobs.size();
		while(totalJobs > 0){
			// cout << "TotalJobs: " << totalJobs << endl;
			vector<Job*> CL;
			vector<int> jobVmDestination;
			vector<int> outputVmDestination;
			vector<bool> onGpu;
			vector<double> cost;
			// cout << "JobsSize: " << jobs.size() << endl;
			// cout << "VMSSize: " << vms.size() << endl;
			// cin.get();
			for(unsigned int i = 0; i < jobs.size(); i++){
				if (jobs[i]->alocated)
					continue;
				// cout << "CL for JOBID: " << jobs[i]->id << endl;
				for(unsigned int m = 0; m < vms.size(); m++){
					// cout << "M: " << m << endl;
					for(unsigned int d = 0; d < vms.size(); d++){
						// cout << "D: " << d << endl;
						// for(unsigned int gpu = 0; gpu < 2; gpu++){
						// 	// cout << "ALOW" << endl;
						// 	bool useGpu = false;
						// 	if(gpu == 0 ){ // Use GPU
						// 		useGpu = true;
						// 	}
							// if(vms[m]->hasGpu) useGpu = true;
							// if(useGpu && !jobs[i]->gpu && !vms[m]->hasGpu)	continue;
							// cout << "Getting min spam" << endl;
							double minSpam = getJobConflictMinSpam(jobs[i]);
							// cout << "Got min spam" << endl;
							// cout << "MinSpam: " << minSpam << endl;
							if(minSpam < 0) break;
							// cin.get();
							// cout << "vmssize: " << vms.size() << endl;
							// cout << "Pushing job.." << endl;
							// bool pushed = vms[m]->pushJob(jobs[i], d, minSpam, useGpu);
							bool pushed = vms[m]->pushJob(jobs[i], d, minSpam);
							// cout << "Pushed job.." << endl;
							if(!pushed){
								break;
							}
							// double insertionCost = calculateMakespam();
							double insertionCost = calculateFO();

							// cout << "JobID: " << jobs[i]->id << " Machine: " << m << " WriteTo: " << d << " GPU: " << useGpu << " cost: " << insertionCost << endl;
							// cout << "JobID: " << jobs[i]->id << " Machine: " << m << " WriteTo: " << d << " cost: " << insertionCost << endl;
							// cin.get();

							if(CL.size() == 0){
								CL.push_back(jobs[i]);
								jobVmDestination.push_back(m);
								cost.push_back(insertionCost);
								outputVmDestination.push_back(d);
								// onGpu.push_back(useGpu);
							} else{
								bool inserted = false;
								for(unsigned int j = 0; j < cost.size(); j++){
									if(cost[j] >= insertionCost){
										cost.insert(cost.begin() + j, insertionCost);
										jobVmDestination.insert(jobVmDestination.begin() + j, m);
										CL.insert(CL.begin() + j, jobs[i]);
										outputVmDestination.insert(outputVmDestination.begin() + j, d);
										// onGpu.insert(onGpu.begin() + j, useGpu);
										inserted = true;
										break;
									}
								}
								if (!inserted){
									cost.push_back(insertionCost);
									jobVmDestination.push_back(m);
									CL.push_back(jobs[i]);
									outputVmDestination.push_back(d);
									// onGpu.push_back(useGpu);
								}
							}
							// cout << "tested!" << endl;
							vms[m]->popJob(jobs[i]->id);
							// cout << "Spam After Removal: " << vms[m]->calculateLocalspam() << endl;
							// cin.get();
						// }
					}
					// cin.get();
				}
			}

			
			if(CL.size() == 0){
				cout << "NO POSSIBLE MOVEMENTS!" << endl;
				cin.get();
			}

			// cout << "CL size: " << CL.size() << endl;
			// for(unsigned int c = 0; c < CL.size(); c++){
			// 	cout << CL[c]->id << " GPU:" << onGpu[c] << " Destination: " <<jobVmDestination[c] << " OutPut: " << outputVmDestination[c] <<  " Cost: " << cost[c] << endl;
			// }
			// cout << endl;
			// cin.get();


			int chosenMovement;
			int maxClPos = (int)(CL.size() * alpha);
			if(maxClPos == 0)
				chosenMovement = 0;
			else
				chosenMovement = random() % maxClPos;

			// cout << "Chosen Movement: " << chosenMovement << endl;
			// cin.get();
			// cout << "CLSIZE: " << CL.size() << endl;
			// bool moveDone = doMovement(jobVmDestination[chosenMovement], outputVmDestination[chosenMovement], CL[chosenMovement], onGpu[chosenMovement]);
			bool moveDone = doMovement(jobVmDestination[chosenMovement], outputVmDestination[chosenMovement], CL[chosenMovement], true);
			if(moveDone){
				// cout << "JobID: " << CL[chosenMovement]->id << " Was Inserted!" << endl;
				totalJobs--;
			}
			// else
				// cout << "JobID: " << CL[chosenMovement]->id << " Was NOT Inserted!" << endl;
			
			// cout << "Spam: " << calculateMakespam() << endl;
			// cin.get();

		}
		// cout << "Testing solution..." << endl;
		// cout << "[";
		// for(unsigned int i = 0; i < jobs.size();i++){
		// 	cout << jobs[i]->alocated_vm_id << ",";
		// }
		// cout << "]" << endl;
		// cout << "[";
		// for(unsigned int i = 0; i < jobs.size();i++){
		// 	cout << jobs[i]->alocated << ",";
		// }
		// cout << "]" << endl;
		// cin.get();
		// vector<int> vmUsage(vms.size());
		// for(unsigned int f = 0; f < files.size(); f++){
		// 	vmUsage[files[f]->alocated_vm_id] += files[f]->size;
		// }
		// for(unsigned int vm = 0; vm < vms.size(); vm++){
		// 	cout << "vmName: " << vms[vm]->name << " vmTotalSize: " << vms[vm]->storage << " usage: " << vmUsage[vm] << endl; 
		// }
		// cin.get();
		return calculateFO();
	}

	bool doMovement(int vm, int output, Job* job, bool GPU){
		Allocation * newAlloc = new Allocation();
		newAlloc->job = job;
		newAlloc->vms = vms[vm];
		newAlloc->writeTo = output;
		// newAlloc->GPU = GPU;
		alloc.push_back(newAlloc);
		// return vms[vm]->pushJob(job, output, getJobConflictMinSpam(job), GPU);
		return vms[vm]->pushJob(job, output, getJobConflictMinSpam(job));
	}

	void preSetStaticFile(Item * file, int vm_id){
		file->alocated_vm_id = vm_id;
	}

	void print(){
		cout << "VMs:" << endl;
		for(unsigned int i = 0; i < vms.size(); i++){
			string gpu = "False";
			if(vms[i]->hasGpu) gpu = "True";
			cout << "ID: " << vms[i]->id << " Name:" << vms[i]->name << " hasGpu: " << gpu << ": " ;
			for(unsigned int j = 0; j < vms[i]->timelineJobs.size(); j++){
				gpu = "False";
				if(vms[i]->timelineJobs[j]->on_gpu) gpu = "True";
				cout << vms[i]->timelineJobs[j]->name  << " onGPU: " << gpu << " ( " << vms[i]->timelineStartTime[j] << "," << vms[i]->timelineFinishTime[j] << " )" << " ";
			}
			cout << endl;
		}

		for(unsigned int i = 0; i < files.size(); i++){
			cout << "file: " << files[i]->id << " VM: " << files[i]->alocated_vm_id << endl;
		}
	}

	double calculateMakespam(){
		double makespam = 0.0;
		for(unsigned int i = 0; i < vms.size(); i++){
			double localspam = vms[i]->calculateLocalspam();
			// cout << "VM: " << vms[i]->id << "Localspam: " << localspam << endl;
			if (localspam > makespam) makespam = localspam;
		}
		return makespam;
	}

	double calculateFO(){
		// cout << "Makespam: " << this->calculateMakespam() << endl;
		// cout << "Cost: " << this->calculateCost() << endl;
		// cout << "FO: " << this->ponderation*(this->calculateMakespam() / this->maxTime) + (1.0 - this->ponderation)*(this->calculateCost() / this->maxCost) << endl;
		return this->ponderation*(this->calculateMakespam() / this->maxTime) + (1.0 - this->ponderation)*(this->calculateCost() / this->maxCost);
		// return this->calculateMakespam();
	}

	double calculateCost(){
		double cost = 0.0;
		for(unsigned int i = 0; i < vms.size(); i++){
			cost += vms[i]->calculateCost();
		}
		return cost;
	}

	bool checkFeasible(){
		bool feasible = true;
		for(unsigned int i = 0; i < files.size(); i++){  // checando se todos os arquivos estao alocados a alguma maquina
			if(files[i]->is_static) continue;
			if(files[i]->alocated_vm_id < 0){
				cout << "ArquivoName: " << files[i]->name << " nao alocado!" << endl;
				feasible = false;
			}
		}

		// if(!feasible) return feasible;

		for(unsigned int i = 0; i < jobs.size(); i++){ // checando se todos os jobs foram alocados
			if(!jobs[i]->alocated){
				cout << "Job: " << i << " nao alocado!" << endl;
				feasible = false;
			}
			if(jobs[i]->alocated_vm_id < 0){
				cout << "Job: " << i << " alocado, mas sem vm_id!" << endl;
				feasible = false;
			}

			//checando se topologia foi respeitada
			for(unsigned int j = 0; j < jobs.size();j++){
				if(conflicts[i][j] == 0)
					continue;
				if(jobs[j]->alocated == false){
					cout << "Job: " << i << " nao teve topologia respeitada! Pre-Job: " << j << endl;
					feasible =  false;
				}

				double preJobFinishtime = 0.0;
				Machine * preJobMachine = vms[jobs[j]->alocated_vm_id];
				int pos = preJobMachine->jobPosOnTimeline(jobs[j]->id);
				preJobFinishtime = preJobMachine->timelineFinishTime[pos];
				
				double jobStartTime = 0.0;
				Machine * JobMachine = vms[jobs[i]->alocated_vm_id];
				pos = JobMachine->jobPosOnTimeline(jobs[i]->id);
				jobStartTime = JobMachine->timelineStartTime[pos];
				// cout << "Job: " << i << " comecou no tempo: " << jobStartTime << ", mas preJob:" << j << " terminou no tempo: " << preJobFinishtime << endl;
				if(jobStartTime < preJobFinishtime){
					cout << "Job: " << i << " comecou no tempo: " << jobStartTime << ", mas preJob:" << j << " terminou no tempo: " << preJobFinishtime << endl;
					feasible =  false;
				}

			}

		}	


		return feasible;

	}

	double getJobConflictMinSpam(Job * job){
		// cout << "JOBID: " << job->id << endl;
		// cout << conflicts.size() << endl;
		// cout << "123" << endl;
		int id = job->id;
		double minSpam = 0.0;
		for(unsigned int i = 0; i < conflicts[id].size(); i++){ // checando tempo minimo baseado na topologia até o arquivo.
			if(conflicts[id][i] == 0)
				continue;
			if(jobs[i]->alocated == false)
				return -1.0;
			int machineID = jobs[i]->alocated_vm_id;
			Machine * vm = vms[machineID];
			double spam = vm->calculateLocalspam();
			if(spam > minSpam)
				minSpam = spam;
		}
		// cout << "MinSpam: " << minSpam << endl;
		// for(unsigned int i = 0; i < notParallelable[id].size(); i++){ // checando tempo minimo baseado na possivel paralelização
		// 	cout << "i: " << i << " notParallelable: " << notParallelable[id][i] << endl;
		// 	if(notParallelable[id][i] == 0)
		// 		continue;
		// 	if(jobs[i]->alocated == false){
		// 		cout << "Conflictuous job not yet assigned" << endl;
		// 		continue;
		// 	}
		// 	int machineID = jobs[i]->alocated_vm_id;
		// 	Machine * vm = vms[machineID]; 
		// 	double spam = vm->calculateLocalspam();
		// 	if(spam > minSpam)
		// 		minSpam = spam;
		// }
		// cout << "MinSpam: " << minSpam << endl;
		// cin.get();
		if (minSpam > 0) minSpam = minSpam + 1;
		return minSpam;
	}

	double Simulate(){

		
		return 0.0;
		// cin.get();
	}

	Item * getFileByName(int name){
		Item * to_return = NULL;

		// if(name == "PeakVals_FFI_0_2_ID00018.bsa"){
		// 	cin.get();
		// }
		for(unsigned int i = 0; i < this->files.size(); i++){
			// cout << "FileName: |" << this->files[i]->name << "|" << name << "|" << endl;
			// if(this->files[i]->name == name) cout << "EQUAL" << endl;
			if(this->files[i]->name == name)
				to_return = this->files[i];
		}
		if(to_return == NULL){
			cout << "Could not find file with name: " << name << endl;
			cin.get();
		}
		return to_return;
	}

	Job * getJobByName(string name){
		for(unsigned int i = 0; i < this->jobs.size(); i++){
			if(this->jobs[i]->name == name)
				return this->jobs[i];
		}
		return NULL;
	}

	Problem(string file){
		// cout << "Starting to read instance..." << endl;

		ifstream in_file(file);
		string line;

		getline(in_file, line);
		vector<string> tokens;
		boost::split(tokens, line, boost::is_any_of(" "));

		int jobs = stoi(tokens[0]);
		int files = stoi(tokens[1]);
		int vms = stoi(tokens[2]);
		
		// cout << "Jobs: " << jobs << " files: " << files << " vms: " << vms << endl;

		vector<string> job_lines;

		for(int j = 0; j < jobs; j++){
			getline(in_file, line);
			job_lines.push_back(line);
			// cout << "ReadLine: " << line << endl;
		}

		// cin.get();
		for(int f = 0; f < files; f++){
			getline(in_file, line);
			// cout << "ReadLine: " << line << endl;
			boost::split(tokens, line, boost::is_any_of(" "));
			int file_name = stoi(tokens[0]);
			// cout << "Filename: " << file_name << " f: " << f << endl;
			// name_map.push_back(file_name);
			// file_name = to_string(f);
			// name_map_code.push_back(f);
			double file_size = stod(tokens[1]);
			int is_static = stoi(tokens[2]);
			// cout << "file_name: |" << file_name << "| file_size: " << file_size << " is_static: " << is_static << endl;
			Item * newItem;
			if(is_static == 1){
				int n_static_vms = stoi(tokens[3]);
				vector<int> static_vms;
				for(int s = 0; s < n_static_vms; s++){
					static_vms.push_back(stoi(tokens[4+s]));
				}
				newItem = new Item(file_name, f, file_size, static_vms);
			} else{
				newItem = new Item(file_name, f, file_size);
			}

			this->files.push_back(newItem);
		}

		// cout << "Finished reading files.." << endl;
		// cin.get();

		double total_time_job_cpu = 0.0;
		double total_time_job_gpu = 0.0;
		for(int j = 0; j < jobs; j++){
			line = job_lines[j];
			// cout << "Stored line: " << line << endl;
			boost::split(tokens, line, boost::is_any_of(" "));
			string job_name = tokens[0];

			double cpu_time = stod(tokens[1]);
			total_time_job_cpu += cpu_time;
			double gpu_time = stod(tokens[2]);
			total_time_job_gpu += gpu_time;
			int n_input = stoi(tokens[3]);
			// cout << "n_input: " << n_input << endl;
			vector<Item*> input, output;
			for(int i = 0; i < n_input; i++){
				// cout << "Reading pos: " << 4+i << endl;
				int id = stoi(tokens[4+i]);
				Item * item = this->files[id];
				input.push_back(item);
			}
			int n_output = stoi(tokens[4+n_input]);
			// cout << "n_output: " << n_output << endl;
			for(int i = 0; i < n_output; i++){
				int id = stoi(tokens[4+n_input+1+i]);
				Item * item = this->files[id];
				output.push_back(item);
			}
			// cout << "ALOW" << endl;
			Job* newJob = new Job();
			newJob->name = job_name;
			newJob->id = j;
			newJob->base_time_cpu = cpu_time;
			newJob->base_time_gpu = gpu_time;
			newJob->input = input;
			newJob->output = output;
			if(gpu_time > 0){
				newJob->gpu = true;
			}
			this->jobs.push_back(newJob);
		}

		// cout << "Finished reading jobs.." << endl;

		vector<vector<string>> machines;
		getline(in_file, line);
		// cout << "ReadLine: " << line << endl;
		boost::split(tokens, line, boost::is_any_of(" "));
		machines.push_back(tokens);
		getline(in_file, line);
		boost::split(tokens, line, boost::is_any_of(" "));
		machines.push_back(tokens);
		getline(in_file, line);
		boost::split(tokens, line, boost::is_any_of(" "));
		machines.push_back(tokens);
		getline(in_file, line);
		boost::split(tokens, line, boost::is_any_of(" "));
		machines.push_back(tokens);

		// cout << "Finished reading Vms.." << endl;

		// for(int y = 0; y < machines.size(); y++){
		// 	for(int z = 0; z < machines[y].size(); z++){
		// 		cout << machines[y][z] << " ";
		// 	}
		// 	cout << endl;
		// }

		// cin.get();

		double slowest_machine_cpu = 0.0;
		double more_expensive_process_cpu = 0.0;
		double slowest_machine_gpu = 0.0;
		double more_expensive_process_gpu = 0.0;

		for(int m = 0; m < vms; m++){
			Machine * newVM = new Machine();
			newVM->name = "";
			newVM->id = m;
			newVM->cpu_slowdown = stod(machines[0][m]);
			if(newVM->cpu_slowdown > slowest_machine_cpu) slowest_machine_cpu = newVM->cpu_slowdown;
			newVM->gpu_slowdown = stod(machines[1][m]);
			if(newVM->gpu_slowdown == 0.0)
				newVM->hasGpu = false;
			else
				newVM->hasGpu = true;
			if(newVM->gpu_slowdown > slowest_machine_gpu) slowest_machine_gpu = newVM->gpu_slowdown;
			newVM->storage = stod(machines[2][m]);
			newVM->usage_cost = stod(machines[3][m]);
			if(newVM->hasGpu){
				if(newVM->usage_cost > more_expensive_process_gpu)
					more_expensive_process_gpu = newVM->usage_cost;
			} else {
				if(newVM->usage_cost > more_expensive_process_cpu)
					more_expensive_process_cpu = newVM->usage_cost;
			}
			this->vms.push_back(newVM);
		}

		this->maxTime = ceil(total_time_job_cpu * slowest_machine_cpu);
		if(total_time_job_gpu * slowest_machine_gpu > this->maxTime) this->maxTime = ceil(total_time_job_gpu * slowest_machine_gpu);

		this->maxCost = ceil(total_time_job_cpu * more_expensive_process_cpu);
		if(total_time_job_gpu * more_expensive_process_gpu > this->maxCost) this->maxCost = ceil(total_time_job_gpu * more_expensive_process_gpu);


		// cout << "TotalTimeCpu: " << total_time_job_cpu << " TotalTimeGpu: " << total_time_job_gpu << endl;
		// cout << "SlowMachineCpu: " << slowest_machine_cpu << " SlowMachineGpu: " << slowest_machine_gpu << endl;
		// cout << "ExpensiveCpu: " << more_expensive_process_cpu << " ExpensiveGpu: " << more_expensive_process_gpu << endl;
		// cout << "MaxTime: " << this->maxTime << " MaxCost: " << this->maxCost << endl;
		// cin.get();


		
		vector<vector<string>> transfer;
		for(int i = 0; i < vms; i++){
			getline(in_file, line);
			// cout << "NewLine: " << line << endl;
			boost::split(tokens, line, boost::is_any_of(" "));
			transfer.push_back(tokens);
		}

		for(int i = 0; i < vms; i++){
			for(int t = 0; t < vms; t++){
				vector<double> double_line;
				for(int t2 = 0; t2 < vms; t2++){
					double_line.push_back(stod(transfer[t][t2]));
					// cout << stod(transfer[t][t2]) << " " ;
				}
				// cout << endl;
				this->vms[i]->bandwidth.push_back(double_line);
			}
		}

		// for(int t = 0; t < vms; t++){
		// 	// cout << "bandwidth vms: " << t << endl;
		// 	for(int i = 0; i < vms; i++){
		// 		for(int j = 0; j < vms; j++){
		// 			cout << this->vms[t]->bandwidth[i][j] << " ";
		// 		}
		// 		cout << endl;
		// 	}
		// }
		// cin.get();

		// esquerda antes da direita
		// cout << "jobs size: " << this->jobs.size() << endl;
		for(int i = 0; i < this->jobs.size(); i++){
			// cout << "i: " << i << endl;
			vector<int> line(this->jobs.size(), 0);
			for(int ti = 0; ti < this->jobs[i]->input.size(); ti++){
				// cout << "ti: " << ti << endl;
				if(this->jobs[i]->input[ti]->is_static)
					continue;
				int jobID = -1;
				for(int j = 0; j < this->jobs.size(); j++){
					// cout << "j: " << j << endl;
					if (i == j) continue;
					bool stop = false;
					for(int tj = 0; tj < this->jobs[j]->output.size(); tj++){
						// cout << "tj: " << tj << endl;
						// cout << "this->jobs[i]->input[ti]->id: " << this->jobs[i]->input[ti]->id << endl;
						// cout << "this->jobs[j]->output.size()': " << this->jobs[j]->output.size() << endl;
						// cout << "this->jobs[j]->output[tj]: " << this->jobs[j]->output[tj] << endl;
						// cout << "this->jobs[j]->name: " << this->jobs[j]->name << endl;
						// cout << "this->jobs[j]->output[tj]->id: " << this->jobs[j]->output[tj]->id << endl;
						
						if(this->jobs[j]->output[tj]->id == this->jobs[i]->input[ti]->id){
							// cout << "entrou if" << endl;
							stop = true;
							jobID = this->jobs[j]->id;
							break;
						}
					}
					if(stop) break;
				}
				// cout << "JOBID: " << jobID << endl;
				line[jobID] = 1;		
			}
			conflicts.push_back(line);
		}

		// cout << "Conflicts: " << endl;
		// for(int i = 0; i < this->jobs.size(); i++){
		// 	for(int j = 0; j < this->jobs.size(); j++){
		// 		cout << conflicts[i][j] << " ";
		// 	}
		// 	cout << endl;
		// }
		// cin.get();
		// cout << "Finished Reading!" << endl;
		// cin.get();
	}

	~Problem() { 
		for(int i = 0; i < this->jobs.size(); i++)
			delete jobs[i];
		for(int i = 0; i < this->vms.size(); i++)
			delete vms[i];
		for(int i = 0; i < this->alloc.size(); i++)
			delete alloc[i];
		for(int i = 0; i < this->files.size(); i++)
			delete files[i];
	}
};

#endif