# Nuclei Audio Library

The Nuclei Audio Library is a collection of audio processing algorithms and utilities optimized for the Nuclei CPU architecture. It provides a set of efficient and high-quality audio codecs, effects, and processing tools tailored for embedded systems and applications requiring low-power and high-performance audio processing.

## Table of Contents

- [Overview](#overview)
- [Prerequests](#prerequests)
- [Supported Libraries](#supported-libraries)
- [License](#license)

## Overview

The Nuclei Audio Library is designed to leverage the capabilities of the Nuclei CPU, offering a range of audio processing functionalities that are both power-efficient and high-performing. It is ideal for developers working on embedded systems, IoT devices, and other applications where audio processing is critical.

## Prerequests

To develop on Nuclei CPU with baremetal environment, we need to install Nuclei SDK and associated toolchain.

We recommend utilizing the latest version of the Nuclei SDK and associated toolchain for optimal performance and compatibility. For this project we use the following versions:

- [Nuclei SDK version 0.6.0](https://github.com/Nuclei-Software/nuclei-sdk/releases/tag/0.6.0)
- [Nuclei Studio IDE for Linux version 2024.06](https://download.nucleisys.com/upload/files/nucleistudio/NucleiStudio_IDE_202406-lin64.tgz)

Please adhere to the instructions outlined in the [Setup Tools and Environment](https://doc.nucleisys.com/nuclei_sdk/quickstart.html#get-and-setup-nuclei-sdk) section to properly prepare your Nuclei SDK and toolchain for use. Both Linux and Windows operating systems are supported, for the purpose of example, we will demonstrate the process using the Ubuntu 20.04 Linux operating system.

It is recommended to setup `NUCLEI_SDK_ROOT` environment variable to point to `/path/to/nuclei-sdk`.

```shell
export NUCLEI_SDK_ROOT=/path/to/nuclei-sdk
```

After that, no matter where this project located in, you can run make to build and run the programs in this repository.

Otherwise, you should place this project in the directory of `$NUCLEI_SDK_ROOT/application/baremetal`

## Supported Libraries

- [x] AMR-WB: Adaptive Multi-Rate Wideband. [encoder](./amrwb_enc/) & [decoder](./amrwb_dec/)
- [x] Opus: [Opus Interactive Audio Codec](./opus/)
- [ ] LC3plus: TODO
- [ ] SpeexDSP: TODO
- [ ] Porcupine: TODO

## License

Nuclei Audio Library is licensed under Apache-2.0.