import subprocess
import os
import sys


ANT = 10
ITERATION = 100

def run_program(k_value, status):
    k_value = int(k_value)
    status = int(status)
    
    input_file = "testing/input.txt"
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found")
        sys.exit(1)
    
    if( status == 0 ): # do not get the answer yet
        with open(input_file, "r") as f:
            result = subprocess.run(["./answer", str(k_value)], stdin=f, capture_output=True, text=True)
    print("OK!")

    answer_list = []
    if os.path.exists("./testing/answer/result.txt"):
        with open("./testing/answer/result.txt", "r") as f:
            answer_list = [line.strip() for line in f.readlines()]
    
    print(answer_list)

    for i in range(1, k_value + 1):
        with open(input_file, "r") as f:
            result = subprocess.run(["./main", str(ANT), str(ITERATION), str(i), str(answer_list[i - 1]) ], stdin=f, capture_output=True, text=True)
        with open("./testing/answer/ant_result.txt", "a") as f:
            f.write(result.stdout)

if __name__ == "__main__":
    run_program(sys.argv[1], sys.argv[2])