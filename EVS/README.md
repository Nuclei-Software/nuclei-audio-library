# EVS Codec

This project provides the **EVS(Enhanced Voice Services) codec** optimized for **Nuclei CPUs**.

The source code comes from [3GPP TS 26.442](https://portal.3gpp.org/desktopmodules/Specifications/SpecificationDetails.aspx?specificationId=1464)
(current version :[18.0.0](https://www.3gpp.org/ftp/Specs/archive/26_series/26.442/26442-i00.zip)).

## File Structure

| Directory | Description |
| -- | -- |
| basic_math | source files |
| basic_op | source files |
| data | test data and reference results |
| lib_com | source files |
| lib_dec | source files |
| lib_enc | source files |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the parent
directory's README.

## Build

Switch to the directory containing the `Makefile`.  
Below are build examples for the **Nuclei N900 CPU**.

Build without extension:

```shell
make CORE=n900 ARCH_EXT= all
```

Build with B and P extension:

```shell
make CORE=n900 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn1x all
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
./bin/gendata.sh -h
```

### Expected Output

A typical output looks like this:

```txt
Nuclei SDK Build Time: Sep 29 2025, 09:40:40
Download Mode: DDR
CPU Frequency 50303139 Hz
CPU HartID: 0
Start Encoding...
Benchmark initialized

===========================================================================
 EVS Codec 3GPP TS26.442 August 12, 2021.
 Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
===========================================================================


Input audio file:       audio/stv48n2_1s.raw
Output bitstream file:  enc.192

Input sampling rate:    48000 Hz
Average bitrate:        5.90 kbps
DTX:                    ON, CNG update interval = 8 frames

Bandwidth limited to WB. To enable FB coding, please use -max_band FB.


------ Running the encoder ------
Frames processed:       50
Encoding finished
CSV, evs_encode, 117.84
Result matches!
Start Decoding...

===========================================================================
 EVS Codec 3GPP TS26.442 August 12, 2021.
 Version 12.15.0 / 13.10.0 / 14.6.0 / 15.4.0 / 16.4.0
===========================================================================


Input bitstream file:   enc.192
Output synthesis file:  dec.raw

Output sampling rate:   48000 Hz
Bitrate:                2.80 kbps

------ Running the decoder ------
Frames processed:       50
Decoding finished

CSV, evs_decode, 66.79
Result matches!
PASS
```

The key results are prefixed with `CSV`, making them easy to parse in scripts
for further analysis.

The performance metric is reported in **MCPS (Milion Cycles Per Second)**, which
represents the number of CPU cycles required to process one second of audio data.
