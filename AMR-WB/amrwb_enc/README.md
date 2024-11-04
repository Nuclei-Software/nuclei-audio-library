# AMR-WB Encoder

This is the [AMR-WB encoder](https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_Wideband) adapted for the Nuclei CPU.

The origin source code is available [here](https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/), current version is [0.1.3](https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/vo-amrwbenc-0.1.3.tar.gz/download).

## Overview

The AMR-WB encoder is an implementation of the Adaptive Multi-Rate Wideband speech codec. It operates at a higher frequency range compared to narrowband codecs, providing better audio quality.

## File Structure

| Directory | Description |
| -- | -- |
| inc | include files |
| src | source files |
| result | test result |

## Prerequests

Please refer to the [Prerequests](../../README.md#prerequests) section in the README.md of parent directory.

## Build

First, change to the directory where `Makefile` is located. We take Nuclei N300 CPU as an example.

To build without extension:

```shell
make CORE=n300 ARCH_EXT= all
```

To build with B and P extension:

```shell
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x all
```

For more information about Nuclei CPU Architecture extension, please refer to [ARCH_EXT](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext) section in Nuclei SDK documentation.

## Performance Test

The data for test input is prepared in [input.h](./input.h). The input data is a single channel, 16k sample rate, PCM_S16LE format audio file. We set output bitrate as 23850bps, the encoder process 20 ms of audio data each time, the input audio data will split into several frames and the encoder encode each frame one by one. We record the CPU cycles consumed to process each frame, and caclulate the average cycles as shown in the following table.

To show the performance of Nuclei CPU extensions, we compare the cpu cycles consumed between w/ and w/o extension. For w/o extension, the build option is `ARCH_EXT=`, for w/ extension, the build option is `ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x`.

    Test bitstream: n300_dual_best_config_ku060_16M_7cd945994_18d811786_202408191002.bit

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| amrwb-enc | 1497434.15 | 1215188.38 | 1.23 |

    Test bitstream: n300_best_config_ku060_16M_7cd945994_18d811786_202408191005.bit

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| amrwb-enc | 1817470.12 | 1362107.88 | 1.33 |

## Changelog

| operator/function | description | file |
| -- | -- | -- |
| saturate | using SCLIP16 to replace | [basic_op.h](./inc/basic_op.h):37 |
| vo_round | using KSLRAW.u to replace | [basic_op.h](./inc/basic_op.h):38 |
| shl | using KSLRA16 to replace | [basic_op.h](./inc/basic_op.h):237 |
| shr | using KSLRA16 to replace | [basic_op.h](./inc/basic_op.h):302 |
| mult | using KHM16 to replace | [basic_op.h](./inc/basic_op.h):371 |
| L_mult | using KDMBB to replace | [basic_op.h](./inc/basic_op.h):422 |
| voround | using KSLRAW to replace | [basic_op.h](./inc/basic_op.h):472 |
| L_mac | using KDMABB to replace | [basic_op.h](./inc/basic_op.h):523 |
| L_msu | using KDMBB to replace multiplication | [basic_op.h](./inc/basic_op.h):573 |
| L_add | using KADDW to replace | [basic_op.h](./inc/basic_op.h):617 |
| L_sub | using KSUBW to replace | [basic_op.h](./inc/basic_op.h):668 |
| L_shl | using KSLRAW to replace | [basic_op.h](./inc/basic_op.h):772 |
| L_shl2 | using KSLLW to replace | [basic_op.h](./inc/basic_op.h):817 |
| L_shr | using KSLRAW to replace | [basic_op.h](./inc/basic_op.h):880 |
| L_shr_r | using KSLRAW.u to replace | [basic_op.h](./inc/basic_op.h):962 |
| norm_s | using CLRS16 to replace | [basic_op.h](./inc/basic_op.h):1016 |
| norm_l | using CLRS32 to replace | [basic_op.h](./inc/basic_op.h):1159 |
| Deemph2 | using KSLRAW.u to replace shift and round | [deemph.c](./src/deemph.c):75 |
| Deemph_32 | using KSLRAW.u to replace shift and round | [deemph.c](./src/deemph.c):115 |
| Dot_product12 | using DSMALDA to achieve 4x parallelism | [math_op.c](./src/math_op.c):222 |
| Autocorr | using SMUL16 to double parallelism | [autocorr.c](./src/autocorr.c):56 |
| Autocorr | using KDMABB to replace | [autocorr.c](./src/autocorr.c):117 |
| Convolve | using KMAXDA to double parallelism | [convolve.c](./src/convolve.c):47-124 |
| cor_h_vec_012 | rvv optimization | [c4t64fx.c](./src/c4t64fx.c):948 |
| Convolve | rvv optimization | [convolve.c](./src/convolve.c):44 |
| Dot_product12 | rvv optimization | [math_op.c](./src/math_op.c):206 |
| Norm_Corr | rvv optimization | [pitch_f4.c](./src/pitch_f4.c):185 |