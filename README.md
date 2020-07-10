**UPDATE 20200710:**

Seems NV stopped public NDA developer registration, maybe let's make the project as sweet memory during my software development life. :flushed:

If you are a poor old guy just like me, and still keeping R36x(R37x?) NDA version of nvapi library, welcome build a toy for fun. :+1:

## Win32 and .NET wrapper for NVDIA GPU Utility

The utility supports basic adjustments of NVIDIA GPUs likes frequencies, voltages, thermal efficiency and power limitation, included projects represent [NVAPI](https://developer.nvidia.com/nvapi) and [NVML](https://developer.nvidia.com/nvidia-management-library-nvml) functions by traditional Windows dll and .NET libraries on Win32 platform.

Make sure you know what influence on GPU before using utility, ignorance may cause damage to your graphic card or system. **This utility will NOT guarantee anything included your whimsical mind**

## Getting Start

First of all, [NVIDIA developer](https://developer.nvidia.com/) account should be registered and downloaded NVAPI SDK and CUDA toolkit. The NVML has been a part of deployment kit as a part of CUDA toolkit.

Most of NVAPI features are only available in **NDA edition**, ~~you may need to complete [NDA request form](https://developer.nvidia.com/content/nvapi_request) for the NDA specific version.~~

Unzip NVAPI SDK and install CUDA toolkit.

A visual studio c++ expert, may using environment variables of these SDK's installation path in additional directories of c++ project property.

Or a lazy guy just like me, copy files which needed in following:

 - *NVSDK_PATH*\nvapi.h -> *REPO_PATH*\nvapihelper\nvapi.h
 - *NVSDK_PATH>*\amd64\nvapi64.lib -> *REPO_PATH*\nvapihelper\lib64\nvapi64.lib
 - *CUDA_PATH*\development\include\nvml.h -> *REPO_PATH*\nvapihelper\nvml.h
 - *CUDA_PATH*\development\lib\x64\nvml.lib -> *REPO_PATH*\nvapihelper\nvml.lib

*These projects do not support x86 platform setting because NVML function depends on installed driver, and NVIDIA does not support 32-bit GPU deployment kit.*

Open VS2017(recommanded) with NvGpuUtility.sln or other version with its own sln file.

After building binary files, NVML function required dynamic library which installed from GPU driver in runtime, usually located at *PROGRAM_FILES*\NVIDIA Corporation\NVSMI\nvml.dll, copy to output directory with nvapihelper.dll.
