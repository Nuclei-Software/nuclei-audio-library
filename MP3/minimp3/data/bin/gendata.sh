#!/bin/bash
#
# Usage:
#   ./gendata.sh [OPTIONS]
#
# Description:
#   A script to generate data from a specified audio file and bitrate.
#
# Options:
#   -h, --help      Show this help message and exit.
#   -p, --pattern   Use test pattern from pattern.txt
#
# Examples:
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
DATA_DIR=$(realpath "$SCRIPT_DIR/..")

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

echo "DATA_DIR=$DATA_DIR"
pushd $DATA_DIR > /dev/null

pattern=${1:-ILL2_center2}
bitfile="./audio/${pattern}.bit"
pcmfile="./audio/${pattern}.pcm"

# check audio file is exist
if [[ ! -e $bitfile || ! -e $pcmfile ]]; then
    echo "${bitfile} or"
    echo "${pcmfile} not exist!"
    exit 1
fi

# generate reference result
decode_cmd="./minimp3_test $bitfile $pcmfile"

# generate encode data
xxd -i $bitfile $DATA_DIR/bit_input.h
# change variable name to 'bit_input'
sed -i "s/char [a-zA-Z0-9_]*/char bit_input/g" $DATA_DIR/bit_input.h
# define input and output length
sed -i -E 's/^.* = ([0-9]+);/#define BIT_INPUT_LEN \1/' $DATA_DIR/bit_input.h

# generate decode data
xxd -i $pcmfile $DATA_DIR/pcm_output.h
# change variable name to 'pcm_input'
sed -i "s/char [a-zA-Z0-9_]*/char pcm_output/g" $DATA_DIR/pcm_output.h
# define input and output length
sed -i -E 's/^.* = ([0-9]+);/#define PCM_OUTPUT_LEN \1/' $DATA_DIR/pcm_output.h

# add encode and decode command
echo "#include <string.h>" > $DATA_DIR/args.h
cmd_to_argv $DATA_DIR/args.h decode_cmd $decode_cmd

popd > /dev/null

echo "bitfile: $bitfile"
echo "pcmfile: $pcmfile"
echo "Test data generated successfully!"
