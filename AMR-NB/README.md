# AMR-NB Codec

This project provides the **AMR-NB encoder/decoder** optimized for **Nuclei CPUs**.

The origin source code comes from [opencore-amr](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/)
(current version :[0.1.6](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/opencore-amr-0.1.6.tar.gz/download)).

## File Structure

| Directory | Description |
| -- | -- |
| enc | AMR-NB encoding source files |
| dec | AMR-NB decoding source files |
| common | AMR-NB base operators used in both encoding and decoding |
| oscl | part of the origin source files |
| data | data manipulation source files and some test results |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the parent
directory's README.

## Build

Switch to the directory containing the `Makefile`.  
Below are build examples for the **Nuclei N300 CPU**.

Build without extension:

```shell
make CORE=n300 ARCH_EXT= all
```

Build with B and P extension:

```shell
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all
```

For details about Nuclei CPU architecture extensions, see the [ARCH_EXT section](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext)
in the Nuclei SDK documentation.

## Performance Test

### Generate Test Data

Codec performance depends on both bitrate and input audio data.  
To simplify testing with different bitrates and audio files, a helper script is provided:

👉 [data/bin/gendata.sh](./data/bin/gendata.sh)

> [!NOTE]  
> This script is only supported on **Linux x86_64**  
> It relies on prebuilt binaries (encoder/decoder) available only for x86_64.  
> You may also build these binaries yourself if needed.

To view usage details:

```bash
./data/bin/gendata.sh -h
```

### Expected Output

A typical output looks like this:

```txt
Nuclei SDK Build Time: Sep  3 2025, 17:55:22
Download Mode: ILM
CPU Frequency 50003968 Hz
CPU HartID: 0
Start Encoding...
bitrate: 12200
CSV, amrnb_encode, 23.05
Result matches!
Start Decoding...
CSV, amrnb_decode, 13.88
Result matches!
PASS
```

The key results are prefixed with `CSV`, making them easy to parse in scripts
for further analysis.

The performance metric is reported in **MCPS (Milion Cycles Per Second)**, which
represents the number of CPU cycles required to process one second of audio data.
