import sys
import csv
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} results.csv")
        sys.exit(1)

    filename = sys.argv[1]

    programs = []
    inputs = {}

    with open(filename, newline="") as file:
        reader = csv.DictReader(file)

        for row in reader:
            program = row["program"]
            input_file = row["input"]
            average_time = float(row["average_time"])

            if program not in programs:
                programs.append(program)

            if input_file not in inputs:
                inputs[input_file] = {}

            inputs[input_file][program] = average_time

    x = range(len(programs))

    for input_file, times in inputs.items():
        y = [times.get(program, 0) for program in programs]

        plt.plot(
            x,
            y,
            marker="o",
            label=input_file
        )

    plt.xticks(x, programs, rotation=45, ha="right", fontsize=8)
    plt.xlabel("Program")
    plt.ylabel("Average execution time (seconds)")
    plt.title("Average Execution Time")
    plt.legend()
    plt.grid(True)
    plt.ylim(0, 10)
    plt.tight_layout()

    plt.savefig("graph1b_longer.png", dpi=300)
    plt.show()


if __name__ == "__main__":
    main()