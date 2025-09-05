#!/bin/bash
# 
# Usage:
#   ./gendata.sh [OPTIONS] [BITRATE] [AUDIO_FILE]
# 
# Description:
#   A script to generate data from a specified audio file and bitrate.
# 
# Arguments:
#   BITRATE         The bitrate for encoding.
#                   Defaults to 6600.
#                   Supported values: 6600, 8850, 12650, 14250, 15850, 18250, 19850, 23050, 23850.
# 
#   AUDIO_FILE      The input WAV audio file.
#                   Defaults to 'stv16n2_2s.wav'.
# 
# Options:
#   -h, --help      Show this help message and exit.
# 
# Examples:
#   # Generate data with a specific bitrate and file
#   ./gendata.sh 6600 audio/stv8n2_2s.wav
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

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    show_help
    exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR=$(realpath "$SCRIPT_DIR/..")
echo "ROOT_DIR=$ROOT_DIR"

bitrate=${1:-6600}
audio=${2:-$ROOT_DIR/audio/stv16n2_2s.wav}

# check system is x86 linux
if [ "$(uname -m)" != "x86_64" ]; then
    echo "Please run on x86 linux"
    exit 1
fi

# check audio file is exist
if [ ! -e $audio ]; then
    echo "$audio not exist"
    exit 1
fi

#check bitrate
available_bitrates=(6600 8850 12650 14250 15850 18250 19850 23050 23850)
if [[ ! " ${available_bitrates[@]} " =~ " $bitrate " ]]; then
    echo "bitrate not support"
    show_help
    exit 1
fi

# check amrnb codec is exist
if [[ ! -e $ROOT_DIR/bin/amrwb-enc || ! -e $ROOT_DIR/bin/amrwb-dec ]]; then
    echo "amrwb encoder or decoer not exist"
    exit 1
fi

# generate reference result
$ROOT_DIR/bin/amrwb-enc -r $bitrate -d $audio $ROOT_DIR/enc.amr
$ROOT_DIR/bin/amrwb-dec $ROOT_DIR/enc.amr $ROOT_DIR/dec.wav

DATA_DIR=$ROOT_DIR/data
# generate data file
xxd -i $audio $DATA_DIR/input.h
#  delete unused line
sed -i "/int/d" $DATA_DIR/input.h
# change variable name to 'input'
sed -i "s/char [a-zA-Z0-9_]*/char input/g" $DATA_DIR/input.h
# add bitrate
echo "#define BITRATE ($bitrate)" >> $DATA_DIR/input.h

xxd -i $ROOT_DIR/enc.amr $DATA_DIR/enc_amr.h
sed -i "/int/d" $DATA_DIR/enc_amr.h
sed -i "s/char [a-zA-Z0-9_]*/char enc_amr/g" $DATA_DIR/enc_amr.h

xxd -i $ROOT_DIR/dec.wav $DATA_DIR/dec_wav.h
sed -i "/int/d" $DATA_DIR/dec_wav.h
sed -i "s/char [a-zA-Z0-9_]*/char dec_wav/g" $DATA_DIR/dec_wav.h

# cleanup temp files
rm $ROOT_DIR/enc.amr
rm $ROOT_DIR/dec.wav

echo "audio file: $audio"
echo "bitrate: $bitrate"
echo "input file: $DATA_DIR/input.h"
echo "enc_amr file: $DATA_DIR/enc_amr.h"
echo "dec_wav file: $DATA_DIR/dec_wav.h"
echo "Test data generated successfully!"
