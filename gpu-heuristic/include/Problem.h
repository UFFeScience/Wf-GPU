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

	double calculateWritetime(Job * job, int write_vm_id){
		double writetime = 0.0;
		if(write_vm_id != -1 && write_vm_id != this->id){
			for(unsigned int i = 0; i < job->output.size(); i++){
				// cout << "size: " << job->input[i]->size << endl;
				writetime += ceil(job->output[i]->size / this->bandwidth[this->id][write_vm_id]);
			}
		} else{
			writetime = 1.0 * job->output.size();
		}

		return writetime;
	}

	double calculateWritetimeWithChanges(Job * job, vector<int> write_vm_ids){
		
		double writetime = 0.0;
		for(unsigned int i = 0; i < job->output.size(); i++){
			int write_vm_id = write_vm_ids[job->output[i]->id];
			if(write_vm_id != -1 && write_vm_id != this->id){
				// cout << "size: " << job->input[i]->size << endl;
				writetime += ceil(job->output[i]->size / this->bandwidth[this->id][write_vm_id]);
			} else{
				writetime += 1.0;
			}
		} 

		return writetime;
	}

	double calculateReadtime(Job* job){
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
						currentReadTime = 1.0;
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
		return readtime;
	}

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
			// if(hasGpu) {
				// this->cost += ceil((timelineJobs[i]->base_time_gpu * this->gpu_slowdown)) * usage_cost;
				// cout << "Size: " << timelineFinishTime.size() << endl;
				if(timelineFinishTime.size() == 0)
					break;
				this->cost += (timelineFinishTime[i] - timelineStartTime[i] + 1) * usage_cost;
			// }
			// else {
			// 	this->cost += ceil((timelineJobs[i]->base_time_cpu * this->cpu_slowdown)) * usage_cost;
			// }
		}
		// }
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
	
	bool popJobKeepAllocation(int jobId){
		for(unsigned int i = 0; i < timelineJobs.size(); i++){
			if (timelineJobs[i]->id == jobId){
				timelineJobs[i]->alocated = false;
				// for(unsigned int f = 0; f < timelineJobs[i]->output.size(); f++){
				// 	timelineJobs[i]->output[f]->alocated_vm_id = -1;
				// }
				// if(timelineJobs[i]->rootJob){
				// 	for(unsigned int f = 0; f < timelineJobs[i]->input.size(); f++){
				// 		if(timelineJobs[i]->input[f]->is_static)
				// 			timelineJobs[i]->input[f]->alocated_vm_id = -1;
				// 	}	
				// }
				timelineJobs.erase(timelineJobs.begin() + i);
				timelineStartTime.erase(timelineStartTime.begin() + i);
				timelineFinishTime.erase(timelineFinishTime.begin() + i);
				return true;
			}
		}
		return false;
	}

	bool pushJobKeepAllocation(Job * job, int write_vm_id, double minSpam, vector<int> allocations){
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
		
		
		
		double readtime = calculateReadtime(job);

		// cout << "ReadTime: " << readtime << endl;
		// cin.get();

		
		double writetime = calculateWritetimeWithChanges(job, allocations);

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

		double finishTime = readtime + writetime + processtime + startTime - 1;

		// if(job->id == 0)
			// cout  << "JobId: " << job->id << " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;
		// if(job->name == "ID00002"){
		// 	cout << "ProcessMachineID: " <<this->id << " ProcessMAchineName: " << this->name << " writeID: " << write_vm_id << " minSpam: " << minSpam << endl;
		// 	cout << "start: " << startTime << " read: " << readtime << " process: " << processtime << " write: " << writetime << " fulltime: " << finishTime << endl;
		// 	cin.get();
		// }

		timelineStartTime.push_back(startTime);
		timelineFinishTime.push_back(finishTime);
		timelineJobs.push_back(job);

		job->alocated = true;
		job->alocated_vm_id = this->id;
		for(unsigned int i = 0; i < job->output.size(); i++){
			int final_vm = this->id;
			if(write_vm_id != -1)
				final_vm = write_vm_id;
			// job->output[i]->alocated_vm_id = final_vm;
		}

		for(unsigned int i = 0; i < job->input.size(); i++){
			if(job->input[i]->is_static){
				if(job->input[i]->alocated_vm_id < 0)
					job->input[i]->alocated_vm_id = this->id;
			}
		}

		return true;
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
		double readtime = calculateReadtime(job);


		double writetime = calculateWritetime(job, write_vm_id);
		// double writetime = 0.0;
		// if(write_vm_id != -1 && write_vm_id != this->id){
		// 	for(unsigned int i = 0; i < job->output.size(); i++){
		// 		// cout << "size: " << job->input[i]->size << endl;
		// 		writetime += ceil(job->output[i]->size / this->bandwidth[this->id][write_vm_id]);
		// 	}
		// } else{
		// 	writetime = 1.0 * job->output.size();
		// }

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

		// cout << "Readtime: " << readtime << " Writetime: " << writetime << " ProcessTime: " << processtime << " JobID: " << job->id << endl;

		double finishTime = readtime + writetime + processtime + startTime - 1;

		timelineStartTime.push_back(startTime);
		timelineFinishTime.push_back(finishTime);
		timelineJobs.push_back(job);

		// cout << "eita" << endl;
		job->alocated = true;
		job->alocated_vm_id = this->id;
		// cout << "job->output.size() " << job->output.size() << endl;
		for(unsigned int i = 0; i < job->output.size(); i++){
			// cout << "i: " << i << endl;
			if(job->output[i] == NULL) cin.get();
			if(job->output[i]->is_static) continue;
			int final_vm = this->id;
			if(write_vm_id != -1)
				final_vm = write_vm_id;
			// cout << "FinalWriteVM: " << final_vm << endl;
			if(final_vm < 0) {cin.get();}
			job->output[i]->alocated_vm_id = final_vm;
		}

		// cout << "eita2" << endl;
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

	double test_swapFileAllocation(){
		// print();
		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		vector<double> BestnewFinishTimes;
		vector<double> BestnewStartTimes;
		double originalCost = this->calculateFO();

		for(int f = 0; f < this->files.size(); f++){
			Item * file = files[f];
			// cout << "File: " << file->id << " is_Static: " << file->is_static << endl;
			// cin.get();
			if(file->is_static) continue;

			double bestMove = originalCost;
			int bestId = -1;
			for(unsigned int i = 0; i < this->vms.size(); i++){
				Machine * testVm = this->vms[i];
				// cout << "Tested VM: " << testVm->id << endl;
				// cin.get();
				if(testVm->id == file->alocated_vm_id) continue;
				
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);

				double newSpan = calculate_swapFileAllocation_Effect(file, testVm->id, newStartTimes, newFinishTimes) + 1;
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " machinePos: " << i << endl;
				// cin.get();
				if(bestMove - newSpan > 0.00001){
					// cout << "The new Span is: " << newSpan << " was: " << originalCost << endl;
					// cout << "File: " << file->id << " is_Static: " << file->is_static << " Tested VM: " << testVm->id << endl;
					// print();
					// cin.get();
					bestMove = newSpan;
					bestId = testVm->id;
					BestnewFinishTimes = newFinishTimes;
					BestnewStartTimes = newStartTimes;
				}
			}
			if(bestId >= 0){
				// cin.get();
				return this->execSwapFileAllocation(file, bestId, BestnewStartTimes, BestnewFinishTimes);
			}
		}
		// cout << "Nao achou melhora!" << endl;
		return -1.0;
	}
	double calculate_swapFileAllocation_Effect(Item * file, int writeTo, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		vector<int> newAlocations = vector<int>(files.size(), -1);
		vector<int> oldAlocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			oldAlocations[files[f]->id] = id;
			if(files[f]->id == file->id) id = writeTo;
			newAlocations[files[f]->id] = id;
		}
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double latestJobConflictFinish = 0.0;
		double latestJobVmFinish = 0.0;

		for(int a = 0; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			latestJobConflictFinish = 0.0;
			latestJobVmFinish = 0.0;
			int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);
			// double oldWriteTime = calculateWritetimeWithChanges(alloc[a]->job, alloc[a]->vms->id, oldAlocations);
			// double oldReadTime = calculateReadtime(alloc[a]->job, alloc[a]->vms->id);
			// double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm] - oldReadTime - oldWriteTime;
			double execTime;
			if(alloc[a]->vms->hasGpu && alloc[a]->job->gpu){
				execTime = ceil(alloc[a]->job->base_time_gpu * alloc[a]->vms->gpu_slowdown);
				alloc[a]->job->on_gpu = true;
			} else if(!alloc[a]->vms->hasGpu || !alloc[a]->job->gpu){
				execTime = ceil(alloc[a]->job->base_time_cpu * alloc[a]->vms->cpu_slowdown);
				alloc[a]->job->on_gpu = false;
			}
			// double execTime = ceil(alloc[a]->vms->slowdown * alloc[a]->job->base_time);
			// cout << "********* calculated execTime: " << execTime << endl;
			// cin.get();


			double readtime = 0.0; // calculando o tempo de leitura de todos os arquivos de input necessarios caso nao estejam alocados na Maquina
			for(unsigned int i = 0; i < alloc[a]->job->input.size(); i++){
				if(alloc[a]->job->input[i]->alocated_vm_id == alloc[a]->vms->id){
					readtime += 1.0;
					continue;
				}
				int source;
				if(alloc[a]->job->input[i]->is_static){
					double readTime = INFINITY;
					for(int j = 0; j < alloc[a]->job->input[i]->static_vms.size(); j++){
						double currentReadTime;
						if(alloc[a]->job->input[i]->static_vms[j] == alloc[a]->vms->id){
							currentReadTime = 1.0;
						} else{
							currentReadTime = ceil(alloc[a]->job->input[i]->size / alloc[a]->vms->bandwidth[alloc[a]->job->input[i]->static_vms[j]][alloc[a]->vms->id]);
						}
						if(currentReadTime < readTime){
							readTime = currentReadTime;
						}
					}
					readtime += readTime;
				} else{
					double readTime;
					Item * auxFile = alloc[a]->job->input[i];
					int origin = auxFile->alocated_vm_id;
					
					if(file->id == auxFile->id) origin = writeTo;

					if(origin == alloc[a]->vms->id){
						readtime += 1;
						continue;
					}

					if(alloc[a]->vms->bandwidth[origin][alloc[a]->vms->id] == 0.0){
						readTime = 0.0;
					} else
						readTime = ceil(alloc[a]->job->input[i]->size / alloc[a]->vms->bandwidth[origin][alloc[a]->vms->id]);
					readtime += readTime;	
				}
			}

			// cout << "calculated readtime: " << readtime << endl;
			// cin.get();

			double writeTime = alloc[a]->vms->calculateWritetimeWithChanges(alloc[a]->job, newAlocations);

			// cout << "calculated writeTime: " << writeTime << endl;
			// cin.get();

			// cout << "execTime: " << execTime + calculateReadtime(alloc[a]->job, alloc[a]->vms->id) << " oldRead: " << calculateReadtime(alloc[a]->job, alloc[a]->vms->id);			

			execTime += readtime + writeTime;

			// cout << "FileID: " << file->id << " newWriteTo: " << writeTo << " newRead: " << readtime << " newExec: " << execTime << endl;


			for(int b = 0; b < a; b++){
				int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
				int bVmId = alloc[b]->vms->id;
				if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
					if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[b]->job->id] + 1;
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[b]->job->id] + 1;
				}
			}
			newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
		}


		// cout << "times after recalculation" << endl;
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes.size(); i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		// cout << "Biggest span: " << biggestSpan << endl;

		double cost = 0.0;
		for(int i = 0; i < vms.size(); i++){
			for(int j = 0; j < vms[i]->timelineJobs.size(); j++){
				// if(hasGpu) {
					// this->cost += ceil((timelineJobs[i]->base_time_gpu * this->gpu_slowdown)) * usage_cost;
					// cout << "Size: " << timelineFinishTime.size() << endl;
					cost += (newFinishTimes[j] - newStartTimes[j] + 1) * vms[i]->usage_cost;
			}
		}
		// cout << "FO of move: " << this->ponderation*(biggestSpan / this->maxTime) + (1.0 - this->ponderation)*(cost / this->maxCost) << endl;
		return this->ponderation*(biggestSpan / this->maxTime) + (1.0 - this->ponderation)*(cost / this->maxCost);
	}

	double execSwapFileAllocation(Item * file, int writeTo, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		file->alocated_vm_id = writeTo;

		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
			vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];
		}
		// cout << "^^^^" << endl;
		// this->print();
		// cin.get();

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}

		return this->calculateFO();
	}
	

	double test_swapMachinePair(){
		vector<double> newFinishTimes;
		vector<double> newStartTimes;

		double originalCost = this->calculateFO();

		vector<int> allocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			allocations[files[f]->id] = id;
		}

		for(int pos = 0; pos < this->alloc.size(); pos++){ // SWAP MACHINE LOOP START
			Allocation * swap = this->alloc[pos];
			Job * job = swap->job;
			Machine * originalAllocationMachine = swap->vms;

			int posOnTimeline = originalAllocationMachine->jobPosOnTimeline(job->id);
			double oldExecTime = originalAllocationMachine->timelineFinishTime[posOnTimeline] - originalAllocationMachine->timelineStartTime[posOnTimeline];

			int write_vm_id = swap->writeTo;

			for(int pos2 = pos + 1; pos2 < this->alloc.size(); pos2++){
				
				Allocation * swap2 = this->alloc[pos2];
				Job * job2 = swap2->job;
				Machine * originalAllocationMachine2 = swap2->vms;

				if(originalAllocationMachine2->id == originalAllocationMachine->id) continue;
				// cout << "JobID: " << job->id << " Job2ID: " << job2->id << endl;

				int posOnTimeline2 = originalAllocationMachine2->jobPosOnTimeline(job2->id);
				double oldExecTime2 = originalAllocationMachine2->timelineFinishTime[posOnTimeline2] - originalAllocationMachine2->timelineStartTime[posOnTimeline2];

				int write_vm_id2 = swap2->writeTo;

				// times for pos job
				Machine * testVm = swap2->vms;
				double readtime = testVm->calculateReadtime(job);

				double writetime = testVm->calculateWritetimeWithChanges(job, allocations);
				// double writetime = calculateWritetime(job, testVm->id, write_vm_id);

				double processtime;
				if(testVm->hasGpu && job->gpu){
					processtime = ceil(job->base_time_gpu * testVm->gpu_slowdown);
					job->on_gpu = true;
				} else if(!testVm->hasGpu || !job->gpu){
					processtime = ceil(job->base_time_cpu * testVm->cpu_slowdown);
					job->on_gpu = false;
				}
				double newTime = readtime + processtime + writetime;

				// times for pos2 job
				testVm = swap->vms;
				readtime = testVm->calculateReadtime(job2);

				writetime = testVm->calculateWritetimeWithChanges(job2, allocations);

				// writetime = calculateWritetime(job, testVm->id, write_vm_id2);

				processtime = 0.0;

				if(testVm->hasGpu && job2->gpu){
					processtime = ceil(job2->base_time_gpu * testVm->gpu_slowdown);
					job2->on_gpu = true;
				} else if(!testVm->hasGpu || !job->gpu){
					processtime = ceil(job2->base_time_cpu * testVm->cpu_slowdown);
					job2->on_gpu = false;
				}

				double newTime2 = readtime + processtime + writetime;


				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);
				double newSpan = calculate_swapMachinePair_effect(pos, pos2, job, job2, newTime, newTime2, newStartTimes, newFinishTimes);
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;
// 123
				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << endl;
				
				// cin.get();
				if(originalCost - newSpan > 0.00001){
					// for(int i = 0; i < newStartTimes.size(); i++){
					// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
					// }
					// cout << "originalCost: " << originalCost << endl;
					// cout << "The new Span is: " << newSpan << endl;
					
					// cin.get();
					// this->print();
					// this->printAlloc();
					// cout << "#################" << endl;
					// cout << "AllocPos: " << pos << " AllocPos2: " << pos2 << endl;
					// cout << "JobID: " << job->id << " Job2ID: " << job2->id << endl;
					// cout << "JOB1VM " << job->alocated_vm_id << " JOB2VM " << job2->alocated_vm_id << endl;
					// cout << "JobID: " << alloc[pos]->job->id << " Job2ID: " << alloc[pos2]->job->id << endl;
					// cout << "JOB1VM " << alloc[pos]->vms->id << " JOB2VM " << alloc[pos2]->vms->id << endl;
					// for(int i = 0; i < newStartTimes.size(); i++){
					// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
					// }
					// cin.get();
					double aux = this->execSwapMachinePair(pos, pos2, newStartTimes, newFinishTimes);
					// cout << "Saida do machinePair aux:" << aux << endl;
					// this->print();
					// cin.get();
					return aux;
					// if(job->id == 8 && job2->id == 13 || job2->id == 8 && job->id == 13) {
					// 	this->print();
					// 	this->printAlloc();
					// 	cin.get();
					// }
				}
				
			}
		}
		return -1;
	}

	void fixMachineTimelineOrder(int vmId){
		Machine * vm = vms[vmId];
		int moves = 9999;
		bool moved = true;
		// cout << "Before fix!" << endl;
		// this->print();
		// cout << "$$$$$$$$$$$$$4" << endl;
		while(moved){
			if(moves < 0){
				cout << "LOOP" << endl;
				this->print();
				this->printAlloc();
				cin.get();
			}
			moved = false;
			for(int j = 1; j < vm->timelineJobs.size(); j++){
				if(vm->timelineStartTime[j] < vm->timelineFinishTime[j - 1]){ // ordem errada na timeline!
					moves--;
					moved = true;
					Job * job = vm->timelineJobs[j];
					double startTime = vm->timelineStartTime[j];
					double finishTime = vm->timelineFinishTime[j];
					
					// cout << "Ordem Errada!" << endl;

					vm->timelineJobs.erase(vm->timelineJobs.begin() + j);
					vm->timelineStartTime.erase(vm->timelineStartTime.begin() + j);
					vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + j);
					bool found = false;
					for(int k = 0; k < vm->timelineJobs.size(); k++){
						if(vm->timelineStartTime[k] >= finishTime){
							found = true;
							vm->timelineJobs.insert(vm->timelineJobs.begin() + k, job);
							vm->timelineStartTime.insert(vm->timelineStartTime.begin() + k, startTime);
							vm->timelineFinishTime.insert(vm->timelineFinishTime.begin() + k, finishTime);
							break;
						}
					}
					if (!found){
						vm->timelineJobs.push_back(job);
						vm->timelineStartTime.push_back(startTime);
						vm->timelineFinishTime.push_back(finishTime);
					}
					break;
				}
			}
			if(moved){
				// this->print();
				// cin.get();
			}
		}
		// cout << "After fix!" << endl;
		// this->print();
	}

	double execSwapMachinePair(int pos, int pos2, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		
		// cout << "execSwapMachinePair" << endl;
		// this->printAlloc();
		// cout << "**********" << endl;

		Job * changedJob = this->alloc[pos]->job;
		Machine * changedVm = this->alloc[pos2]->vms;
		Job * changedJob2 = this->alloc[pos2]->job;
		Machine * changedVm2 = this->alloc[pos]->vms;

		// cout << "JOB1VM: " << changedVm2->id << " JOB2VM: " << changedVm->id << endl;
		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			if(job->id == changedJob->id){
				// cout << "Is Changed Job!" << endl;
				job->alocated_vm_id = changedVm->id;
				vm->timelineJobs.erase(vm->timelineJobs.begin() + posOnTimeline);
				vm->timelineStartTime.erase(vm->timelineStartTime.begin() + posOnTimeline);
				vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + posOnTimeline);
				// cout << "Erased" << endl;

				// this->print();
				// cin.get();

				bool inserted = false;

				// cout << "Changed VM ID: " << changedVm->id << endl;
				for(int j = 0; j < changedVm->timelineJobs.size(); j++){
					// cout << "j: " << j << endl;
					int jId = changedVm->timelineJobs[j]->id;
					if(newStartTimes[jId] >= newFinishTimes[job->id]){
						// cout << "Found possition: " << j << endl;
						changedVm->timelineJobs.insert(changedVm->timelineJobs.begin() + j, job);
						changedVm->timelineStartTime.insert(changedVm->timelineStartTime.begin() + j, newStartTimes[job->id]);
						changedVm->timelineFinishTime.insert(changedVm->timelineFinishTime.begin() + j, newFinishTimes[job->id]);
						inserted=true;
						break;
					}
				}
				if(!inserted){
					changedVm->timelineJobs.push_back(job);
					changedVm->timelineStartTime.push_back(newStartTimes[job->id]);
					changedVm->timelineFinishTime.push_back(newFinishTimes[job->id]);
				}
			

			} else if(job->id == changedJob2->id){
				job->alocated_vm_id = changedVm2->id;
				vm->timelineJobs.erase(vm->timelineJobs.begin() + posOnTimeline);
				vm->timelineStartTime.erase(vm->timelineStartTime.begin() + posOnTimeline);
				vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + posOnTimeline);
				// cout << "Erased" << endl;

				// this->print();
				// cin.get();

				bool inserted = false;

				// cout << "Changed VM ID: " << changedVm->id << endl;
				for(int j = 0; j < changedVm2->timelineJobs.size(); j++){
					// cout << "j: " << j << endl;
					int jId = changedVm2->timelineJobs[j]->id;
					if(newStartTimes[jId] >= newFinishTimes[job->id]){
						// cout << "Found possition: " << j << endl;
						changedVm2->timelineJobs.insert(changedVm2->timelineJobs.begin() + j, job);
						changedVm2->timelineStartTime.insert(changedVm2->timelineStartTime.begin() + j, newStartTimes[job->id]);
						changedVm2->timelineFinishTime.insert(changedVm2->timelineFinishTime.begin() + j, newFinishTimes[job->id]);
						inserted=true;
						break;
					}
				}
				if(!inserted){
					changedVm2->timelineJobs.push_back(job);
					changedVm2->timelineStartTime.push_back(newStartTimes[job->id]);
					changedVm2->timelineFinishTime.push_back(newFinishTimes[job->id]);
				}
			
			} else{
				// cout << "Regular Job" << endl;
				vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
				vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];
			}
		}

		this->alloc[pos]->vms = vms[changedJob->alocated_vm_id];
		this->alloc[pos2]->vms = vms[changedJob2->alocated_vm_id];
		// cout << "Job1: " << changedJob->name << " Job2: " << changedJob2->name << endl;
		// this->print();

		// cin.get();

		// fixing order on each VM
		// cout << "Fixing order on VMs" << endl;
		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}

		// this->calculateMakespam();
		// this->print();

		// // this->checkFeasible();
		// cin.get();

		// this->printAlloc();
		// cout << "**********" << endl;

		return this->calculateFO();
	}

	double calculate_swapMachinePair_effect(int allocPos, int allocPos2, Job* job, Job* job2, double newTime, double newTime2, vector<double> &newStartTimes, vector<double> &newFinishTimes){
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		int vmId = alloc[allocPos2]->vms->id;
		int vmId2 = alloc[allocPos]->vms->id;

		// procurar latest job na vmId e latest job que tenha conflito 
		double latestJobVmFinish =  0.0;
		double latestJobConflictFinish = 0.0;
		for(int a = 0; a < allocPos; a++){
			if(conflicts[job->id][alloc[a]->job->id] == 0){ // nao tem conflito
				if(alloc[a]->vms->id == vmId){ // esta na mesma maquina
					if(newFinishTimes[alloc[a]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
						latestJobVmFinish = newFinishTimes[alloc[a]->job->id] + 1;
				}
			} else{ // tem conflito
				if(newFinishTimes[alloc[a]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
					latestJobConflictFinish = newFinishTimes[alloc[a]->job->id] + 1;
			}
		}
		newStartTimes[alloc[allocPos]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
		newFinishTimes[alloc[allocPos]->job->id] = newStartTimes[alloc[allocPos]->job->id] + newTime; // tempo de fim

		// printAlloc();

		// cout << "######################" << endl;
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }


		for(int a = allocPos + 1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			if(a == allocPos2){
				latestJobVmFinish =  0.0;
				latestJobConflictFinish = 0.0;
				for(int b = 0; b < allocPos2; b++){
					if(conflicts[job2->id][alloc[b]->job->id] == 0){ // nao tem conflito
						int id = alloc[b]->vms->id;
						if (a == allocPos) id = vmId;
						if(id == vmId2){ // esta na mesma maquina
							// cout << "Mesma Maquina! ID: " << alloc[a]->job->id << endl;
							if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
								latestJobVmFinish = newFinishTimes[alloc[b]->job->id] + 1;
						}
					} else{ // tem conflito
						if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
							latestJobConflictFinish = newFinishTimes[alloc[b]->job->id] + 1;
					}
				}
				newStartTimes[alloc[allocPos2]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
				newFinishTimes[alloc[allocPos2]->job->id] = newStartTimes[alloc[allocPos2]->job->id] + newTime2; // tempo de fim
			}
			else{
				latestJobConflictFinish = 0.0;
				latestJobVmFinish = 0.0;
				int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);
				
				double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm];
				for(int b = 0; b < a; b++){
					int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
					int bVmId = alloc[b]->vms->id;
					if(b == allocPos){ // eh o modificado
						bVmId = vmId;
					} else if(b == allocPos2){
						bVmId = vmId2;
					}
					if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
						if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
							if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
								latestJobVmFinish = newFinishTimes[alloc[b]->job->id] + 1;
						}
					} else { // tem conflito
						if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
							latestJobConflictFinish = newFinishTimes[alloc[b]->job->id] + 1;
					}
				}
				newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
				newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
			}
		}


		// cout << "**************************" << endl;
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes.size(); i++){
			if(newFinishTimes[i] + 1 > biggestSpan)
				biggestSpan = newFinishTimes[i] + 1;
		}

		// cout << "Biggest span: " << biggestSpan << endl;

		double cost = 0.0;
		for(int i = 0; i < jobs.size(); i++){
			int j = jobs[i]->id;
			int timeUsed = newFinishTimes[j] - newStartTimes[j] + 1;
			int machineId = jobs[i]->alocated_vm_id;
			if (jobs[i]->id == job->id){
				machineId = vmId;
			}
			if (jobs[i]->id == job2->id){
				machineId = vmId2;
			}

			cost += timeUsed * vms[machineId]->usage_cost;
		}
		// cout << "Move cost: " << cost << endl;
		// cout << "FO of move: " << this->ponderation*(biggestSpan / this->maxTime) + (1.0 - this->ponderation)*(cost / this->maxCost) << endl;
		return this->ponderation*(biggestSpan / this->maxTime) + (1.0 - this->ponderation)*(cost / this->maxCost);
	}

	double calculate_swapMachine_effect(int allocPos, Job* job, double newTime, int vmId, vector<double> &newStartTimes, vector<double> &newFinishTimes){
		// print();
		// cout << "newTime: " << newTime  << " vmId: " << vmId << endl;
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		// procurar latest job na vmId e latest job que tenha conflito 
		double latestJobVmFinish =  0.0;
		double latestJobConflictFinish = 0.0;
		for(int a = 0; a < allocPos; a++){
			if(conflicts[job->id][alloc[a]->job->id] == 0){ // nao tem conflito
				if(alloc[a]->vms->id == vmId){ // esta na mesma maquina
					if(newFinishTimes[alloc[a]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
						latestJobVmFinish = newFinishTimes[alloc[a]->job->id] + 1;
				}
			} else{ // tem conflito
				if(newFinishTimes[alloc[a]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
					latestJobConflictFinish = newFinishTimes[alloc[a]->job->id] + 1;
			}
		}
		newStartTimes[alloc[allocPos]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
		newFinishTimes[alloc[allocPos]->job->id] = newStartTimes[alloc[allocPos]->job->id] + newTime; // tempo de fim


		for(int a = allocPos + 1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			latestJobConflictFinish = 0.0;
			latestJobVmFinish = 0.0;
			int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);

			double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm];
			for(int b = 0; b < a; b++){
				int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
				int bVmId = alloc[b]->vms->id;
				if(b == allocPos){ // eh o modificado
					bVmId = vmId;
				} 
				if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
					if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[b]->job->id] + 1;
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[b]->job->id] + 1;
				}
			}
			newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
		}



		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes.size(); i++){
			if(newFinishTimes[i] + 1 > biggestSpan)
				biggestSpan = newFinishTimes[i] + 1;
		}

		// cout << "Biggest span: " << biggestSpan << endl;

		double cost = 0.0;
		for(int i = 0; i < jobs.size(); i++){
			int j = jobs[i]->id;
			int timeUsed = newFinishTimes[j] - newStartTimes[j] + 1;
			int machineId = jobs[i]->alocated_vm_id;
			if (jobs[i]->id == job->id){
				machineId = vmId;
			}
			cost += timeUsed * vms[machineId]->usage_cost;
		}
		return this->ponderation*(biggestSpan / this->maxTime) + (1.0 - this->ponderation)*(cost / this->maxCost);
	}

	double test_swapMachine(){

		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		double originalCost = this->calculateFO();

		vector<int> allocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			allocations[files[f]->id] = id;
		}
		for(int pos = 0; pos < this->alloc.size(); pos++){ // SWAP MACHINE LOOP START
		
			Allocation * swap = this->alloc[pos];
			Job * job = swap->job;
			Machine * originalAllocationMachine = swap->vms;

			// cout << "JobID: " << job->id << endl;

			// int posOnTimeline = originalAllocationMachine->jobPosOnTimeline(job->id);
			// double oldExecTime = originalAllocationMachine->timelineFinishTime[posOnTimeline] - originalAllocationMachine->timelineStartTime[posOnTimeline];

			// cout << "WRITE_TO: " << write_vm_id << endl;

			for(unsigned int i = 0; i < this->vms.size(); i++){
				Machine * testVm = this->vms[i];
				if(testVm->id == originalAllocationMachine->id) continue;
				double readtime = testVm->calculateReadtime(job);

				double writetime = testVm->calculateWritetimeWithChanges(job, allocations);
				// double writetime = calculateWritetime(job, testVm->id, alloc[pos]->writeTo);

				double processtime;
				if(testVm->hasGpu && job->gpu){
					processtime = ceil(job->base_time_gpu * testVm->gpu_slowdown);
					job->on_gpu = true;
				} else if(!testVm->hasGpu || !job->gpu){
					processtime = ceil(job->base_time_cpu * testVm->cpu_slowdown);
					job->on_gpu = false;
				}

				double newTime = readtime + processtime + writetime;

				
				
				// cout << "Calculating cascade effect" << endl;
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);
				double newSpan = calculate_swapMachine_effect(pos, job, newTime, testVm->id, newStartTimes, newFinishTimes);
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " oldExecTime: " << oldExecTime << " newTime: " << newTime << " machinePos: " << i << endl;
				// cin.get();
				if(originalCost - newSpan > 0.00001){
					// cout << "The new Span is: " << originalCost - oldExecTime + newTime << endl;
					
					// cin.get();
					double aux = this->execSwapMachine(pos, i, newStartTimes, newFinishTimes);

					// cout << "SimulatedCost: " << newSpan << " ActualCost: " << aux << endl;

					return aux;
				}
			}
		}
		return -1;

	}

	double execSwapMachine(int pos, int vmId, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		Job * changedJob = this->alloc[pos]->job;
		Machine * changedVm = this->vms[vmId];

		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			if(job->id == changedJob->id){
				// cout << "Is Changed Job!" << endl;
				job->alocated_vm_id = vmId;
				vm->timelineJobs.erase(vm->timelineJobs.begin() + posOnTimeline);
				vm->timelineStartTime.erase(vm->timelineStartTime.begin() + posOnTimeline);
				vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + posOnTimeline);
				// cout << "Erased" << endl;

				// this->print();
				// cin.get();

				bool inserted = false;

				// cout << "Changed VM ID: " << changedVm->id << endl;
				for(int j = 0; j < changedVm->timelineJobs.size(); j++){
					// cout << "j: " << j << endl;
					int jId = changedVm->timelineJobs[j]->id;
					if(newStartTimes[jId] >= newFinishTimes[job->id]){
						// cout << "Found possition: " << j << endl;
						changedVm->timelineJobs.insert(changedVm->timelineJobs.begin() + j, job);
						changedVm->timelineStartTime.insert(changedVm->timelineStartTime.begin() + j, newStartTimes[job->id]);
						changedVm->timelineFinishTime.insert(changedVm->timelineFinishTime.begin() + j, newFinishTimes[job->id]);
						inserted=true;
						break;
					}
				}
				if(!inserted){
					changedVm->timelineJobs.push_back(job);
					changedVm->timelineStartTime.push_back(newStartTimes[job->id]);
					changedVm->timelineFinishTime.push_back(newFinishTimes[job->id]);
				}

			} else{
				// cout << "Regular Job" << endl;
				vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
				vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];
			}
		}

		this->alloc[pos]->vms = vms[changedJob->alocated_vm_id];

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}
		// this->calculateMakespam();
		// this->print();

		// this->checkFeasible();
		// cin.get();

		return this->calculateFO();
	}


	// double calculate_swapMachineWrite_effect(int allocPos, Job* job, double newTime, vector<int>& output, int vmId, vector<double> &newStartTimes, vector<double> &newFinishTimes){
	// 	// print();
	// 	// cout << "newTime: " << newTime  << " vmId: " << vmId << endl;
		
	// 	for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
	// 		int jobId = alloc[a]->job->id;
	// 		Machine * vm = alloc[a]->vms;
	// 		int posOnVm = vm->jobPosOnTimeline(jobId);
	// 		newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
	// 		newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
	// 	}

	// 	// for(int i = 0; i < newStartTimes.size(); i++){
	// 	// 	if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
	// 	// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
	// 	// }
	// 	// cin.get();

	// 	newFinishTimes[alloc[allocPos]->job->id] = newStartTimes[alloc[allocPos]->job->id] + newTime; // tempo de fim

	// 	double latestJobConflictFinish = 0.0;
	// 	double latestJobVmFinish = 0.0;

	// 	for(int a = allocPos + 1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
	// 		latestJobConflictFinish = 0.0;
	// 		latestJobVmFinish = 0.0;
	// 		int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);

	// 		double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm] - calculateReadtime(alloc[a]->job, alloc[a]->vms->id);
	// 		// double writeTime = calculateWritetime(alloc[a]->job, alloc[a]->vms->id, alloc[a]->writeTo);
	// 		// double processTime = ceil(alloc[a]->job->base_time * alloc[a]->vms->slowdown);
			
	// 		double readTime = 0.0;

	// 		for(int f = 0; f < alloc[a]->job->input.size(); f++){
	// 			int minBandwidthVm = alloc[a]->vms->id;
	// 			if(alloc[a]->job->input[f]->is_static){
	// 				bool transferNeed = true;
	// 				double maxBandwidth = 0.0;
	// 				int id;
	// 				for(int j = 0; j < alloc[a]->job->input[f]->static_vms.size(); j++){
	// 					if(alloc[a]->vms->id == alloc[a]->job->input[f]->static_vms[j]){
	// 						transferNeed = false;
	// 						break;
	// 					}
	// 					if(alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->job->input[f]->static_vms[j]] > maxBandwidth){
	// 						maxBandwidth = alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->job->input[f]->static_vms[j]];
	// 						id = alloc[a]->job->input[f]->static_vms[j];
	// 					}
	// 				}

	// 				if(maxBandwidth < alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->vms->id])
	// 					minBandwidthVm = id;
	// 				if(transferNeed) readTime += alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm];
	// 			} else{
	// 				// cout << "Not Static!" << endl;
	// 				Item * file = alloc[a]->job->input[f];
	// 				int origin =  file->alocated_vm_id;
	// 				for(int i = 0; i < output.size(); i++){
	// 					if(file->id == output[i]){
	// 						// cout << "Changed VM!" << endl;
	// 						origin = vmId;						
	// 						break;			
	// 					}
	// 				}				
	// 				if(origin == alloc[a]->vms->id) continue;

	// 				if(alloc[a]->job->input[f]->VMsBandwidth[origin] < alloc[a]->job->input[f]->VMsBandwidth[minBandwidthVm])
	// 					minBandwidthVm = origin;
	// 				// cout << "ReadTime += " <<  alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm] << endl;
	// 				readTime += alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm];
	// 			}
	// 		}
	// 		// cout << "execTime: " << execTime + calculateReadtime(alloc[a]->job, alloc[a]->vms->id) << " oldRead: " << calculateReadtime(alloc[a]->job, alloc[a]->vms->id);			

	// 		execTime += readTime;

	// 		// cout << " newRead: " << readTime << " newExec: " << execTime << endl;

	// 		for(int b = 0; b < a; b++){
	// 			int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
	// 			int bVmId = alloc[b]->vms->id;
	// 			if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
	// 				if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
	// 					if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
	// 						latestJobVmFinish = newFinishTimes[alloc[b]->job->id];
	// 				}
	// 			} else { // tem conflito
	// 				if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
	// 					latestJobConflictFinish = newFinishTimes[alloc[b]->job->id];
	// 			}
	// 		}
	// 		newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
	// 		newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
	// 	}



	// 	// for(int i = 0; i < newStartTimes.size(); i++){
	// 	// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
	// 	// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
	// 	// }
	// 	// cin.get();

	// 	double biggestSpan = 0.0;
	// 	for(int i = 0; i < newFinishTimes[i]; i++){
	// 		if(newFinishTimes[i] > biggestSpan)
	// 			biggestSpan = newFinishTimes[i];
	// 	}

	// 	return biggestSpan;

	// }

	// double test_swapMachineWrite(){
	// 	vector<double> newFinishTimes;
	// 	vector<double> newStartTimes;
	// 	for(int pos = 0; pos < this->alloc.size(); pos++){

	// 		Allocation * swap = this->alloc[pos];
	// 		Job * job = swap->job;
	// 		Machine * originalAllocationMachine = swap->vms;
	// 		double originalCost = this->calculateMakespam();

	// 		// cout << "JobID: " << job->name << endl;

	// 		int posOnTimeline = originalAllocationMachine->jobPosOnTimeline(job->id);
	// 		double oldExecTime = originalAllocationMachine->timelineFinishTime[posOnTimeline] - originalAllocationMachine->timelineStartTime[posOnTimeline];
	// 		// cout << "WRITE_TO: " << write_vm_id << endl;

	// 		vector<int> outputs;
	// 		for(int i = 0; i < job->output.size(); i++){
	// 			outputs.push_back(job->output[i]->id);
	// 		}

	// 		for(unsigned int i = 0; i < this->vms.size(); i++){
	// 			Machine * testVm = this->vms[i];
	// 			if(testVm->id == swap->writeTo) continue;
	// 			double readtime = calculateReadtime(job, originalAllocationMachine->id);

	// 			double writetime = calculateWritetime(job, originalAllocationMachine->id, testVm->id);

	// 			double processtime = ceil(job->base_time * originalAllocationMachine->slowdown);

	// 			double newTime = readtime + processtime + writetime;

				
				
	// 			// cout << "Calculating cascade effect" << endl;
	// 			newFinishTimes = vector<double>(this->jobs.size(), 0.0);
	// 			newStartTimes = vector<double>(this->jobs.size(), 0.0);
	// 			double newSpan = calculate_swapMachineWrite_effect(pos, job, newTime, outputs, testVm->id, newStartTimes, newFinishTimes);
	// 			// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

	// 			// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " oldExecTime: " << oldExecTime << " newTime: " << newTime << " machinePos: " << i << endl;
	// 			// cin.get();
	// 			if(newSpan < originalCost){
	// 				// cout << "The new Span is: " << newSpan << endl;
					
	// 				// cin.get();
	// 				 return this->execSwapMachineWrite(pos, i, newStartTimes, newFinishTimes);
	// 			}
	// 		}
	// 	}
	// 	return -1.0;
	// }

	// double execSwapMachineWrite(int pos, int vmId, vector<double>& newStartTimes, vector<double>& newFinishTimes){
	// 	Job * changedJob = this->alloc[pos]->job;
	// 	Machine * changedVm = this->vms[vmId];

	// 	for(int a = 0; a < this->alloc.size(); a++){
	// 		Job * job = this->alloc[a]->job;
	// 		Machine * vm = this->alloc[a]->vms;
	// 		int posOnTimeline = vm->jobPosOnTimeline(job->id);

	// 		if(job->id == changedJob->id){
	// 			// cout << "Is Changed Job!" << endl;
	// 			for(int f = 0; f < job->output.size(); f++){
	// 				job->output[f]->alocated_vm_id = vmId;
	// 			}
	// 		} 
	// 		vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
	// 		vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];			
	// 	}

	// 	this->alloc[pos]->writeTo = vmId;

	// 	for(int vm = 0; vm < vms.size(); vm++){
	// 		this->fixMachineTimelineOrder(vm);
	// 	}
	// 	// this->calculateMakespam();
	// 	// this->print();

	// 	// this->checkFeasible();
	// 	// cin.get();

	// 	return this->calculateMakespam();
	// }


	bool test_reallocate_valid(int pos, int newPos){
		Job * job = alloc[pos]->job;

		//esquerda filho do da direita
		if(newPos > pos){
			for(int i = 0; i < newPos; i++){
				
				Job * aux = alloc[i]->job;
				if (i == pos) aux = alloc[newPos]->job;

				if(conflicts[aux->id][job->id] == 1){
					return false;
				}
			}
		} else{
			for(int i = newPos + 1; i <= pos; i++){				
				Job * aux = alloc[i]->job;

				if(conflicts[job->id][aux->id] == 1){
					return false;
				}
			}
		}

		return true;
	}

	double exec_reallocate(int pos1, int pos2, vector<double> newStartTimes, vector<double> newFinishTimes){
		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);
			vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
			vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];			
		}
		Allocation * aux = alloc[pos1];
		alloc[pos1] = alloc[pos2];
		alloc[pos2] = aux;

		this->calculateMakespam();
		// this->print();
		// this->printAlloc();
		// cin.get();

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}
		

		// this->checkFeasible();
		// cin.get();

		return this->calculateFO();
	}

	double test_reallocate(){
		// this->print();
		// this->printAlloc();
		// cin.get();

		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		double originalCost = this->calculateFO();
		for(int pos1 = 0; pos1 < alloc.size() - 1; pos1++){
			for(int pos2 = pos1 + 1; pos2 < alloc.size(); pos2++){
				
				if(!test_reallocate_valid(pos1, pos2) || !test_reallocate_valid(pos2, pos1)) {
					// cout << "Pos1: " << pos1 << " pos2: " << pos2 << " Nao podem ser trocados!" << endl;
					continue;
				}

				// cout << "Pos1: " << pos1 << " pos2: " << pos2 << " PODEM ser trocados!" << endl;
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);

				double newSpan = calculate_reallocate_effect(pos1, pos2, newStartTimes, newFinishTimes);
				if(originalCost - newSpan > 0.00001){
					// cout << "newSpan: " << newSpan << " oldSpam: " << originalCost << endl;
					// cin.get();
					// cout << "Pos1: " << pos1 << " pos2: " << pos2 <<  endl;
					// return newSpan;
					return exec_reallocate(pos1, pos2, newStartTimes, newFinishTimes);
				}
			}
		}

		return -1.0;
	}

	double calculate_reallocate_effect(int pos1, int pos2, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		//  for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		for(int a = pos1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
		
			double latestJobConflictFinish = 0.0;
			double latestJobVmFinish = 0.0;
			int usedPos = a;
			if(a == pos1) usedPos = pos2;
			if(a == pos2) usedPos = pos1;

			int aPosOnVm = alloc[usedPos]->vms->jobPosOnTimeline(alloc[usedPos]->job->id);

			double execTime = alloc[usedPos]->vms->timelineFinishTime[aPosOnVm] - alloc[usedPos]->vms->timelineStartTime[aPosOnVm];
			for(int b = 0; b < a; b++){

				int usedPos2 = b;
				if(b == pos1) usedPos2 = pos2;
				if(b == pos2) usedPos2 = pos1;

				int bPosOnVm = alloc[usedPos2]->vms->jobPosOnTimeline(alloc[usedPos2]->job->id);
				int bVmId = alloc[usedPos2]->vms->id;
				
				if(conflicts[alloc[usedPos]->job->id][alloc[usedPos2]->job->id] == 0){ // nao tem conflito
					if(alloc[usedPos]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[usedPos2]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[usedPos2]->job->id] + 1;
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[usedPos2]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[usedPos2]->job->id] + 1;
				}
			}
			// cout << "Recalculando Job: " << alloc[usedPos]->job->id << " ExecTime: " << execTime << " latestJobVmFinish: " << latestJobVmFinish << " latestJobConflictFinish: " << latestJobConflictFinish << endl;

			newStartTimes[alloc[usedPos]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[usedPos]->job->id] = newStartTimes[alloc[usedPos]->job->id] + execTime; // tempo de fim
		}

		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes.size(); i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		double cost = 0.0;
		for(int i = 0; i < jobs.size(); i++){
			int j = jobs[i]->id;
			int timeUsed = newFinishTimes[j] - newStartTimes[j] + 1;
			int machineId = jobs[i]->alocated_vm_id;
			cost += timeUsed * vms[machineId]->usage_cost;
		}
		return this->ponderation*(biggestSpan / this->maxTime) + (1.0 - this->ponderation)*(cost / this->maxCost);

	}

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
		this->maxCost = p.maxCost;
		this->maxTime = p.maxTime;

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
				// cout << "CL for JOBID: " << jobs[i]->name << endl;
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
		this->checkFeasible();
		return calculateFO();
	}

	bool perturbateWriteTo(int pos){
		// int newVM = rand() % vms.size();
		// Job* job = alloc[pos]->job;
		
		// Machine * testVm = this->vms[newVM];
		
		// vector<int> newAlocations = vector<int>(files.size(), -1);
		// for(int f = 0; f < files.size(); f++){
		// 	int id = files[f]->alocated_vm_id;
		// 	newAlocations[files[f]->id] = id;
		// }

		// for(int f = 0; f < job->output.size(); f++){
		// 	newAlocations[job->output[f]->id] = testVm->id;
		// }

		// double readtime = calculateReadtime(job, alloc[pos]->vms->id);

		// double writetime = calculateWritetimeWithChanges(job, alloc[pos]->vms->id, newAlocations);

		// double processtime = ceil(job->base_time * alloc[pos]->vms->slowdown);

		// double newTime = readtime + processtime + writetime;

		// vector<int> outputs;
		// for(int i = 0; i < job->output.size(); i++){
		// 	outputs.push_back(job->output[i]->id);
		// }
		
		// // cout << "Calculating cascade effect" << endl;
		// vector<double> newFinishTimes = vector<double>(this->jobs.size(), 0.0);
		// vector<double> newStartTimes = vector<double>(this->jobs.size(), 0.0);
		// double newSpan = calculate_swapMachineWrite_effect(pos, job, newTime, outputs, testVm->id, newStartTimes, newFinishTimes);

		// this->execSwapMachineWrite(pos, testVm->id, newStartTimes, newFinishTimes);

		return true;
	}

	bool perturbateMachine(int pos){

		// int newVM = rand() % vms.size();

		// Job* job = alloc[pos]->job;

		// Machine * testVm = this->vms[newVM];

		// double readtime = calculateReadtime(job, testVm->id);
		// double writetime = calculateWritetime(job, testVm->id, alloc[pos]->writeTo);
		// double processtime = ceil(job->base_time * testVm->slowdown);

		// double newTime = readtime + processtime + writetime;

		// vector<double> newFinishTimes = vector<double>(this->jobs.size(), 0.0);
		// vector<double> newStartTimes = vector<double>(this->jobs.size(), 0.0);
		// double newSpan = calculate_swapMachine_effect(pos, job, newTime, testVm->id, newStartTimes, newFinishTimes);

		// this->execSwapMachine(pos, testVm->id, newStartTimes, newFinishTimes);

		return true;
	}

	bool doMovement(int vm, int output, Job* job, bool GPU){
		
		bool pushed = vms[vm]->pushJob(job, output, getJobConflictMinSpam(job));
		if(pushed){
			Allocation * newAlloc = new Allocation();
			newAlloc->job = job;
			newAlloc->vms = vms[vm];
			newAlloc->writeTo = output;
			// newAlloc->GPU = GPU;
			alloc.push_back(newAlloc);
		}
		// return vms[vm]->pushJob(job, output, getJobConflictMinSpam(job), GPU);
		
		// cin.get();
		return pushed;
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
		return makespam + 1.0;
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

	Problem(string file, int maxTime, double maxCost){
		// cout << "Starting to read instance..." << endl;

		ifstream in_file(file);
		string line;

		this->maxTime = maxTime;
		this->maxCost = maxCost;

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
			// cout << "JobNAme: " << job_name << endl;
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
				// cout << "Output item: " << item->id << endl;
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
		// cin.get();

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
		int total_inputs = 0;
		int total_outputs = 0;
		int maxTimeCPU = 0;
		for(int j = 0; j < jobs; j++){
			maxTimeCPU += ceil(this->jobs[j]->base_time_cpu * slowest_machine_cpu);
			total_inputs += this->jobs[j]->input.size();
			total_outputs += this->jobs[j]->output.size();
		}

		// this->maxTime = maxTimeCPU + total_inputs + total_outputs;

		// this->maxCost = this->maxTime * more_expensive_process_gpu;


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

		// cin.get();

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
			// cout << "***" << endl;
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

		cout << "Conflicts: " << endl;
		for(int i = 0; i < this->jobs.size(); i++){
			for(int j = 0; j < this->jobs.size(); j++){
				cout << conflicts[i][j] << " ";
			}
			cout << endl;
		}
		cin.get();
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