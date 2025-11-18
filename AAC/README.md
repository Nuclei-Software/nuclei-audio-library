# AAC Decoder

This directory contains the FAAD2 AAC decoder library,
which is capable of decoding AAC audio streams. The library
supports various AAC profiles, including LC (Low Complexity),
HE-AAC (High Efficiency AAC). **But HE-AAC v2 is not supported.**

The original source code comes from [FAAD2 2.8.8](https://sourceforge.net/p/faac/faad2/ci/2_8_8/tree/)

## File Structure

| Directory | Description |
| -- | -- |
| audio | Audio files used to test |
| bin   | Binaries to generate reference results |
| data  | Pre-generated test data |
| faad2 | FAAD2 source files |

## Prerequisites

Please refer to the [Prerequisites](../README.md#prerequests) section in the parent
directory's README.

## Build

Switch to the `AAC/faad2` directory containing the `Makefile`.
Ensure that the Nuclei SDK is properly set up and the `NUCLEI_SDK_ROOT` environment
variable is configured.

Below are build examples for the **Nuclei N300 CPU**.

```shell
make CORE=n300 ARCH_EXT= all
```

Build with extra B and P extensions:

```shell
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all
```

For details about Nuclei CPU architecture extensions, see the [ARCH_EXT section](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext)

## Performance Test

### Generate Test Data

Codec performance depends on the input audio data.
To test with different audio files, a helper script is provided:

👉 [bin/gendata.sh](./bin/gendata.sh)

> [!NOTE]  
> This script is only supported on **Linux x86_64**  
> It relies on prebuilt binaries (encoder/decoder) available only for x86_64.  
> You may also build these binaries yourself if needed.  
> And one more thing, the run results on RISC-V platform are not bit-exact with x86_64.

To view usage details:

```bash
./bin/gendata.sh -h
```

### Expected Output

A typical output looks like this:

```txt
```

The key results are prefixed with `CSV`, making them easy to parse in scripts
for further analysis.

The performance metric is reported in **MCPS (Million Cycles Per Second)**, which
represents the number of CPU cycles required to process one second of audio data.
