#pragma once
#include "stdafx.h"

extern "C"
{
	__declspec(dllexport) BOOL Initialize();
	__declspec(dllexport) VOID UnInitialize();

	__declspec(dllexport) NvAPI_Status GetGpuCountAndPrimaryIndex(NvU32 *pdwPhysicalGpuCount, NvU32 *pdwPrimaryGpuIndex);
	__declspec(dllexport) NvAPI_Status GetGpuInformation(NvU32 dwGpuIndex, NvAPI_ShortString szFullName, NvAPI_ShortString szBiosVersion, NvU32 *pdwDeviceId, NvU32 *pdwSubSystemId);
	__declspec(dllexport) NvAPI_Status GetCurrentClockFrequencies(NvU32 dwGpuIndex, NvU32 *pdwCoreClock, NvU32 *pdwMemoryClock);
	__declspec(dllexport) NvAPI_Status GetBaseClockFrequencies(NvU32 dwGpuIndex, NvU32 *pdwCoreClock, NvU32 *pdwMemoryClock);
	__declspec(dllexport) NvAPI_Status GetBoostClockFrequencies(NvU32 dwGpuIndex, NvU32 *pdwCoreClock, NvU32 *pdwMemoryClock);
	__declspec(dllexport) NvAPI_Status GetCurrentPState(NvU32 dwGpuIndex, unsigned short *pwCurrentPState);
	__declspec(dllexport) NvAPI_Status GetPStateLimits(NvU32 dwGpuIndex, unsigned short *pwHardwareLimitPState, unsigned short *pwSoftwareLimitPState);
	__declspec(dllexport) NvAPI_Status GetPStatesInfo(NvU32 dwGpuIndex, bool *isInfoEditable, bool *isPStateEditable, bool *isGpuFreqEditable, bool *isMemFreqEditable, bool *isBaseVoltEditable, bool *isOverVoltEditable);
	__declspec(dllexport) NvAPI_Status GetPStatesInfoGpuFrequencyDelta(NvU32 dwGpuIndex, NvS32 *pdwGpuFreqDelta, unsigned short *pwEditablePState);
	__declspec(dllexport) NvAPI_Status SetPStatesInfoGpuFrequencyDelta(NvU32 dwGpuIndex, NvS32 dwGpuFreqDelta);
	__declspec(dllexport) NvAPI_Status GetPStatesInfoMemFrequencyDelta(NvU32 dwGpuIndex, NvS32 *pdwMemFreqDelta, unsigned short *pwEditablePState);
	__declspec(dllexport) NvAPI_Status SetPStatesInfoMemFrequencyDelta(NvU32 dwGpuIndex, NvS32 dwMemFreqDelta);
	__declspec(dllexport) NvAPI_Status GetPStatesInfoBaseVoltageDelta(NvU32 dwGpuIndex, NvS32 *pdwBaseVoltDelta, unsigned short *pwEditablePState);
	__declspec(dllexport) NvAPI_Status SetPStatesInfoBaseVoltageDelta(NvU32 dwGpuIndex, NvS32 dwBaseVoltDelta);
	__declspec(dllexport) NvAPI_Status GetPStatesInfoOverVoltageDelta(NvU32 dwGpuIndex, NvS32 *pdwOverVoltDelta);
	__declspec(dllexport) NvAPI_Status SetPStatesInfoOverVoltageDelta(NvU32 dwGpuIndex, NvS32 dwOverVoltDelta);
	__declspec(dllexport) NvAPI_Status GetDynamicPStatesInfo(NvU32 dwGpuIndex, NvU32 *pdwGpuUtilization, NvU32 *pdwFameBufferUtilization);
	__declspec(dllexport) NvAPI_Status GetThermalSettings(NvU32 dwGpuIndex, NvU32 dwSensorIndex, NvS32 *pdwGpuCurrentTemperature);
	__declspec(dllexport) NvAPI_Status GetTachometerReading(NvU32 dwGpuIndex, NvU32 *pdwTachometerValue);
	__declspec(dllexport) NvAPI_Status GetCoolerSettings(NvU32 dwGpuIndex, NvU32 dwCoolerIndex, NvU32 *pdwCoolerCurrentLevel, NvU32 *pdwTachometerValue);
	__declspec(dllexport) NvAPI_Status SetCoolerLevels(NvU32 dwGpuIndex, NvU32 dwCoolerIndex, bool bIsManual, NvU32 dwCoolerLevel);
	__declspec(dllexport) NvAPI_Status QueryIlluminationSupport(NvU32 dwGpuIndex, bool *pbIsSupportLogo, bool *pbIsSupportSli);
	__declspec(dllexport) NvAPI_Status GetIllumination(NvU32 dwGpuIndex, NvU32 dwAttribute, NvU32 *pdwValue);
	__declspec(dllexport) NvAPI_Status SetIllumination(NvU32 dwGpuIndex, NvU32 dwAttribute, NvU32 dwValue);

	__declspec(dllexport) nvmlReturn_t GetDevicePowerLimit(NvU32 dwGpuIndex, unsigned int *pdwPowerLimit, unsigned int *pdwDefaultLimit, unsigned int *pdwEnforcedLimit);
	__declspec(dllexport) nvmlReturn_t SetDevicePowerLimit(NvU32 dwGpuIndex, unsigned int dwPowerLimit);
	__declspec(dllexport) nvmlReturn_t GetProcessUtilization(NvU32 dwGpuIndex, unsigned int *pdwProcessCount, nvmlProcessUtilizationSample_t *pstSample);

}