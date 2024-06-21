# Audio Algorithm Optimization

Here is the directory structure of this repository

```shell
<REPO_ROOT>
├── amrwb_enc 
└── README.md
```

- amrwb_enc: This is the AMR-WB encoding algorithm migrated from [vo-amrwbenc-0.1.3](https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/), which is only optimized for Nuclei dual-issue N300 CPU with P extension.

# Build

Clone the repository into `$NUCLEI_SDK_ROOT/application/baremetal/`, and build with SDK

## amrwb_enc

Change to the directory where `Makefile` is located. Then:

```shell
# build for none optimization
make CORE=n300 all
# build for p-ext optimization
make CORE=n300 ARCH_EXT=_xxldsp all
```


