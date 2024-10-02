#!/bin/bash


echo "== Test 1: Echo a simple message =="
echo "Running: echo Hello, World!"
./smallsh << EOF
echo Hello, World!
exit
EOF
echo "Expected: Hello, World!"
echo "----------------------"


echo "== Test 2: Show current directory =="
echo "Running: pwd"
./smallsh << EOF
pwd
exit
EOF
echo "Expected: Current working directory path"
echo "----------------------"


echo "== Test 3: Change directory =="
echo "Running: cd /tmp && pwd"
./smallsh << EOF
cd /tmp
pwd
exit
EOF
echo "Expected: /tmp"
echo "----------------------"


echo "== Test 4: Background process =="
echo "Running: sleep 5 &"
./smallsh << EOF
sleep 5 &
exit
EOF
echo "Expected: Process runs in background"
echo "----------------------"

echo "== Test 5: Input redirection =="
echo "Running: cat < input.txt"
echo "Hello, file!" > input.txt
./smallsh << EOF
cat < input.txt
exit
EOF
echo "Expected: Hello, file!"
echo "----------------------"


echo "== Test 6: Output redirection =="
echo "Running: echo 'Hello, redirected!' > output.txt"
./smallsh << EOF
echo "Hello, redirected!" > output.txt
cat output.txt
exit
EOF
echo "Expected: Hello, redirected!"
echo "----------------------"


echo "== Test 7: Append output =="
echo "Running: echo 'Appended text' >> output.txt"
./smallsh << EOF
echo "Appended text" >> output.txt
cat output.txt
exit
EOF
echo "Expected: Hello, redirected!\nAppended text"
echo "----------------------"


echo "== Test 8: PID expansion =="
echo "Running: echo $$"
./smallsh << EOF
echo $$
exit
EOF
echo "Expected: The current process ID"
echo "----------------------"


echo "== Test 9: Exit with status =="
echo "Running: exit 42"
./smallsh << EOF
exit 42
EOF
EXIT_STATUS=$?
echo "Expected: 42"
echo "Actual: $EXIT_STATUS"
echo "----------------------"
