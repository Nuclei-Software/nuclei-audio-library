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
#                   Defaults to 12200.
#                   Supported values: 4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200.
# 
#   AUDIO_FILE      The input WAV audio file.
#                   Defaults to 'stv8n2_2s.wav'.
# 
# Options:
#   -h, --help      Show this help message and exit.
# 
# Examples:
#   # Generate data with a specific bitrate and file
#   ./gendata.sh 12200 data/audio/stv8n2_2s.wav
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
DATA_DIR=$(realpath "$SCRIPT_DIR/..")
echo "DATA_DIR=$DATA_DIR"

bitrate=${1:-12200}
audio=${2:-$DATA_DIR/audio/stv8n2_2s.wav}

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
available_bitrates=(4750 5150 5900 6700 7400 7950 10200 12200)
if [[ ! " ${available_bitrates[@]} " =~ " $bitrate " ]]; then
    echo "bitrate not support"
    show_help
    exit 1
fi

# check amrnb codec is exist
if [[ ! -e $DATA_DIR/bin/amrnb-enc || ! -e $DATA_DIR/bin/amrnb-dec ]]; then
    echo "amrnb encoder or decoer not exist"
    exit 1
fi

# generate reference result
$DATA_DIR/bin/amrnb-enc -r $bitrate -d $audio $DATA_DIR/enc.amr
$DATA_DIR/bin/amrnb-dec $DATA_DIR/enc.amr $DATA_DIR/dec.wav

# generate data file
xxd -i $audio $DATA_DIR/input.h
#  delete unused line
sed -i "/int/d" $DATA_DIR/input.h
# change variable name to 'input'
sed -i "s/char [a-zA-Z0-9_]*/char input/g" $DATA_DIR/input.h
# add bitrate
echo "#define BITRATE ($bitrate)" >> $DATA_DIR/input.h

xxd -i $DATA_DIR/enc.amr $DATA_DIR/enc_amr.h
sed -i "/int/d" $DATA_DIR/enc_amr.h
sed -i "s/char [a-zA-Z0-9_]*/char enc_amr/g" $DATA_DIR/enc_amr.h

xxd -i $DATA_DIR/dec.wav $DATA_DIR/dec_wav.h
sed -i "/int/d" $DATA_DIR/dec_wav.h
sed -i "s/char [a-zA-Z0-9_]*/char dec_wav/g" $DATA_DIR/dec_wav.h

# cleanup temp files
rm $DATA_DIR/enc.amr
rm $DATA_DIR/dec.wav

echo "audio file: $audio"
echo "bitrate: $bitrate"
echo "input file: $DATA_DIR/input.h"
echo "enc_amr file: $DATA_DIR/enc_amr.h"
echo "dec_wav file: $DATA_DIR/dec_wav.h"
echo "Test data generated successfully!"
