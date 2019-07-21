import sys


def main():
    args_size = len(sys.argv)
    seed_number = int(sys.argv[len(sys.argv) - 2])
    total_instance = int(sys.argv[len(sys.argv) - 1])

    for file_name in range(1, args_size - 2):
        file_reader = open(sys.argv[file_name], "r")
        lines = file_reader.readlines()

        current_line = 0
        new_lines = []
        for instance in range(total_instance):
            instance_name = lines[current_line].split(" ")[0]
            value_sum = 0.0
            value_best = 999999.0
            time_sum = 0.0
            for seed in range(seed_number):
                value = float(lines[current_line + seed].split(" ")[1])
                if value < value_best:
                    value_best = value
                value_sum += value
                time_sum += float(lines[current_line + seed].split(" ")[2])
            new_line = "{},{},{},{}\n".format(instance_name, value_best, value_sum / seed_number, time_sum / seed_number)
            new_lines.append(new_line)
            current_line += seed_number

        file_reader.close()

        file_reader = open("{}_table.csv".format(sys.argv[file_name]), "w")
        file_reader.write("instance,best,avg,time\n")
        for nl in new_lines:
            file_reader.write(nl)
        file_reader.close()


main()