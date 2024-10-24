# Sub-band Codec

This is the SBC encoder/decoder adapted for the Nuclei CPU.

The origin source code is available [here](https://github.com/google/libsbc), current version is based on the commit [6e50565](https://github.com/google/libsbc/commit/6e505650145c9973d08a0bdd5e5f5e1914305e40).

We designed a `sbc_demo` to show how to use the SBC codec by encoding/decoding loop. We also encode and decode the same audio data on x86 platform, and compare the results run on Nuclei CPU to ensure the correctness.

## File Structure

| Directory | Description |
| -- | -- |
| src | libsbc source files |
| include | libsrc public header files |
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

The data for test input is prepared in [input.h](./data/input.h). The input data is generated from [in_1s.wav](./data/in_1s.wav) by `xxd` tool. The [enc.sbc](./data/enc.sbc) is the encoded output run on x86 platform, and the [dec.wav](./data/dec.wav) is the decoded output run on x86 platform. We also transfer these two files to [enc_sbc.h](./data/enc_sbc.h) and [dec_wav.h](./data/dec_wav.h) for reference. We compare the results run on Nuclei CPU with the reference output to ensure the correctness. 

We record the CPU cycles consumed to encode/decode, and caclulate the average cycles as shown in the following table. To show the performance of Nuclei CPU extensions, we compare the cpu cycles consumed between w/ and w/o extension. For w/o extension, the build option is `ARCH_EXT=`, for w/ extension, the build option is `ARCH_EXT=_zba_zbb_zbc_zbs_xxldspn3x`.

    Test bitstream: n300_dual_best_config_ku060_16M_7cd945994_18d811786_202408191002.bit

| case | w/o ext (avg cycles) | w/ ext (avg cycles) | speedup ratio |
| -- | -- | -- | -- |
| encode | 18954.86 | 17458.26 | 1.09 |
| decode | 17485.30 | 16241.40 | 1.08 |
