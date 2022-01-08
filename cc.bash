#!/bin/bash
gcc -x c $1 2> /dev/null
./a.out ${@:2}
rm a.out 2> /dev/null
exit 0
