from macpath import join


def main():
    file_path = "D:\\mestrado\\Wf-GPU\\input\\gpu\\SchedGPU.txt"
    write_path = "D:\\mestrado\\Wf-GPU\\input\\gpu\\SchedGPU_test.dag"

    file_path = "/home/murilostockinger/Mestrado/Wf-GPU/input/gpu/SchedGPU.txt"
    write_path = "/home/murilostockinger/Mestrado/Wf-GPU/input/gpu/SchedGPU_test.dag"

    file_manager = open(file_path, "r")

    lines = file_manager.readlines()
    file_manager.close()
    files_dict = {}

    id_count = 0

    for i in range(156, 1041):
        line = lines[i]
        file_name = line.split(" ")[0]
        file_id = files_dict.get(file_name, None)
        if file_id is None:
            files_dict[file_name] = id_count
            id_count += 1


    old_start_line = lines[0]
    tasks = old_start_line.split(" ")[0]
    files = old_start_line.split(" ")[1]
    machines = old_start_line.split(" ")[2]
    new_start_line = "{} {} {}\n".format(tasks, len(files_dict), machines)
    lines[0] = new_start_line

    file_manager = open(write_path, "w")
    file_manager.write(new_start_line)
    for i in range(1, 156):
        line = lines[i].split(" ")
        new_line = ""
        for j in range(len(line)):
            item = line[j]
            id = files_dict.get(item, None)
            if id is not None:
                new_line += " {}".format(id)
            else:
                new_line += " {}".format(item)
        file_manager.write(new_line.lstrip())

    used_dict = {}

    for i in range(156, 1041):
        line = lines[i].split(" ")
        id = line[0]
        is_used = used_dict.get(id, None)
        if is_used is None:
            used_dict[id] = True
            line[0] = files_dict[id]
            new_line = ""
            for j in range(len(line)):
                new_line += " {}".format(line[j])
            file_manager.write(new_line.lstrip())

    for i in range(1041, len(lines)):
        file_manager.write(lines[i])


    file_manager.write("\n\n\n*****************\n\n\n")

    for key in files_dict:
        file_manager.write("{} -> {}\n".format(key, files_dict[key]))

    file_manager.close()






    print("test")

main()
