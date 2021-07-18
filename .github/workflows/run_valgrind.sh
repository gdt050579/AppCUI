#!/bin/bash
timeout 5s valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -v ./bin/tester
exitcode=$?
echo "the error code is: $exitcode"
if (( $exitcode == 1 )); then
	# exit code 1 for errors
	exit 1
fi
# no errros
exit 0
