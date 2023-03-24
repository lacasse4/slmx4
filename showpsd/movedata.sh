#!/bin/bash
# Move 6 predefined files to the directory given in parameter
# The directory is created if it does not exist

if [ -z "$1" ]; then
    echo "usage: movedata.sh <new directory>"
    exit 0
fi

if [ -e "$1" ]; then
    if [ ! -d "$1" ]; then
        echo "$1 is not a directory"
        exit 1
    fi 
fi

mkdir -p $1
mv RAW_FRAMES $1/ 
mv DIFF_FRAMES $1/
mv FILTERED_FRAMES $1/
mv RMS_FRAMES $1/
mv MAX_FRAMES $1/
mv FREQ_FRAMES $1/
echo "move successful"

