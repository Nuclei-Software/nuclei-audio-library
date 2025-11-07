#!/bin/bash
# 
# Usage:
#   ./gendata.sh [OPTIONS] [BITRATE] [SAMPLERATE] [AUDIO_FILE] [DTX] [RF]
# 
# Description:
#   A script to generate data from a specified audio file and bitrate.
# 
# Arguments:
#   BITRATE         The bitrate for encoding.
#                   Defaults to 5900.
#                   Supported values: 
#                     EVS native: 5900, 7200, 8000. 9600, 13200, 16400
#                   24400, 32000, 48000, 64000, 96000, 128000
#                     AMR-WB: 6600, 8850, 12650, 14200, 15850, 18250
#                   19850, 23050, 23850
# 
#   SAMPLERATE      Input sampling rate in kHz, (8, 16, 32, 48)
# 
#   AUDIO_FILE      The input RAW audio file.
#                   Defaults to 'stv48n2_1s.raw'.
#   DTX             DTX update rate frames.
#                   Defaults to '8', 'none' for no DTX
#   RF              '-rf p o' in p,o format, see "EVS_cod -h" for more details
# 
# Options:
#   -h, --help      Show this help message and exit.
#   -p, --pattern   Use test pattern from pattern.txt 
# 
# Examples:
#   # Generate data with a specific bitrate and file
#   ./gendata.sh 13200 16 stv16n2_2s.raw 8 HI,3
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

bitrate=${1:-5900}
samplerate=${2:-48}
audio=${3:-"audio/stv48n2_1s.raw"}
dtx=${4:-"8"}
rf=${5:-""}

# check system is x86 linux
if [ "$(uname -m)" != "x86_64" ]; then
    echo "Please run on x86 linux"
    exit 1
fi

# check audio file is exist
if [ ! -e $audio ]; then
    echo "$audio not exist, the audio file path should related to the $DATA_DIR"
    exit 1
fi

if [ "$dtx" != "none" ]; then
    dtx="-dtx $dtx"
else
    dtx=""
fi

if [ -n "$rf" ]; then
    oldIFS=$IFS
    IFS=, read -r ind ofs <<< "$rf"
    IFS=$oldIFS
    rf="-rf $ind $ofs"
fi

#check bitrate
available_bitrates=(5900 7200 8000 9600 13200 16400 24400 32000 48000 64000 96000 128000)
available_bitrates+=(6600 8850 12650 14250 15850 18250 19850 23050 23850)
if [[ ! " ${available_bitrates[@]} " =~ " $bitrate " ]]; then
    echo "bitrate not support"
    show_help
    exit 1
fi

# check codec is exist
if [[ ! -e "$DATA_DIR/bin/EVS_cod" || ! -e "$DATA_DIR/bin/EVS_dec" ]]; then
    echo "EVS encoder or decoer not exist"
    exit 1
fi

# generate reference result
encode_cmd="./bin/EVS_cod $dtx $rf $bitrate $samplerate $audio enc.192"
decode_cmd="./bin/EVS_dec $samplerate enc.192 dec.raw"
eval $encode_cmd || exit 1
eval $decode_cmd || exit 1

# generate data file
xxd -i $audio $DATA_DIR/input.h
#  delete unused line
sed -i "/int/d" $DATA_DIR/input.h
# change variable name to 'input'
sed -i "s/char [a-zA-Z0-9_]*/char input/g" $DATA_DIR/input.h
# add bitrate
echo "#include <string.h>" >> $DATA_DIR/input.h
cmd_to_argv $DATA_DIR/input.h enc_argv $encode_cmd
cmd_to_argv $DATA_DIR/input.h dec_argv $decode_cmd

xxd -i $DATA_DIR/enc.192 $DATA_DIR/enc_192.h
sed -i "/int/d" $DATA_DIR/enc_192.h
sed -i "s/char [a-zA-Z0-9_]*/char enc_192/g" $DATA_DIR/enc_192.h

xxd -i $DATA_DIR/dec.raw $DATA_DIR/dec_raw.h
sed -i "/int/d" $DATA_DIR/dec_raw.h
sed -i "s/char [a-zA-Z0-9_]*/char dec_raw/g" $DATA_DIR/dec_raw.h

# cleanup temp files
rm $DATA_DIR/enc.192
rm $DATA_DIR/dec.raw

popd > /dev/null

echo "audio file: $DATA_DIR/$audio"
echo "bitrate: $bitrate"
echo "input file: $DATA_DIR/input.h"
echo "enc_amr file: $DATA_DIR/enc_192.h"
echo "dec_wav file: $DATA_DIR/dec_raw.h"
echo "Test data generated successfully!"
