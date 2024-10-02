#!/bin/bash

# smallsh executable
SMALLSH_EXEC="./smallsh"


run_test() {
    local description="$1"
    local command="$2"
    local expected_output="$3"

    echo "== $description =="
    echo "Running: $command"
    result=$($SMALLSH_EXEC <<< "$command")

    if [[ "$result" == "$expected_output" ]]; then
    echo "Passed"
    else
    echo "Failed"
    echo "Expected: $expected_output"
    echo "Got: $result"
    fi
            echo
}

# echo with a comment
run_test "Test 1: Echo with comment" "_echo Hello World! # this is a comment" "Hello World!"

# Using $$ (PID)
run_test "Test 2: Echo with PID" "_echo $$" "$$"

# Testing exit status
run_test "Test 3: Exit and echo exit status" "_exit 82\n_echo $?" "82"

# Background process with $!
run_test "Test 4: Background process" "_suspend &\n_echo $!" "[PID]"

# Parameter expansion
run_test "Test 5: Parameter expansion" "_echo ${P_QgLmuKY}" "evKMARhG"

# Multiple parameters in one command
run_test "Test 6: Multiple parameters" "_echo ${P_QgLmuKY}${P_2p7K6AfFgbRV}" "evKMARhGOQTJRgvdhh"

# Exit command
run_test "Test 7: Exit with status" "exit 32" ""

# Change directory and print working directory
run_test "Test 8: cd command" "cd /tmp\npwd" "/tmp"

# Input redirection with `<`
run_test "Test 9: Input redirection" "cat < testfile" "Hello World!"

# Output redirection with `>`
run_test "Test 10: Output redirection" "printf Goodbye World!\\n > testfile\ncat testfile" "Goodbye World!"

# Appending output with `>>`
run_test "Test 11: Append output" "echo Hello World! >> testfile\ncat testfile" "Goodbye World!\nHello World!"

echo "All tests completed!"
