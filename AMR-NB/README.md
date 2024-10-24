# AMR-NB Codec

This is the AMR-NB encoder/decoder adapted for the Nuclei CPU.

The origin source code is available [here](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/), current version is [0.1.6](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/opencore-amr-0.1.6.tar.gz/download).

AMR-NB only support 8k sample rate. We designed a `amrnb_demo` to show how to use the SBC codec by encoding/decoding loop. We also encode and decode the same audio data on x86 platform, and compare the results run on Nuclei CPU to ensure the correctness.

## File Structure

| Directory | Description |
| -- | -- |
| enc | AMR-NB encoding source files |
| dec | AMR-NB decoding source files |
| common | AMR-NB base operators used in both encoding and decoding |
| oscl | part of the origin source files |
| data | data manipulation source files and some test results |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the README.md of parent directory.

## Build

First, change to the directory where `Makefile` is located. We take Nuclei N300 CPU as an example.

To build without extension:

```shell
make CORE=n300fd ARCH_EXT= all
```

To build with B and P extension:

```shell
make CORE=n300fd ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all
```

For more information about Nuclei CPU Architecture extension, please refer to [ARCH_EXT](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext) section in Nuclei SDK documentation.

## Performance Test

The data for test input is prepared in [in_1s_8k.h](./data/in_1s_8k.h). The input data is generated from [in_1s_8k.wav](./data/in_1s_8k.wav) by `xxd` tool. The [enc.amr](./data/enc.amr) is the encoded output run on x86 platform, and the [dec.wav](./data/dec.wav) is the decoded output run on x86 platform. We also transfer these two files to [enc_amr.h](./data/enc_amr.h) and [dec_wav.h](./data/dec_wav.h) for reference. We compare the results run on Nuclei CPU with the reference output to ensure the correctness. 

We record the CPU cycles consumed to encode/decode, and caclulate the average cycles as shown in the following table. To show the performance of Nuclei CPU extensions, we compare the cpu cycles consumed between w/ and w/o extension. For w/o extension, the build option is `ARCH_EXT=`, for w/ extension, the build option is `ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x`.

    Test bitstream: n300_dual_best_config_ku060_16M_7cd945994_18d811786_202408191002.bit

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| encode | 578708.10 | 528383.84 | 1.10 |
| decode | 124616.40 | 122306.18 | 1.02 |
