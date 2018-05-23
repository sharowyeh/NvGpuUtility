## Win32 and .NET wrapper for NVDIA GPU Utility

The utility support basic adjustments for NVIDIA GPU likes frequencies, voltages, thermal efficiency and power limitation, included projects represent [NVAPI](https://developer.nvidia.com/nvapi) and [NVML](https://developer.nvidia.com/nvidia-management-library-nvml) function as traditional Win32 and .NET libraries.

Make sure you know what influence on GPU before using utility, ignorance may cause damage to your graphic card or system. **This utility will NOT guarantee anything includes whimsical mind**

## Getting Start

First of all, must registered [NVIDIA developer](https://developer.nvidia.com/) account then downloaded NVAPI SDK and CUDA toolkit, because NVML has been merged a part of deployment kit also as a part of CUDA toolkit

Unzip NVAPI SDK and install CUDA toolkit.

A visual studio c++ expert, may checking or making environment variables of these SDK's installation path, for later using in additional directories of nvapihelper project property.

Or a lazy guy just like me, copy files which needs in following:

 - *NVSDK_PATH*\nvapi.h -> *REPO_PATH*\nvapihelper\nvapi.h
 - *NVSDK_PATH>*\amd64\nvapi64.lib -> *REPO_PATH*\nvapihelper\lib64\nvapi64.lib
 - *CUDA_PATH*\development\include\nvml.h -> *REPO_PATH*\nvapihelper\nvml.h
 - *CUDA_PATH*\development\lib\x64\nvml.lib -> *REPO_PATH*\nvapihelper\nvml.lib

*Projects do not support x86 platform setting because NVML function depends on installed driver, and NVIDIA does not support 32-bit of GPU deployment kit either.*

Open VS2017(recommanded) with NvGpuUtility.sln or other version with its sln file

After building binary files, NVML needs dynamic library located at *PROGRAM_FILES*\NVIDIA Corporation\NVSMI\nvml.dll, copy to output directory with nvapihelper.dll.
