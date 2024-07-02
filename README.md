# Audio Algorithm Optimization

Here is the directory structure of this repository

```shell
<REPO_ROOT>
├── README.md
├── amrwb_dec
└── amrwb_enc
```

- amrwb_enc: This is the AMR-WB encoding algorithm migrated from [vo-amrwbenc-0.1.3](https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/), which is only optimized for Nuclei dual-issue N300 CPU with P extension.
- amrwb_dec: This is the AMR-WB decoding algorithm migrated from [opencore-amr-0.1.6](https://sourceforge.net/projects/opencore-amr/files/opencore-amr/), which is only optimized for Nuclei dual-issue N300 CPU with P extension.

# Build

Clone the repository into `$NUCLEI_SDK_ROOT/application/baremetal/`, checkout to `develop` branch, and then build with SDK.

Change to the directory where `Makefile` is located. Then:

```shell
# build for none optimization
make CORE=n300 all
# build for p-ext optimization
make CORE=n300 ARCH_EXT=_xxldspn3x all
```

# Result

For the test of [amrwb_end](./amrwb_enc/) and [amrwb_dec](./amrwb_dec/), the data for test input is prepared in [input.h(amrwb_enc)](./amrwb_enc/input.h) and [input.h(amrwb_dec)](./amrwb_dec/input.h).Run program, then the test program will execute encoding/decoding for 50 times, and we store the data in corresponding `result` folder manually. The stored data represents the number of CPU cycles used for each encoding/decoding.

We calculate the percentage reduction in the number of cycles each time:

$$p = \frac{base\_cycles - opt\_cycles}{base\_cycles}\times 100\%$$

Then calculate the average of the 50 results as the final result. To check result, change directory into `result` folder, then run command:

```python
# for single-issue n300 CPU
./cmp.py single_origin.csv single_pext_opt.csv
# for dual-issue n300 CPU
./cmp.py dual_origin.csv dual_pext_opt.csv
```

the `.py` script need python3 installed in your local environment

| CPU | amrwb_enc | amrwb_dec |
| -- | -- | -- |
| n300 single-issue | 17.18% | 11.54% |
| n300 dual-issue | 15.13% | 10.70% |

# Changelog

## amrwb_enc

| operator/function | description | file |
| -- | -- | -- |
| saturate | using SCLIP16 to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):37 |
| vo_round | using KSLRAW.u to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):38 |
| shl | using KSLRA16 to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):237 |
| shr | using KSLRA16 to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):302 |
| mult | using KHM16 to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):371 |
| L_mult | using KDMBB to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):422 |
| voround | using KSLRAW to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):472 |
| L_mac | using KDMABB to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):523 |
| L_msu | using KDMBB to replace multiplication | [basic_op.h](./amrwb_enc/inc/basic_op.h):573 |
| L_add | using KADDW to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):617 |
| L_sub | using KSUBW to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):668 |
| L_shl | using KSLRAW to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):772 |
| L_shl2 | using KSLLW to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):817 |
| L_shr | using KSLRAW to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):880 |
| L_shr_r | using KSLRAW.u to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):962 |
| norm_s | using CLRS16 to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):1016 |
| norm_l | using CLRS32 to replace | [basic_op.h](./amrwb_enc/inc/basic_op.h):1159 |
| Deemph2 | using KSLRAW.u to replace shift and round | [deemph.c](./amrwb_enc/src/deemph.c):75 |
| Deemph_32 | using KSLRAW.u to replace shift and round | [deemph.c](./amrwb_enc/src/deemph.c):115 |
| Dot_product12 | using DSMALDA to achieve 4x parallelism | [math_op.c](./amrwb_enc/src/math_op.c):222 |
| Autocorr | using SMUL16 to double parallelism | [autocorr.c](./amrwb_enc/src/autocorr.c):56 |
| Autocorr | using KDMABB to replace | [autocorr.c](./amrwb_enc/src/autocorr.c):117 |
| Convolve | using KMAXDA to double parallelism | [convolve.c](./amrwb_enc/src/convolve.c):47-124 |

## amrwb_dec

| operator/function | description | file |
| -- | -- | -- |
| add_int16 | using KADD16 to replace | [pvamrwbdecoder_basic_op_cequivalent.h](./amrwb_dec/src/pvamrwbdecoder_basic_op_cequivalent.h):89 |
| sub_int16 | using KSUB16 to replace | [pvamrwbdecoder_basic_op_cequivalent.h](./amrwb_dec/src/pvamrwbdecoder_basic_op_cequivalent.h):132 |
| mult_int16 | using KHM16 to replace | [pvamrwbdecoder_basic_op_cequivalent.h](./amrwb_dec/src/pvamrwbdecoder_basic_op_cequivalent.h):173 |
| add_int32 | using KADDW to replace | [pvamrwbdecoder_basic_op_cequivalent.h](./amrwb_dec/src/pvamrwbdecoder_basic_op_cequivalent.h):214 |