#!/bin/bash
# 
# Usage:
#   ./gendata.sh [AUDIO_FILE]
# 
# Description:
#   A script to generate data from audio file
# 
# Arguments:
# 
# Options:
#   -h, --help      Show this help message and exit.
#   -p, --pattern   Use test pattern from pattern.txt 
# 
# Examples:
#   # Generate data with a specific bitrate and file
#   ./gendata.sh audio/sr44k1_s16c2_2s.aac
# 
#   # Use default values
#   ./gendata.sh
# 
#   # Display help
#   ./gendata.sh -h
# 
show_help() {
    sed -n '2,/^[^#]/ { /^#/p }' "$0" | sed 's/^# //; s/^#//'
}

cmd_to_argv() {
    local file=$1
    local variable=$2
    local cmd="${@:3}"
    local arr=()
    
    read -a arr <<< "$cmd"

    echo "char *$variable[] = {" >> "$file"
    for arg in "${arr[@]}"; do
        echo "    \"$arg\", " >> "$file"
    done
    echo "    NULL};" >> "$file"
}

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PATTERNFILE="${SCRIPT_DIR}/pattern.txt"
ROOT_DIR=$(realpath "$SCRIPT_DIR/..")

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    show_help
    exit
elif [[ "$1" == "-p" || "$1"  == "--pattern" ]]; then
    line=${2:-1}
    shift 2
    # check pattern file is exist
    if [ ! -e $PATTERNFILE ]; then
        echo "$PATTERNFILE not exist"
        exit 1
    fi
    # get pattern from pattern file
    pattern=$(sed -n "${line}p" $PATTERNFILE)
    echo "pattern: $pattern"
    if [[ -z "$pattern" || "$pattern" =~ "^#" ]]; then
        echo "Invalid pattern line: $line"
        exit 1
    fi
    # rerun script with args from pattern file
    exec $SCRIPT_DIR/gendata.sh $pattern
fi

echo "ROOT_DIR=$ROOT_DIR"
pushd $ROOT_DIR > /dev/null

audio=${1:-"audio/sr44k1_s16c2_2s.aac"}

# check system is x86 linux
if [ "$(uname -m)" != "x86_64" ]; then
    echo "Please run on x86 linux"
    exit 1
fi

# check audio file is exist
if [ ! -e $audio ]; then
    echo "$audio not exist, the audio file path should related to the $ROOT_DIR"
    exit 1
fi

# check codec is exist
if [[ ! -e "$ROOT_DIR/bin/faad" ]]; then
    echo "FAAC decoder not exist"
    exit 1
fi

# generate reference result
decode_cmd="./bin/faad -o dec.wav $audio"
eval $decode_cmd || exit 1

# generate data file
xxd -i $audio $ROOT_DIR/data/input_data.h
#  delete unused line
sed -i "/int/d" $ROOT_DIR/data/input_data.h
# change variable name to 'input'
sed -i "s/char [a-zA-Z0-9_]*/char input/g" $ROOT_DIR/data/input_data.h
# add bitrate
echo "#include <string.h>" >> $ROOT_DIR/data/input_data.h
cmd_to_argv $ROOT_DIR/data/input_data.h dec_argv $decode_cmd

xxd -i dec.wav $ROOT_DIR/data/dec_wav.h
sed -i "/int/d" $ROOT_DIR/data/dec_wav.h
sed -i "s/char [a-zA-Z0-9_]*/char dec_wav/g" $ROOT_DIR/data/dec_wav.h

# cleanup temp files
rm $ROOT_DIR/dec.wav

popd > /dev/null

echo "audio file: $ROOT_DIR/$audio"
echo "input file: $ROOT_DIR/data/input_data.h"
echo "dec_wav file: $ROOT_DIR/data/dec_wav.h"
echo "Test data generated successfully!"
