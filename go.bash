#!/bin/bash
cat - >temp.go
go run temp.go
rm temp.go 2> /dev/null
exit 0
