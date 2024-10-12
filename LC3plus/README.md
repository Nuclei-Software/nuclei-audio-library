# LC3plus Codec

This is the [LC3plus](https://www.iis.fraunhofer.de/en/ff/amm/communication/lc3.html) encoder/decoder adapted for the Nuclei CPU.

The origin source code is available [here](https://github.com/arkq/LC3plus), current TS (Technical Specification) version is V1.3.1, the corresponding software version is [V1.6.4](https://github.com/arkq/LC3plus/releases/tag/v1.3.1).

We designed a `lc3plus_demo` to show how to use the LC3plus codec. In `lc3plus_demo`, we prepare a piece of audio data and first encode it by calling `lc3plus_enc16`, and then decode the encoded data by calling `lc3plus_dec16`. We also encode and decode the same audio data on x86 platform, and compare the results run on Nuclei CPU to ensure the correctness.

## File Structure

| Directory | Description |
| -- | -- |
| fixed_point | fixed point source files |
| float_point | float point source files |
| data | data manipulation source files and some test results |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the README.md of parent directory.

## Build

LC3plus has both floating point and fixed-point implementation. Nuclei CPU support some extensions, such as B (Bitmanip) extension and P extension, which can enhance the performance of the codec. So there are some different build options for both floating point and fixed-point version.

First, change to the directory where [Makefile](./Makefile) is located. We take Nuclei N300 CPU as an example.

To build **fixed-point** version without extension:

```shell
make CORE=n300 ARCH_EXT= FIXED_POINT=1 all
```

To build **floating-point** version without extension:

```shell
make CORE=n300 ARCH_EXT= FIXED_POINT=0 all
```

To build **fixed-point** version with B and P extension:

```shell
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x FIXED_POINT=1 all
```

To build **floating-point** version with B and P extension:

```shell
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x FIXED_POINT=0 all
```

For more information about Nuclei CPU Architecture extension, please refer to [ARCH_EXT](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext) section in Nuclei SDK documentation.

## Performance Test

The test audio is [in_1s.wav](./data/in_1s.wav), which is a 1-second duration, 16k sample rate, PCM_S16LE format audio file. The bitrate we specified is 20kbps, and the real bitrate after encoding is close to the specified bitrate.

The `enc.bin` and `dec.wav` in [data/fixed](./data/fixed/) and [data/float](./data/float/) is the encode and decode result run on x86 platform. And we use `xxd` tool to convert these two files into C header files to verify result correctness in code. 

The encoder process 10 ms of audio data each time, so the 1s duration of audio data should be processed at least 100 times. The decoder should follow inverse order, so the decoder should decode the frames for about 100 times(a little more than 100 times). We record the CPU cycles consumed to process each frame, and caclulate the average cycles as shown in the following table.

For w/o extension, the build option is `ARCH_EXT=`, for w/ extension, the build option is `ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x`.

    Test bistream: n300_dual_best_config_ku060_16M_7cd945994_18d811786_202408191002.bit

These results can be easily calculated by [data/bench/cmp.py](./data/bench/cmp.py).

### fixed-point

| case | fixed-point w/o ext | fixed-point w/ ext | speedup ratio |
| -- | -- | -- | -- |
| encode (avg cycles) | 771938.96 | 577580.34 | 1.34 |
| decode (avg cycles) | 307473.51 | 263427.15 | 1.17 |

### float-point

| case | float-point w/o ext | float-point w/ ext | speedup ratio |
| -- | -- | -- | -- |
| encode (avg cycles) | 4864125.64 | 4004559.35 | 1.21 |
| decode (avg cycles) | 1009454.70 | 833115.61 | 1.21 |

## Changelog

| operator/function | description | file |
| -- | -- | -- |
| saturate | using SCLIP32 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):262 |
| add | using KADD16 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):330 |
| sub | using KSUB16 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):382 |
| abs_s | using KABS16 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):428 |
| shl | using KSLRA16 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):504 |
| shr | using KSLRA16 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):590 |
| mult | using KHM16 to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):679 |
| L_mult | using KDMBB to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):739 |
| L_mac | using KDMABB to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):995 |
| L_msu | using KDMBB to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):1055 |
| L_add | using KADDW to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):1108 |
| L_sub | using KSUBW to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):1171 |
| L_shl | using KSLRAW to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):1342 |
| L_shr | using KSLRAW to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):1427 |
| L_mac0 | using KMABB to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):2318 |
| L_msu0 | using KMABB to replace | [basicop32.c](./fixed_point/basic_op/basop32.c):2377 |
