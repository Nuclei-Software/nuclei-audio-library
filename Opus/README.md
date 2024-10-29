# Opus Codec

This is the [Opus](https://www.opus-codec.org/) encoder/decoder adapted for the Nuclei CPU.

The origin source code is available [here](https://github.com/xiph/opus), current version is [v1.5.2](https://github.com/xiph/opus/releases/tag/v1.5.2).

We did not compile the code into a library as in the original repository, but instead compiled the executable directly from the source code. `opus_demo` is a test program we designed, which is suitable for running directly on the bare-metal Nuclei CPU.

In `opus_demo`, we prepare a piece of audio data and first encode it by calling `opus_encode`, and then decode the encoded data by calling `opus_decode` to obtain the processed audio data.

- We analyze the processed audio data to ensure the correctness of the opus encoding and decoding functionality;
- We measure the execution time of `opus_encode` and `opus_decode` to assess their performance.

## File Structure

| Directory | Description |
| -- | -- |
| celt | celt source files which is a part of Opus |
| silk | silk source files which is a part of Opus |
| include | Opus header files |
| src | Opus source files |
| data | data manipulation source files and some test results |
| reference | same test code but run on operating system as a reference |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the README.md of parent directory.

## Build

Opus has both floating point and fixed-point implementation. Nuclei CPU support some extensions, such as B (Bitmanip) extension and P extension, which can enhance the performance of the codec. So there are some different build options for both floating point and fixed-point version.

First, change to the directory where [Makefile](./Makefile) is located. We take Nuclei N300 CPU as an example.

To build **fixed-point** version without extension:

```shell
make CORE=n300 ARCH_EXT= FIXED_POINT=1 all
```

To build **floating-point** version without extension:

```shell
make CORE=n300fd ARCH_EXT= FIXED_POINT=0 all
```

To build **fixed-point** version with B and P extension:

```shell
make CORE=n300 ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x FIXED_POINT=1 all
```

To build **floating-point** version with B and P extension:

```shell
make CORE=n300fd ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x FIXED_POINT=0 all
```

For more information about Nuclei CPU Architecture extension, please refer to [ARCH_EXT](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext) section in Nuclei SDK documentation.

## Functional Test

We have two `opus_demo.c` files, one [opus_demo.c](./opus_demo.c) is for Nulcei CPU, and the other [reference/opus_demo.c](./reference/opus_demo.c) is for running on operating system with File I/O.

The test audio is [in_1s.wav](./reference/in_1s.wav), which is a 1-second duration, 16k sample rate, PCM_S16LE format audio file. We extract the data from [in_1s.wav](./reference/in_1s.wav) to obtain [in_1s.raw](./reference/in_1s.raw).

For Nuclei CPU baremetal environment, we use `xxd` tool to convert the raw format file [in_1s.raw](./reference/in_1s.raw) into data stored in [data/in_1s.h](./data/in_1s.h).

When run on Nuclei CPU, we print the decode output data to console when uncomment the `DUMP_DEC_RESULT` macro in [opus_demo.c](./opus_demo.c). Then we save these data into a log file [data/test/n300_fixed.txt](./data/test/n300_fixed.txt) and [data/test/n300_float.txt](./data/test/n300_float.txt) manually. And you can convert the data to raw format file by [to_raw.py](./data/test/to_raw.py). The raw files can be easily loaded by Audacity to show waveforms as below:

![test.png](./data/test/test.png)

Although these four audio are not exactly same, but they are very close to each other. And people can hardly tell the difference between them by ear.

## Performance Test

The encoder process 20 ms of audio data each time, so the 1s duration of audio data is divided into 50 frames. The decoder should follow inverse order, so the decoder should decode the frames for 50 times. We record the CPU cycles consumed to process each frame, and caclulate the average cycles as shown in the following table.

To show the performance of Nuclei CPU extensions, we compare the cpu cycles consumed between w/ and w/o extension. For w/o extension, the build option is `ARCH_EXT=`, for w/ extension, the build option is `ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x`.

    Test bitstream: n300_dual_best_config_ku060_16M_7cd945994_18d811786_202408191002.bit

These results can be easily calculated by [data/bench/cmp.py](./data/bench/cmp.py).

### fixed-point

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| encode | 6951502.84 | 5716036.98 | 1.22 |
| decode | 112640.22 | 106151.44 | 1.06 |

### float-point

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| encode | 2889317.46 | 2861932.38 | 1.01 |
| decode | 133390.06 | 126119.08 | 1.06 |

## Changelog

| operator/function | description | file |
| -- | -- | -- |
| SATURATE16 | using SCLI32 to replace | [fixed_riscv.h](./celt/fixed_riscv.h):11 |
| MAX32 | using MAXW to replace | [fixed_riscv.h](./celt/fixed_riscv.h):15 |
| MIN32 | using MINW to replace | [fixed_riscv.h](./celt/fixed_riscv.h):18 |
| VSHR32 | using KSLRAW to replace | [fixed_riscv.h](./celt/fixed_riscv.h):21 |
| MULT16_16_Q15 | using KHMBB to replace | [fixed_riscv.h](./celt/fixed_riscv.h):24 |
| PSHR32 | using SRA_U to replace | [fixed_riscv.h](./celt/fixed_riscv.h):27 |
