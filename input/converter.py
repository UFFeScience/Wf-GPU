import random
import sys


class File(object):

    def __init__(self):
        self.size = 0.0
        self.static = True
        self.static_list = []
        self.id = ""


class Job(object):

    def __init__(self):
        self.id = ""
        self.name = ""
        self.cpu_time = 0.0
        self.gpu_time = 0.0
        self.gpu = False
        self.input = []
        self.output = []


class Machine(object):
    def __init__(self):
        self.id = 0
        self.name = ""
        self.cpu_slowdown = 0.0
        self.gpu_slowdown = 0.0
        self.bandwidth = 0
        self.storage = 0.0
        self.cpu_cost = 0.0
        self.has_gpu = True


def main():
    workflow_file_path = sys.argv[1]
    cluster_file_path = sys.argv[2]
    file_end = workflow_file_path.split(".")[1]
    file_start = workflow_file_path.split(".")[0]
    gpu_path = file_start + "_gpu" + "." + file_end

    file_reader = open(workflow_file_path, "r")

    lines = file_reader.readlines()

    job_dict = {}
    file_dict = {}
    file_code_dict = {}
    vm_dict = {}

    static_files = 0
    dinamic_files = 0
    total_jobs = 0

    change_made = True
    while change_made:
        change_made = False
        for line in range(len(lines)):
            if lines[line] == "\n":
                del lines[line]
                change_made = True
                break
    add_line = 0
    for line in range(len(lines)):
        line_added = line + add_line
        if line_added >= len(lines):
            break
        read_line = lines[line_added]

        split_line = read_line.split(" ")
        if line_added == 0:
            static_files = int(split_line[0])
            dinamic_files = int(split_line[1])
            total_jobs = int(split_line[2])
        elif line < static_files + 1:
            id = split_line[0]
            size = float(split_line[1])
            static_machines_n = int(split_line[2])
            static_id = []
            for i in range(3, 3 + static_machines_n):
                static_id.append(int(split_line[i]))
            new_file = File()
            file_code_dict[id] = len(file_code_dict)
            new_file.id = file_code_dict[id]
            new_file.size = size
            new_file.static = True
            new_file.static_list = static_id
            file_dict[id] = new_file
        elif line < dinamic_files + static_files + 1:
            id = split_line[0]
            size = float(split_line[1])
            new_file = File()
            file_code_dict[id] = len(file_code_dict)
            new_file.id = file_code_dict[id]
            new_file.size = size
            new_file.static = False
            file_dict[id] = new_file
        elif line < dinamic_files + static_files + total_jobs + 1:
            id = split_line[0]
            name = split_line[1]
            cpu_time = float(split_line[2])
            total_input = int(split_line[3])
            total_output = int(split_line[4])
            gpu = False
            gpu_possible = random.randint(1, 2)
            gpu_time = 0.0
            input_list = []
            output_list = []
            if gpu_possible == 1:
                gpu = True
                gpu_time = float("{0:.2f}".format(random.uniform(float(cpu_time) * 0.3, float(cpu_time) * 0.7)))
            for i in range(line_added + 1, line_added + total_input + 1):
                input_list.append(file_code_dict[lines[i].replace("\n", "")])
                add_line += 1
            for i in range(line_added + total_input + 1, line_added + total_input + 1 + total_output):
                output_list.append(file_code_dict[lines[i].replace("\n", "")])
                add_line += 1

            new_job = Job()
            new_job.cpu_time = cpu_time
            new_job.gpu = gpu
            new_job.gpu_time = gpu_time
            new_job.name = name
            new_job.id = id
            new_job.output = output_list
            new_job.input = input_list

            job_dict[id] = new_job

    file_reader.close()
    file_reader = open(cluster_file_path, "r")
    lines = file_reader.readlines()
    read_line = lines[1]
    split_line = read_line.split(" ")
    total_vms = int(split_line[len(split_line) - 1])
    for vm in range(2, total_vms + 2):
        read_line = lines[vm]
        split_line = read_line.split(" ")
        id = int(split_line[0])
        name = split_line[1]
        cpu_slowdown = float(split_line[2])
        gpu_slowdown = float("{0:.2f}".format(random.uniform(float(cpu_slowdown) * 0.3, float(cpu_slowdown) * 0.7)))
        storage = float(split_line[3]) * 1024.0
        bandwidth = float(split_line[4])
        cpu_cost = float(split_line[5])
        gpu_cost = float("{0:.2f}".format(random.uniform(float(cpu_cost) * 8.0, float(cpu_cost) * 15.0)))
        new_machine = Machine()
        new_machine.bandwidth = bandwidth
        new_machine.cpu_cost = cpu_cost
        new_machine.cpu_slowdown = cpu_slowdown
        new_machine.gpu_cost = gpu_cost
        has_gpu = random.randint(1, 2)
        new_machine.gpu_slowdown = gpu_slowdown
        if has_gpu == 1:
            new_machine.has_gpu = True
            new_machine.cpu_cost = gpu_cost
        else:
            new_machine.has_gpu = False
            new_machine.gpu_slowdown = 0.0
        new_machine.id = id
        new_machine.storage = storage
        new_machine.name = name
        vm_dict[id] = new_machine

    bandwidth_matrix = []

    for i in range(0, total_vms):
        band_line = []
        for j in range(0, total_vms):
            if i == j:
                band_line.append(0)
            else:
                min_band = vm_dict[i].bandwidth
                if vm_dict[j].bandwidth < min_band:
                    min_band = vm_dict[j].bandwidth
                band_line.append(min_band)
        bandwidth_matrix.append(band_line)

    file_reader.close()
    # WRITING NEW FILE!
    file_writer = open(gpu_path, "w")

    file_writer.write("{} {} {}\n".format(total_jobs, static_files + dinamic_files, total_vms))
    for job in job_dict:
        jobObj = job_dict[job]
        file_writer.write("{} {} {} {}".format(jobObj.id, jobObj.cpu_time, jobObj.gpu_time, len(jobObj.input)))
        for inp in jobObj.input:
            file_writer.write(" {}".format(inp))
        file_writer.write(" {}".format(len(jobObj.output)))
        for out in jobObj.output:
            file_writer.write(" {}".format(out))
        file_writer.write("\n")

    for file in file_dict:
        fileObj = file_dict[file]
        if fileObj.static:
            file_writer.write("{} {} 1 {}".format(fileObj.id, fileObj.size, len(fileObj.static_list)))
            for static in fileObj.static_list:
                file_writer.write(" {}".format(static))
        else:
            file_writer.write("{} {} 0".format(fileObj.id, fileObj.size))
        file_writer.write("\n")

    for i in range(0, 4):
        if i == 0:
            for vm in vm_dict:
                vmObj = vm_dict[vm]
                file_writer.write("{} ".format(vmObj.cpu_slowdown))
            file_writer.write("\n")
        if i == 1:
            for vm in vm_dict:
                vmObj = vm_dict[vm]
                file_writer.write("{} ".format(vmObj.gpu_slowdown))
            file_writer.write("\n")
        if i == 2:
            for vm in vm_dict:
                vmObj = vm_dict[vm]
                file_writer.write("{} ".format(vmObj.storage))
            file_writer.write("\n")
        if i == 3:
            for vm in vm_dict:
                vmObj = vm_dict[vm]
                file_writer.write("{} ".format(vmObj.cpu_cost))
            file_writer.write("\n")
        if i == 4:
            for vm in vm_dict:
                vmObj = vm_dict[vm]
                file_writer.write("{} ".format(vmObj.gpu_cost))
            file_writer.write("\n")

    for i in range(0, total_vms):
        for j in range(0, total_vms):
            file_writer.write("{} ".format(bandwidth_matrix[i][j]))
        file_writer.write("\n")

    file_writer.write("\n\n\n\nMAPING **************** \n")

    for id in file_code_dict:
        file_writer.write("{} -> {}\n".format(id, file_code_dict[id]))
    file_writer.close()


main()
