#!/bin/bash
# 
# Usage:
#   ./gendata.sh [OPTIONS] [AUDIO_FILE]
# 
# Description:
#   A script to generate data from a specified audio file
# 
# Arguments:
#   AUDIO_FILE      The input RAW audio file.
#                   Defaults to 'stv8n2_2s.raw'.
# 
# Options:
#   -h, --help      Show this help message and exit.
# 
# Examples:
#   # Generate data with a specific bitrate and file
#   ./data/bin/gendata.sh data/audio/stv8n2_2s.wav
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

audio=${1:-$DATA_DIR/audio/stv8n2_2s.raw}

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

# check evrc codec is exist
if [[ ! -e $DATA_DIR/bin/evrc_codec ]]; then
    echo "evrc codec not exist"
    exit 1
fi

# generate reference result
$DATA_DIR/bin/evrc_codec e $audio $DATA_DIR/enc.evrc
$DATA_DIR/bin/evrc_codec d $DATA_DIR/enc.evrc $DATA_DIR/dec.raw

# generate data file
xxd -i $audio $DATA_DIR/input.h
#  delete unused line
sed -i "/int/d" $DATA_DIR/input.h
# change variable name to 'input'
sed -i "s/char [a-zA-Z0-9_]*/char input/g" $DATA_DIR/input.h

xxd -i $DATA_DIR/enc.evrc $DATA_DIR/enc_evrc.h
sed -i "/int/d" $DATA_DIR/enc_evrc.h
sed -i "s/char [a-zA-Z0-9_]*/char enc_evrc/g" $DATA_DIR/enc_evrc.h

xxd -i $DATA_DIR/dec.raw $DATA_DIR/dec_raw.h
sed -i "/int/d" $DATA_DIR/dec_raw.h
sed -i "s/char [a-zA-Z0-9_]*/char dec_raw/g" $DATA_DIR/dec_raw.h

# cleanup temp files
rm $DATA_DIR/enc.evrc
rm $DATA_DIR/dec.raw

echo "audio file: $audio"
echo "input file: $DATA_DIR/input.h"
echo "enc_evrc file: $DATA_DIR/enc_evrc.h"
echo "dec_raw file: $DATA_DIR/dec_raw.h"
echo "Test data generated successfully!"
