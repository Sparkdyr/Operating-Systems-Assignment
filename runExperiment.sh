#!/bin/bash

# Experiment settings
REPEATS=5
OUTPUT="thread2.csv"

PROGRAMS=(
    "./thread2"
)

INPUTS=(
    "test1_small.txt"
    "test1_random.txt"
    "test1_manyTasks.txt"
)

# Temporary file for timing results
TIME_FILE=$(mktemp)

# Write CSV header
echo "program,input,average_time" > "$OUTPUT"

for program in "${PROGRAMS[@]}"; do
    for input in "${INPUTS[@]}"; do

        total=0

        for ((i=1; i<=REPEATS; i++)); do

            # Run program, discard its output, and save execution time
            /usr/bin/time -f "%e" -o "$TIME_FILE" \
                "$program" < "$input" > /dev/null

            time=$(cat "$TIME_FILE")

            total=$(awk -v total="$total" -v time="$time" \
                'BEGIN {print total + time}')
        done

        average=$(awk -v total="$total" -v repeats="$REPEATS" \
            'BEGIN {print total / repeats}')

        echo "$(basename "$program"),$(basename "$input"),$average" >> "$OUTPUT"

        echo "$(basename "$program") with $(basename "$input"): $average seconds"
    done
done

rm "$TIME_FILE"

echo ""
echo "Results written to $OUTPUT"