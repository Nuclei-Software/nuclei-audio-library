# AMR-WB Codec

This project provides the **AMR-WB encoder/decoder** optimized for **Nuclei CPUs**.

The decoder source code comes from [opencore-amr](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/)
(current version :[0.1.6](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/opencore-amr-0.1.6.tar.gz/download)).

The encoder source code comes from [vo-amrwbenc](https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/)
(current version: [0.1.3](https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/vo-amrwbenc-0.1.3.tar.gz/download)).

## File Structure

| Directory | Description |
| -- | -- |
| amrwb_dec | AMR-WB decoding source files |
| amrwb_enc | AMR-WB decoding source files |
| audio | audio files used for test |
| bin | binaries to generate reference results |
| data | pre-generated test data |

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

👉 [bin/gendata.sh](./bin/gendata.sh)

> [!NOTE]  
> This script is only supported on **Linux x86_64**  
> It relies on prebuilt binaries (encoder/decoder) available only for x86_64.  
> You may also build these binaries yourself if needed.

To view usage details:

```bash
./bin/gendata.sh -h
```

### Expected Output

Run encoder and decoder in two corresponding sub-directories.

```bash
cd amrwb_enc
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all upload
cd ../amrwb_dec
makr CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all upload
```

A typical output looks like this:

```txt
Nuclei SDK Build Time: Sep  9 2025, 19:26:18
Download Mode: ILM
CPU Frequency 16006512 Hz
CPU HartID: 0
bitrate: 6600
CSV, amrwb_encode, 40.71
PASS

Nuclei SDK Build Time: Sep  9 2025, 19:26:38
Download Mode: ILM
CPU Frequency 16004546 Hz
CPU HartID: 0
CSV, amrwb_decode, 12.13
PASS
```

The key results are prefixed with `CSV`, making them easy to parse in scripts
for further analysis.

The performance metric is reported in **MCPS (Milion Cycles Per Second)**, which
represents the number of CPU cycles required to process one second of audio data.
