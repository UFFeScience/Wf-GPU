def main():
    file_path = "/home/murilostockinger/Mestrado/Wf-GPU/input/gpu/SchedGPU_test.dag"
    write_path = "/home/murilostockinger/Mestrado/Wf-GPU/input/gpu/SchedGPU_test_fixed.dag"

    file_manager = open(file_path, "r")

    lines = file_manager.readlines()

    file_manager.close()

    changes_dict = {}

    next = 0

    for i in range(156, 466):
        line = lines[i].split(" ")
        old_id = int(line[0])
        if old_id != next:
            changes_dict[old_id] = next
            line[0] = next
            lines[i] = "".join("{} ".format(str(e)) for e in line).lstrip()
        next += 1

    for i in range(1, 280):
        line = lines[i].replace("\n", "").split(" ")
        for j in range(3, len(line) - 3):
            test = changes_dict.get(int(line[j]), None)
            if test is not None:
                line[j] = test
        lines[i] = "".join("{} ".format(str(e)) for e in line).lstrip() + "\n"

    file_manager = open(write_path, "w")
    file_manager.write(lines[0])

    for i in range(1, 466):
        file_manager.write("{}".format(lines[i]).lstrip())

    for i in range(466, len(lines)):
        file_manager.write(lines[i])

    file_manager.write("\n\n\n*****************\n\n\n")

    for key in changes_dict:
        file_manager.write("{} -> {}\n".format(key, changes_dict[key]))

    print("123")

main()