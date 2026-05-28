# minimp3 Decoder

This project provides the **minimp3 MP1/MP2/MP3 decoder** optimized for
**Nuclei CPUs**.

The source code comes from the upstream
[lieff/minimp3](https://github.com/lieff/minimp3) project.

## File Structure

| Directory | Description |
| -- | -- |
| data | test data, generated headers, and helper scripts |
| minimp3.h | core decoder implementation |
| minimp3_ex.h | extended streaming and file decoding interfaces |
| minimp3_test.c | test and benchmark entry |

## Prerequests

Please refer to the [Prerequests](../README.md#prerequests) section in the
parent directory's README.

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

Build with a specific test pattern:

```shell
make CORE=n900 PATTERN=1 all
```

When `PATTERN` is not specified, the build uses the pre-generated test data in
`data/`. When `PATTERN` is set, the build regenerates `bit_input.h`,
`pcm_output.h`, and `args.h` before compiling.

For details about Nuclei CPU architecture extensions, see the
[ARCH_EXT section](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext)
in the Nuclei SDK documentation.

## Performance Test

### Generate Test Data

The test program compares decoded PCM output against reference data embedded in
the build. A helper script is provided to switch among the bundled test
patterns:

👉 [data/bin/gendata.sh](./data/bin/gendata.sh)

To view usage details:

```bash
./data/bin/gendata.sh -h
```

To generate data from a specific pattern line in `pattern.txt`:

```bash
./data/bin/gendata.sh -p 1
```

The script updates:

- `data/bit_input.h`: embedded MP3 bitstream input
- `data/pcm_output.h`: embedded PCM reference output
- `data/args.h`: default decode command arguments

### Expected Output

A typical output looks like this:

```txt
Nuclei SDK Build Time: Sep 29 2025, 09:40:40
Download Mode: DDR
CPU Frequency 50303907 Hz
CPU HartID: 0
rate=44100 samples=4608 max_diff=0 PSNR=99.000000
PASS
```

The key validation metric is **PSNR**. The test is considered successful when
the decoded PCM matches the embedded reference closely enough and the program
prints `PASS`.

The `PATTERN` build variable can be used to benchmark multiple bundled vectors.
The CI configuration currently covers a large set of patterns from
`data/bin/pattern.txt`.
