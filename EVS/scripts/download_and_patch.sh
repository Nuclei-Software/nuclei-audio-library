#!/bin/bash

EVS_SRC_URL="https://www.3gpp.org/ftp/Specs/archive/26_series/26.442/26442-j00.zip"
EVS_SRC_MD5SUM="7013649b4e594ed1478b24c2a7f4798e"

# Function to download a file from URL and check its md5sum
download_and_check() {
    local url="$1"
    local expected_md5sum="$2"
    local local_path=${3:-/opt/archive}
    local filename=$(basename "$url")
    local filepath="downloads/$filename"
    echo "local_path: $local_path"

    # Check if file already exists
    if [ -f "$filepath" ]; then
        echo "$filename already exists, checking md5sum..."

        # Calculate the md5sum of the existing file
        local actual_md5sum
        actual_md5sum=$(md5sum "$filepath" | cut -d' ' -f1)

        # Compare the calculated md5sum with the expected one
        if [ "$actual_md5sum" = "$expected_md5sum" ]; then
            echo "md5sum verification passed for $filename (file already exists)"
            return 0
        else
            echo "md5sum verification failed for $filename (file exists but checksum is wrong)"
            echo "Expected: $expected_md5sum"
            echo "Actual: $actual_md5sum"
            echo "Re-downloading $filename..."
        fi
    else
        echo "Downloading $filename..."
        # Download the file to the downloads directory if it doesn't exist or has wrong checksum
        if [ -f "$local_path/$filename" ]; then
            ln -sf "$local_path/$filename" "$filepath"
        elif ! wget -O "$filepath" "$url"; then
            echo "Error: Failed to download $filename"
            return 1
        fi
    fi

    # Calculate the md5sum of the downloaded file
    local actual_md5sum
    actual_md5sum=$(md5sum "$filepath" | cut -d' ' -f1)

    # Compare the calculated md5sum with the expected one
    if [ "$actual_md5sum" = "$expected_md5sum" ]; then
        echo "md5sum verification passed for $filename"
        return 0
    else
        echo "md5sum verification failed for $filename"
        echo "Expected: $expected_md5sum"
        echo "Actual: $actual_md5sum"
        rm "$filepath"  # Remove the corrupted file
        return 1
    fi
}

download() {
    local url="$1"
    local local_path=${2:-/opt/archive}
    local filename=$(basename "$url")
    local filepath="downloads/$filename"

    # Download the file to the downloads directory if it doesn't exist
    if [ -f "$filepath" ]; then
        echo "$filename already exists, skipping download..."
    elif [ -f "$local_path/$filename" ]; then
        echo "$filename already exists in $local_path, skipping download..."
        pushd downloads > /dev/null
        ln -sf "$local_path/$filename"
        popd > /dev/null
    elif ! wget -O "$filepath" "$url"; then
        echo "Error: Failed to download $filename"
        return 1
    fi
}

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
pushd "$SCRIPT_DIR" > /dev/null

local_path=${1:-/opt/archive}
echo "local_path: $local_path"

# check downloads directory is exist
if [ ! -d "downloads" ]; then
    mkdir downloads
fi

if ! download_and_check "$EVS_SRC_URL" "$EVS_SRC_MD5SUM" "$local_path"; then
    echo "Failed to download or verify EVS source archive"
    exit 1
fi
# extract zip files
rm -rf src
unzip downloads/$(basename "$EVS_SRC_URL") -d src
unzip src/*.zip -d src
rm -f src/*.zip src/*.docx
# apply patch
pushd src > /dev/null
patch -p1 < ../rv32p_opt.patch
popd > /dev/null

# extract baremetal needed source code
rm -rf ../src
cp -r src ../
pushd ../src/c-code > /dev/null
rm *.exe Makefile readme.txt basic_op/basop.rme
rm -rf Workspace_msvc
popd > /dev/null

# apply baremetal patch
pushd ../src > /dev/null
patch -p1 < ../scripts/baremetal.patch
popd > /dev/null

popd > /dev/null
