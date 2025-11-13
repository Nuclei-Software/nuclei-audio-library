# EVRC Codec

Enhanced Variable Rate Codec (EVRC) is a codec for voice communication.
It is only support **8k sampling rate**.

The origin source code comes from [maolin-cdzl/evrcc commit:3171eb](https://github.com/maolin-cdzl/evrcc/commit/3171eb34ca87b4b21299f365825c547fa94ab053)

## File Structure

| Directory | Description |
| -- | -- |
| code | EVRC source files |
| dspmath | EVRC source files |
| include | EVRC header files |
| data | data manipulation source files and some test results |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the parent
directory's README.

## Build

Switch to the directory containing the `Makefile`.
Below are build examples for the **Nuclei N300 CPU**.

```shell
make CORE=n300fd ARCH_EXT= all
```

Build with extra B and P extension:

```shell
make CORE=n300fd ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all
```

For details about Nuclei CPU architecture extensions, see the [ARCH_EXT section](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext)

> [!NOTE]
> The 'd'(double float) extension is necessary for EVRC codec,
or it will perform poorly.

## Performance Test

### Generate Test Data

Codec performance depends on the input audio data.
To testing with different audio files, a helper script is provided:

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
Nuclei SDK Build Time: Nov 13 2025, 14:57:32
Download Mode: ILM
CPU Frequency 15999631 Hz
CPU HartID: 0
encode 100 frames for 2000 ms, use 215544227 cycles
CSV, evrc_encode, 107.77
Result matches!
decode 100 frames
CSV, evrc_decode, 8.32
Result matches!
```

The key results are prefixed with `CSV`, making them easy to parse in scripts
for further analysis.

The performance metric is reported in **MCPS (Milion Cycles Per Second)**, which
represents the number of CPU cycles required to process one second of audio data.
