# Helix MP3 Decoder

Helix MP3 Decoder is a high-performance MP3 decoding library designed to provide swift and reliable audio data decoding capabilities. This project is based on Lefucjusz's [Helix-MP3-Decoder](https://github.com/Lefucjusz/Helix-MP3-Decoder), commit [31773ea](https://github.com/Lefucjusz/Helix-MP3-Decoder/commit/31773ea10f829663b9bc4d82178284b74d9e5177).

We designed a `helix_demo` to demonstrate how to use the Helix MP3 Decoder and run on Nuclei CPU.

## File Structure

| Directory | Description |
| -- | -- |
| src | source files |
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

The data for test input is prepared in [input.h](./data/input.h). The input data is generated from [in_1s_32k.mp3](./data/in_1s_32k.mp3) by `xxd` tool. And the reference output is in [output.h](./data/output.h). The output data is generated from [out_32k.raw](./data/out_32k.raw) which is the decoded output run on x86 platform. We compare the results run on Nuclei CPU with the reference output to ensure the correctness. 

We record the CPU cycles consumed to decode MP3 data, and caclulate the average cycles as shown in the following table. To show the performance of Nuclei CPU extensions, we compare the cpu cycles consumed between w/ and w/o extension. For w/o extension, the build option is `ARCH_EXT=`, for w/ extension, the build option is `ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x`.

    Test bitstream: n300_dual_best_config_ku060_16M_7cd945994_18d811786_202408191002.bit

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| mp3 decode | 405719.97 | 246706.90 | 1.64 |
