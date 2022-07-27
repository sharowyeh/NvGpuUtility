#include "stdafx.h"
#include "nvapihelper.h"
#include "logger.h"
#include <string>

struct NvGpuStatus
{
	NvAPI_ShortString szFullName;
	NvAPI_ShortString szBiosVersion;
	NvU32 dwDeviceId = 0;
	NvU32 dwSubSystemId = 0;
	NvU32 dwCurrentCoreClock = 0;
	NvU32 dwCurrentMemoryClock = 0;
	NvU32 dwBaseCoreClock = 0;
	NvU32 dwBaseMemoryClock = 0;
	NvU32 dwBoostCoreClock = 0;
	NvU32 dwBoostMemoryClock = 0;
	NvU32 dwGpuUtilization = 0;
	NvU32 dwFrameBufferUtilization = 0;
	NvS32 dwGpuCurrentTemperature = 0;
	NvU32 dwTachometerValue = 0;
	NvU32 dwCoolerCurrentLevel = 0;
	NvU32 dwCoolerMinLevel = 0;
	NvU32 dwCoolerMaxLevel = 0;
};

struct NvApiData
{
	bool bIsNvApiInitialized = false;
	NvPhysicalGpuHandle phPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS];
	NvU32 dwPhysicalGpuCount = 0;
	NvU32 dwPrimaryGpuIndex = 0;
	NvGpuStatus pstGpuStatus[NVAPI_MAX_PHYSICAL_GPUS];
	NV_GPU_PERF_PSTATES20_INFO pstPStatesInfo[NVAPI_MAX_PHYSICAL_GPUS];
	bool bIsNvMLInitialized = false;
	nvmlDevice_t pstGpuDevice[NVAPI_MAX_PHYSICAL_GPUS];
	nvmlProcessUtilizationSample_t *pstUtilization;
};

NvApiData* pData = new NvApiData();

__declspec(dllexport) BOOL Initialize()
{
	BOOL bResult = FALSE;
	
	if (pData->bIsNvApiInitialized == false)
	{
		NvAPI_Status status = NvAPI_Initialize();
		if (status == NVAPI_OK)
		{
			pData->bIsNvApiInitialized = true;
			for (NvU32 index = 0; index < NVAPI_MAX_PHYSICAL_GPUS; index++)
			{
				pData->phPhysicalGpu[index] = NULL;
				pData->pstPStatesInfo[index] = { 0, };
			}
			pData->pstGpuStatus;
		}
		
		LOGFILE("status:%d NvAPI_Initialize --->\n", status);
	}

	// Initialize management library and handle array match with NvAPI
	if (pData->bIsNvMLInitialized == false)
	{
		nvmlReturn_t result = nvmlInit();
		if (result == NVML_SUCCESS)
		{
			pData->bIsNvMLInitialized = true;
			for (NvU32 index = 0; index < NVAPI_MAX_PHYSICAL_GPUS; index++)
			{
				pData->pstGpuDevice[index] = NULL;
			}
			// Reserved size of process utilization sample list
			pData->pstUtilization = (nvmlProcessUtilizationSample_t*)malloc(100 * sizeof(nvmlProcessUtilizationSample_t));
		}
		LOGFILE("result:%d nvmlInit --->\n", result);
	}

	if (pData->bIsNvApiInitialized && pData->bIsNvMLInitialized)
		bResult = TRUE;

	return bResult;
}

__declspec(dllexport) VOID UnInitialize()
{
	if (pData->bIsNvApiInitialized)
	{
		NvAPI_Status status = NvAPI_Unload();
		if (status == NVAPI_OK)
			pData->bIsNvApiInitialized = false;
			
		LOGFILE("status:%d NvAPI_Unload <---\n", status);
	}

	if (pData->bIsNvMLInitialized)
	{
		nvmlReturn_t result = nvmlShutdown();
		if (result == NVML_SUCCESS)
			pData->bIsNvMLInitialized = false;

		LOGFILE("result:%d nvmlShutdown <---\n", result);
	}

	delete pData;
}

// Forward definition
nvmlReturn_t GetNvMLDeviceHandle(NvU32 dwGpuIndex);

// Enumated all GPU handle before do anything
NvAPI_Status EnumAllPhysicalGpuHandle()
{
	NvAPI_Status status = NVAPI_OK;
	NvU32 dwDisplayIds = 0;
	NV_GPU_DISPLAYIDS* pDisplayIds = NULL;
	
	// Enumerate the physical GPU handle
	status = NvAPI_EnumPhysicalGPUs(pData->phPhysicalGpu, &pData->dwPhysicalGpuCount);
	LOGFILE("status:%d NvAPI_EnumPhysicalGPUs list:0x%p count:%d\n", status, pData->phPhysicalGpu, pData->dwPhysicalGpuCount);
	if (status == NVAPI_OK)
	{
		// get the display ids of connected displays
		for (NvU32 index = 0; index < pData->dwPhysicalGpuCount; index++)
		{
			status = NvAPI_GPU_GetConnectedDisplayIds(pData->phPhysicalGpu[index], pDisplayIds, &dwDisplayIds, 0);
			LOGFILE(" status:%d index:%d NvAPI_GPU_GetConnectedDisplayIds handle:0x%x display:%d\n", status, index, pData->phPhysicalGpu[index], dwDisplayIds);
			if (status == NVAPI_OK && dwDisplayIds)
			{
				pData->dwPrimaryGpuIndex = index;
				//break;
			}
			// get nvml device handle
			nvmlReturn_t result = GetNvMLDeviceHandle(index);
		}
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetGpuCountAndPrimaryIndex(NvU32 *pdwPhysicalGpuCount, NvU32 *pdwPrimaryGpuIndex)
{
	NvAPI_Status status = NVAPI_OK;
	
	status = EnumAllPhysicalGpuHandle();
	if (status == NVAPI_OK)
	{
		*pdwPhysicalGpuCount = pData->dwPhysicalGpuCount;
		*pdwPrimaryGpuIndex = pData->dwPrimaryGpuIndex;
	}
	else
	{
		*pdwPhysicalGpuCount = 0;
		*pdwPrimaryGpuIndex = 0;
	}

	return status;
}

NvAPI_Status CheckPhysicalGpuExists(NvU32 *pdwGpuIndex)
{
	NvAPI_Status status = NVAPI_OK;
	if (pData->phPhysicalGpu[*pdwGpuIndex] == NULL)
		*pdwGpuIndex = pData->dwPrimaryGpuIndex;
	if (pData->phPhysicalGpu[*pdwGpuIndex] == NULL)
	{
		status = EnumAllPhysicalGpuHandle();
	}
	return status;
}

__declspec(dllexport) NvAPI_Status GetGpuInformation(NvU32 dwGpuIndex, NvAPI_ShortString szFullName, NvAPI_ShortString szBiosVersion, NvU32 *pdwDeviceId, NvU32 *pdwSubSystemId)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		szFullName = "";
		szBiosVersion = "";
		*pdwDeviceId = 0;
		*pdwSubSystemId = 0;
		return status;
	}

	LOGFILE("Get GPU info index:%d handle:0x%x\n", dwGpuIndex, pData->phPhysicalGpu[dwGpuIndex]);
	status = NvAPI_GPU_GetFullName(pData->phPhysicalGpu[dwGpuIndex], pData->pstGpuStatus[dwGpuIndex].szFullName);
	LOGFILE("status:%d NvAPI_GPU_GetFullName %s\n", status, pData->pstGpuStatus[dwGpuIndex].szFullName);
	if (status == NVAPI_OK)
		strcpy_s(szFullName, sizeof(NvAPI_ShortString), pData->pstGpuStatus[dwGpuIndex].szFullName);
	else
		szFullName = "";

	status = NvAPI_GPU_GetVbiosVersionString(pData->phPhysicalGpu[dwGpuIndex], pData->pstGpuStatus[dwGpuIndex].szBiosVersion);
	LOGFILE("status:%d NvAPI_GPU_GetVbiosVersionString %s\n", status, pData->pstGpuStatus[dwGpuIndex].szBiosVersion);
	if (status == NVAPI_OK)
		strcpy_s(szBiosVersion, sizeof(NvAPI_ShortString), pData->pstGpuStatus[dwGpuIndex].szBiosVersion);
	else
		szBiosVersion = "";

	NvU32 reversion = 0;
	NvU32 extid = 0;
	NvAPI_GPU_GetPCIIdentifiers(pData->phPhysicalGpu[dwGpuIndex], &pData->pstGpuStatus[dwGpuIndex].dwDeviceId, &pData->pstGpuStatus[dwGpuIndex].dwSubSystemId, &reversion, &extid);
	LOGFILE("status:%d NvAPI_GPU_GetPCIIdentifiers device:%x subsys:%x rev:%x ext:%x\n", status, pData->pstGpuStatus[dwGpuIndex].dwDeviceId, pData->pstGpuStatus[dwGpuIndex].dwSubSystemId, reversion, extid);
	if (status == NVAPI_OK)
	{
		*pdwDeviceId = pData->pstGpuStatus[dwGpuIndex].dwDeviceId;
		*pdwSubSystemId = pData->pstGpuStatus[dwGpuIndex].dwSubSystemId;
	}
	else
	{
		*pdwDeviceId = 0;
		*pdwSubSystemId = 0;
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetCurrentClockFrequencies(NvU32 dwGpuIndex, NvU32 *pdwCoreClock, NvU32 *pdwMemoryClock)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwCoreClock = 0;
		*pdwMemoryClock = 0;
		return status;
	}

	NV_GPU_CLOCK_FREQUENCIES freqs = { 0, };
	freqs.version = NV_GPU_CLOCK_FREQUENCIES_VER;
	freqs.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;

	status = NvAPI_GPU_GetAllClockFrequencies(pData->phPhysicalGpu[dwGpuIndex], &freqs);
	LOGFILE("status:%d NvAPI_GPU_GetAllClockFrequencies freqs:0x%p type:%d(current)\n", status, freqs, freqs.ClockType);
	if (status == NVAPI_OK)
	{
		LOGFILE(" domain:%d present:%d frequency:%dKHz\n", NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency);
		if (freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwCurrentCoreClock = freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
		else
			pData->pstGpuStatus[dwGpuIndex].dwCurrentCoreClock = 0;

		LOGFILE(" domain:%d present:%d frequency:%dKHz\n", NVAPI_GPU_PUBLIC_CLOCK_MEMORY, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency);
		if (freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwCurrentMemoryClock = freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency;
		else
			pData->pstGpuStatus[dwGpuIndex].dwCurrentMemoryClock = 0;

		*pdwCoreClock = pData->pstGpuStatus[dwGpuIndex].dwCurrentCoreClock;
		*pdwMemoryClock = pData->pstGpuStatus[dwGpuIndex].dwCurrentMemoryClock;
	}
	else
	{
		*pdwCoreClock = 0;
		*pdwMemoryClock = 0;
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetBaseClockFrequencies(NvU32 dwGpuIndex, NvU32 *pdwCoreClock, NvU32 *pdwMemoryClock)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwCoreClock = 0;
		*pdwMemoryClock = 0;
		return status;
	}

	NV_GPU_CLOCK_FREQUENCIES freqs = { 0, };
	freqs.version = NV_GPU_CLOCK_FREQUENCIES_VER;
	freqs.ClockType = NV_GPU_CLOCK_FREQUENCIES_BASE_CLOCK;

	status = NvAPI_GPU_GetAllClockFrequencies(pData->phPhysicalGpu[dwGpuIndex], &freqs);
	LOGFILE("status:%d NvAPI_GPU_GetAllClockFrequencies freqs:0x%p type:%d(base)\n", status, freqs, freqs.ClockType);
	if (status == NVAPI_OK)
	{
		LOGFILE(" domain:%d present:%d frequency:%dKHz\n", NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency);
		if (freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwBaseCoreClock = freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
		else
			pData->pstGpuStatus[dwGpuIndex].dwBaseCoreClock = 0;

		LOGFILE(" domain:%d present:%d frequency:%dKHz\n", NVAPI_GPU_PUBLIC_CLOCK_MEMORY, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency);
		if (freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwBaseMemoryClock = freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency;
		else
			pData->pstGpuStatus[dwGpuIndex].dwBaseMemoryClock = 0;

		*pdwCoreClock = pData->pstGpuStatus[dwGpuIndex].dwBaseCoreClock;
		*pdwMemoryClock = pData->pstGpuStatus[dwGpuIndex].dwBaseMemoryClock;
	}
	else
	{
		*pdwCoreClock = 0;
		*pdwMemoryClock = 0;
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetBoostClockFrequencies(NvU32 dwGpuIndex, NvU32 *pdwCoreClock, NvU32 *pdwMemoryClock)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwCoreClock = 0;
		*pdwMemoryClock = 0;
		return status;
	}

	NV_GPU_CLOCK_FREQUENCIES freqs = { 0, };
	freqs.version = NV_GPU_CLOCK_FREQUENCIES_VER;
	freqs.ClockType = NV_GPU_CLOCK_FREQUENCIES_BOOST_CLOCK;

	status = NvAPI_GPU_GetAllClockFrequencies(pData->phPhysicalGpu[dwGpuIndex], &freqs);
	LOGFILE("status:%d NvAPI_GPU_GetAllClockFrequencies freqs:0x%p type:%d(boost)\n", status, freqs, freqs.ClockType);
	if (status == NVAPI_OK)
	{
		LOGFILE(" domain:%d present:%d frequency:%dKHz\n", NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency);
		if (freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwBoostCoreClock = freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
		else
			pData->pstGpuStatus[dwGpuIndex].dwBoostCoreClock = 0;

		LOGFILE(" domain:%d present:%d frequency:%dKHz\n", NVAPI_GPU_PUBLIC_CLOCK_MEMORY, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent, freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency);
		if (freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwBoostMemoryClock = freqs.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency;
		else
			pData->pstGpuStatus[dwGpuIndex].dwBoostMemoryClock = 0;

		*pdwCoreClock = pData->pstGpuStatus[dwGpuIndex].dwBoostCoreClock;
		*pdwMemoryClock = pData->pstGpuStatus[dwGpuIndex].dwBoostMemoryClock;
	}
	else
	{
		*pdwCoreClock = 0;
		*pdwMemoryClock = 0;
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetCurrentPState(NvU32 dwGpuIndex, unsigned short *pwCurrentPState)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pwCurrentPState = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_UNDEFINED;
		return status;
	}

	NV_GPU_PERF_PSTATE_ID pstate;
	status = NvAPI_GPU_GetCurrentPstate(pData->phPhysicalGpu[dwGpuIndex], &pstate);
	LOGFILE("status:%d NvAPI_GPU_GetCurrentPstate pstate:%d\n", status, pstate);
	if (status == NVAPI_OK)
	{
		*pwCurrentPState = pstate;
	}
	else
	{
		*pwCurrentPState = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_UNDEFINED;
	}
	

	/// NvAPI_GPU_GetPerfClocks can not get information which really care about
	/*NV_GPU_PERF_CLOCK_TABLE table = { 0, };
	table.version = NV_GPU_PERF_CLOCK_TABLE_VER;
	status = NvAPI_GPU_GetPerfClocks(pData->phPhysicalGpu[dwGpuIndex], 0, &table);
	LOGFILE("status:%d NvAPI_GPU_GetPerfClocks numLevel:%d numDomain:%d perfFlag:%d\n", status, table.levelCount, table.domainCount, table.gpuPerfFlags);
	if (status == NVAPI_OK)
	{
		for (int i = 0; i < table.levelCount; i++)
		{
			if (table.perfLevel[i].level != i)
				break;
			LOGFILE(" index:%d level:%d flag:%d\n", i, table.perfLevel[i].level, table.perfLevel[i].flags);
			for (int j = 0; j < table.domainCount; j++)
			{
				LOGFILE("  index:%d domain:%d flag:%d isset:%d defFreq:%d curFreq:%d minFreq:%d maxFreq:%d\n", j, table.perfLevel[i].domain[j].domainId, table.perfLevel[i].domain[j].domainFlags, table.perfLevel[i].domain[j].bSetClock, table.perfLevel[i].domain[j].defaultFreq, table.perfLevel[i].domain[j].currentFreq, table.perfLevel[i].domain[j].minFreq, table.perfLevel[i].domain[j].maxFreq);
			}
		}
	}*/

	//NvAPI_GPU_SetPerfClocks(pData->phPhysicalGpu[dwGpuIndex], 0, &table);

	return status;
}

__declspec(dllexport) NvAPI_Status GetPStateLimits(NvU32 dwGpuIndex, unsigned short *pwHardwareLimitPState, unsigned short *pwSoftwareLimitPState)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		return status;
	}

	NV_GPU_PERF_PSTATE_ID limit = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_UNDEFINED;
	status = NvAPI_GPU_GetPstateClientLimits(pData->phPhysicalGpu[dwGpuIndex], NV_GPU_PERF_PSTATE_CLIENT_LIMIT_ID::NVAPI_PERF_PSTATE_CLIENT_LIMIT_HARD, &limit);
	LOGFILE("status:%d NvAPI_GPU_GetPstateClientLimits hardlimit:%d\n", status, limit);
	*pwHardwareLimitPState = limit;
	
	limit = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_UNDEFINED;
	status = NvAPI_GPU_GetPstateClientLimits(pData->phPhysicalGpu[dwGpuIndex], NV_GPU_PERF_PSTATE_CLIENT_LIMIT_ID::NVAPI_PERF_PSTATE_CLIENT_LIMIT_SOFT, &limit);
	LOGFILE("status:%d NvAPI_GPU_GetPstateClientLimits softlimit:%d\n", status, limit);
	*pwSoftwareLimitPState = limit;
	
	return status;
}

/// Overload function that we don't needs return parameters
NvAPI_Status GetPStatesInfo(NvU32 dwGpuIndex)
{
	bool isInfoEditable;
	bool isPStateEditable;
	bool isGpuFreqEditable;
	bool isMemFreqEditable;
	bool isBaseVoltEditable;
	bool isOverVoltEditable;
	return GetPStatesInfo(dwGpuIndex, &isInfoEditable, &isPStateEditable, &isGpuFreqEditable, &isMemFreqEditable, &isBaseVoltEditable, &isOverVoltEditable);
}

__declspec(dllexport) NvAPI_Status GetPStatesInfo(NvU32 dwGpuIndex, bool *isInfoEditable, bool *isPStateEditable, bool *isGpuFreqEditable, bool *isMemFreqEditable, bool *isBaseVoltEditable, bool *isOverVoltEditable)
{
	*isInfoEditable = false;
	*isGpuFreqEditable = false;
	*isMemFreqEditable = false;
	*isBaseVoltEditable = false;
	*isOverVoltEditable = false;

	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	/// Reset and ready to update our saved info
	pData->pstPStatesInfo[dwGpuIndex] = { 0, };

	NV_GPU_PERF_PSTATES20_INFO info = { 0, };
	info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
	status = NvAPI_GPU_GetPstates20(pData->phPhysicalGpu[dwGpuIndex], &info);
	LOGFILE("status:%d NvAPI_GPU_GetPstates20 editable:%d numPstate:%d numClock:%d numVoltage:%d numOV:%d\n", status, info.bIsEditable, info.numPstates, info.numClocks, info.numBaseVoltages, info.ov.numVoltages);
	if (status == NVAPI_OK)
	{
		/// Save to our list
		pData->pstPStatesInfo[dwGpuIndex] = info;
		if (info.bIsEditable == 1)
			*isInfoEditable = true;
		
		/// pstates
		for (size_t i = 0; i < info.numPstates; i++)
		{
			LOGFILE(" index:%d pstate:%d editable:%d\n", i, info.pstates[i].pstateId, info.pstates[i].bIsEditable);
			if (info.pstates[i].bIsEditable == 1)
				*isPStateEditable = true;
			/// clocks
			for (size_t j = 0; j < info.numClocks; j++)
			{
				LOGFILE("  clocks:%d domain:%d editable:%d freqDelta:%d[min:%d max:%d] ", j, info.pstates[i].clocks[j].domainId, info.pstates[i].clocks[j].bIsEditable, info.pstates[i].clocks[j].freqDelta_kHz.value, info.pstates[i].clocks[j].freqDelta_kHz.valueRange.min, info.pstates[i].clocks[j].freqDelta_kHz.valueRange.max);
				if (info.pstates[i].clocks[j].bIsEditable == 1)
				{
					if (info.pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS)
						*isGpuFreqEditable = true;
					if (info.pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_MEMORY)
						*isMemFreqEditable = true;
				}
				if (info.pstates[i].clocks[j].typeId == NV_GPU_PERF_PSTATE20_CLOCK_TYPE_ID::NVAPI_GPU_PERF_PSTATE20_CLOCK_TYPE_SINGLE)
				{
					LOGFILE("type:%d(single) freq:%d\n", info.pstates[i].clocks[j].typeId, info.pstates[i].clocks[j].data.single.freq_kHz);
				}
				else if (info.pstates[i].clocks[j].typeId == NV_GPU_PERF_PSTATE20_CLOCK_TYPE_ID::NVAPI_GPU_PERF_PSTATE20_CLOCK_TYPE_RANGE)
				{
					LOGFILE("type:%d(range) freq:[min:%d max:%d]\n", info.pstates[i].clocks[j].typeId, info.pstates[i].clocks[j].data.range.minFreq_kHz, info.pstates[i].clocks[j].data.range.maxFreq_kHz);
				}
			}
			/// baseVoltages
			for (size_t j = 0; j < info.numBaseVoltages; j++)
			{
				LOGFILE("  voltages:%d domain:%d editable:%d curVolt:%d deltaVolt:%d[min:%d max:%d]\n", j, info.pstates[i].baseVoltages[j].domainId, info.pstates[i].baseVoltages[j].bIsEditable, info.pstates[i].baseVoltages[j].volt_uV, info.pstates[i].baseVoltages[j].voltDelta_uV.value, info.pstates[i].baseVoltages[j].voltDelta_uV.valueRange.min, info.pstates[i].baseVoltages[j].voltDelta_uV.valueRange.max);
				if (info.pstates[i].baseVoltages[j].bIsEditable == 1)
					*isBaseVoltEditable = true;
			}
		}

		/// ov
		for (size_t i = 0; i < info.ov.numVoltages; i++)
		{
			LOGFILE(" index:%d ov: domain:%d editable:%d curVolt:%d deltaVolt:%d[min:%d max:%d]\n", i, info.ov.voltages[i].domainId, info.ov.voltages[i].bIsEditable, info.ov.voltages[i].volt_uV, info.ov.voltages[i].voltDelta_uV.value, info.ov.voltages[i].voltDelta_uV.valueRange.min, info.ov.voltages[i].voltDelta_uV.valueRange.max);
			if (info.ov.voltages[i].bIsEditable == 1)
				*isOverVoltEditable = true;
		}
	}

	return status;
}

// Check PStates info is exists and editable 
NvAPI_Status CheckPStatesInfoExists(NvU32 *pdwGpuIndex)
{
	NvAPI_Status status = CheckPhysicalGpuExists(pdwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	/// Get PStates info if it's never get before
	LOGFILE("Check pstates info initialized: numpstates:%d\n", pData->pstPStatesInfo[*pdwGpuIndex].numPstates);
	if (pData->pstPStatesInfo[*pdwGpuIndex].numPstates == 0)
	{
		status = GetPStatesInfo(*pdwGpuIndex);
		if (status != NVAPI_OK)
			return status;
	}

	/// Check PStatesInfo is editable or/and check sperated sub editable ability by each given parameters.
	LOGFILE("Check pstates info ability: editable:%d\n", pData->pstPStatesInfo[*pdwGpuIndex].bIsEditable);
	if (pData->pstPStatesInfo[*pdwGpuIndex].bIsEditable == 0)
	{
		return NVAPI_NOT_SUPPORTED;
	}
	return status;
}

// It can get editable frequency delta aftering GetPStatesInfo fill pData->allGpusPStatesInfo[dwGpuIndex]
__declspec(dllexport) NvAPI_Status GetPStatesInfoGpuFrequencyDelta(NvU32 dwGpuIndex, NvS32 *pdwGpuFreqDelta, unsigned short *pwEditablePState)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;
	
	*pdwGpuFreqDelta = 0;
	*pwEditablePState = 0;

	// Only fill out parameters if PState is editable and frequency of PState is editable and maximum of all editable PStates
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].numPstates; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].bIsEditable == 1)
		{
			for (size_t j = 0; j < pData->pstPStatesInfo[dwGpuIndex].numClocks; j++)
			{
				if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].bIsEditable == 1 &&
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS)
				{
					if (*pdwGpuFreqDelta < pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz.value)
					{
						*pdwGpuFreqDelta = pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz.value;
						*pwEditablePState = pData->pstPStatesInfo[dwGpuIndex].pstates[i].pstateId;
					}
				}
			}
		}
	}

	return status;
}

__declspec(dllexport) NvAPI_Status SetPStatesInfoGpuFrequencyDelta(NvU32 dwGpuIndex, NvS32 dwGpuFreqDelta)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;
	
	NV_GPU_PERF_PSTATES20_INFO info = { 0, };
	info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
	info.bIsEditable = 1;
	NvU32 pstateIndex = 0;
	NvU32 clockIndex = 0;
	/// Build given pstates info depends on exists pstates info ability
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].numPstates; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].bIsEditable == 1)
		{
			clockIndex = 0;
			for (size_t j = 0; j < pData->pstPStatesInfo[dwGpuIndex].numClocks; j++)
			{
				if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].bIsEditable == 1 &&
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS)
				{
					info.pstates[pstateIndex].bIsEditable = 1;
					info.pstates[pstateIndex].pstateId = pData->pstPStatesInfo[dwGpuIndex].pstates[i].pstateId;
					info.pstates[pstateIndex].clocks[clockIndex].bIsEditable = 1;
					info.pstates[pstateIndex].clocks[clockIndex].domainId = NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS;
					info.pstates[pstateIndex].clocks[clockIndex].freqDelta_kHz.value = dwGpuFreqDelta;
					//LOGFILE("Build info:%d pstate:%d clock:%d domain:%d delta:%d\n", pstateIndex, info.pstates[pstateIndex].pstateId, clockIndex, info.pstates[pstateIndex].clocks[clockIndex].domainId, info.pstates[pstateIndex].clocks[clockIndex].freqDelta_kHz.value);
					/// Increase editable pstates count within editable GPU clock
					pstateIndex++;
					/// Save to numbers of pstates
					info.numPstates = pstateIndex;
					/// Increase editable clocks count but it always will be 1 because clocks list only has one GPU domain per one pstate
					clockIndex++;
					/// Save to numbers of clocks if largest
					if (info.numClocks < clockIndex)
						info.numClocks = clockIndex;
				}
			}
		}
	}
	
	/*info.numPstates = 1;
	info.numClocks = 1;
	info.pstates[0].bIsEditable = 1;
	info.pstates[0].pstateId = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_P0;
	info.pstates[0].clocks[0].bIsEditable = 1;
	info.pstates[0].clocks[0].domainId = NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS;
	info.pstates[0].clocks[0].freqDelta_kHz.value = dwGpuFreqDelta;*/
	
	status = NvAPI_GPU_SetPstates20(pData->phPhysicalGpu[dwGpuIndex], &info);
	LOGFILE("status:%d NvAPI_GPU_SetPstates20 pstates:%d clocks:%d freqDelta:%d\n", status, info.numPstates, info.numClocks, info.pstates[0].clocks[0].freqDelta_kHz.value);
	if (status == NVAPI_OK)
	{
		/// Overwrite save to our list
		for (size_t i = 0; i < info.numPstates; i++)
		{
			for (size_t j = 0; j < info.numClocks; j++)
			{
				if (info.pstates[i].clocks[j].bIsEditable &&
					info.pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS)
				{
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz = info.pstates[i].clocks[j].freqDelta_kHz;
					LOGFILE("Save pstates info GPU freqDelta:%d\n", pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz);
				}
			}
		}
	}

	/// return NVAPI_NOT_SUPPORTED
	//status = NvAPI_GPU_EnableOverclockedPstates(pData->phPhysicalGpu[dwGpuIndex], 1);
	//LOGFILE("status:%d NvAPI_GPU_EnableOverclockedPstates set enabled\n", status);

	/// Only effect on None-OC GPU (don't know why, seems depend on VBIOS)
	//status = NvAPI_GPU_EnableDynamicPstates(pData->phPhysicalGpu[dwGpuIndex], 1);
	//LOGFILE("status:%d NvAPI_GPU_EnableDynamicPstates set disabled\n", status);

	return status;
}

// It can get editable frequency delta aftering GetPStatesInfo fill pData->allGpusPStatesInfo[dwGpuIndex]
__declspec(dllexport) NvAPI_Status GetPStatesInfoMemFrequencyDelta(NvU32 dwGpuIndex, NvS32 *pdwMemFreqDelta, unsigned short *pwEditablePState)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	*pdwMemFreqDelta = 0;
	*pwEditablePState = 0;

	// Only fill out parameters if PState is editable and frequency of PState is editable and maximum of all editable PStates
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].numPstates; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].bIsEditable == 1)
		{
			for (size_t j = 0; j < pData->pstPStatesInfo[dwGpuIndex].numClocks; j++)
			{
				if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].bIsEditable == 1 &&
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_MEMORY)
				{
					if (*pdwMemFreqDelta < pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz.value)
					{
						*pdwMemFreqDelta = pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz.value;
						*pwEditablePState = pData->pstPStatesInfo[dwGpuIndex].pstates[i].pstateId;
					}
				}
			}
		}
	}

	return status;
}

__declspec(dllexport) NvAPI_Status SetPStatesInfoMemFrequencyDelta(NvU32 dwGpuIndex, NvS32 dwMemFreqDelta)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	NV_GPU_PERF_PSTATES20_INFO info = { 0, };
	info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
	info.bIsEditable = 1;
	NvU32 pstateIndex = 0;
	NvU32 clockIndex = 0;
	/// Build given pstates info depends on exists pstates info ability
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].numPstates; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].bIsEditable == 1)
		{
			clockIndex = 0;
			for (size_t j = 0; j < pData->pstPStatesInfo[dwGpuIndex].numClocks; j++)
			{
				if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].bIsEditable == 1 &&
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_MEMORY)
				{
					info.pstates[pstateIndex].bIsEditable = 1;
					info.pstates[pstateIndex].pstateId = pData->pstPStatesInfo[dwGpuIndex].pstates[i].pstateId;
					info.pstates[pstateIndex].clocks[clockIndex].bIsEditable = 1;
					info.pstates[pstateIndex].clocks[clockIndex].domainId = NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_MEMORY;
					info.pstates[pstateIndex].clocks[clockIndex].freqDelta_kHz.value = dwMemFreqDelta;
					//LOGFILE("Build info:%d pstate:%d clock:%d domain:%d delta:%d\n", pstateIndex, info.pstates[pstateIndex].pstateId, clockIndex, info.pstates[pstateIndex].clocks[clockIndex].domainId, info.pstates[pstateIndex].clocks[clockIndex].freqDelta_kHz.value);
					/// Increase editable pstates count within editable memory clock
					pstateIndex++;
					/// Save to numbers of pstates
					info.numPstates = pstateIndex;
					/// Increase editable clocks count but it always will be 1 because clocks list only has one memory domain per one pstate
					clockIndex++;
					/// Save to numbers of clocks if largest
					if (info.numClocks < clockIndex)
						info.numClocks = clockIndex;
				}
			}
		}
	}

	/*info.numPstates = 1;
	info.numClocks = 1;
	info.pstates[0].bIsEditable = 1;
	info.pstates[0].pstateId = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_P0;
	info.pstates[0].clocks[0].bIsEditable = 1;
	info.pstates[0].clocks[0].domainId = NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_MEMORY;
	info.pstates[0].clocks[0].freqDelta_kHz.value = dwMemFreqDelta;*/

	status = NvAPI_GPU_SetPstates20(pData->phPhysicalGpu[dwGpuIndex], &info);
	LOGFILE("status:%d NvAPI_GPU_SetPstates20 pstates:%d clocks:%d freqDelta:%d\n", status, info.numPstates, info.numClocks, info.pstates[0].clocks[0].freqDelta_kHz.value);
	if (status == NVAPI_OK)
	{
		/// Overwrite save to our list
		for (size_t i = 0; i < info.numPstates; i++)
		{
			for (size_t j = 0; j < info.numClocks; j++)
			{
				if (info.pstates[i].clocks[j].bIsEditable &&
					info.pstates[i].clocks[j].domainId == NV_GPU_PUBLIC_CLOCK_ID::NVAPI_GPU_PUBLIC_CLOCK_MEMORY)
				{
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz = info.pstates[i].clocks[j].freqDelta_kHz;
					LOGFILE("Save pstates info memory freqDelta:%d\n", pData->pstPStatesInfo[dwGpuIndex].pstates[i].clocks[j].freqDelta_kHz);
				}
			}
		}
	}

	return status;
}

// It can get editable voltage delta aftering GetPStatesInfo fill pData->allGpusPStatesInfo[dwGpuIndex]
__declspec(dllexport) NvAPI_Status GetPStatesInfoBaseVoltageDelta(NvU32 dwGpuIndex, NvS32 *pdwBaseVoltDelta, unsigned short *pwEditablePState)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	*pdwBaseVoltDelta = 0;
	*pwEditablePState = 0;

	// Only fill out parameters if PState is editable and frequency of PState is editable and maximum of all editable PStates
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].numPstates; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].bIsEditable == 1)
		{
			for (size_t j = 0; j < pData->pstPStatesInfo[dwGpuIndex].numBaseVoltages; j++)
			{
				if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].bIsEditable == 1 &&
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].domainId == NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE)
				{
					if (abs(*pdwBaseVoltDelta) < abs(pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].voltDelta_uV.value))
					{
						*pdwBaseVoltDelta = pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].voltDelta_uV.value;
						*pwEditablePState = pData->pstPStatesInfo[dwGpuIndex].pstates[i].pstateId;
					}
				}
			}
		}
	}

	return status;
}

/// Base voltage increase 6250uV in each steps, and seems like high PState can not lower then low PState (not comfirmed because it only can edit P0 State)
__declspec(dllexport) NvAPI_Status SetPStatesInfoBaseVoltageDelta(NvU32 dwGpuIndex, NvS32 dwBaseVoltDelta)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	NV_GPU_PERF_PSTATES20_INFO info = { 0, };
	info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
	info.bIsEditable = 1;
	NvU32 pstateIndex = 0;
	NvU32 voltIndex = 0;
	/// Build given pstates info depends on exists pstates info ability
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].numPstates; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].bIsEditable == 1)
		{
			voltIndex = 0;
			for (size_t j = 0; j < pData->pstPStatesInfo[dwGpuIndex].numBaseVoltages; j++)
			{
				if (pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].bIsEditable == 1 &&
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].domainId == NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE)
				{
					info.pstates[pstateIndex].bIsEditable = 1;
					info.pstates[pstateIndex].pstateId = pData->pstPStatesInfo[dwGpuIndex].pstates[i].pstateId;
					info.pstates[pstateIndex].baseVoltages[voltIndex].bIsEditable = 1;
					info.pstates[pstateIndex].baseVoltages[voltIndex].domainId = NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE;
					info.pstates[pstateIndex].baseVoltages[voltIndex].voltDelta_uV.value = dwBaseVoltDelta;
					//LOGFILE("Build info:%d pstate:%d baseVolt:%d domain:%d delta:%d\n", pstateIndex, info.pstates[pstateIndex].pstateId, voltIndex, info.pstates[pstateIndex].baseVoltages[voltIndex].domainId, info.pstates[pstateIndex].baseVoltages[voltIndex].voltDelta_uV.value);
					/// Increase editable pstates count within editable base voltage
					pstateIndex++;
					/// Save to numbers of pstates
					info.numPstates = pstateIndex;
					/// Increase editable base voltage count but it always will be 1 because base voltage list only has one voltage domain per one pstate
					voltIndex++;
					/// Save to numbers of base voltages if largest
					if (info.numBaseVoltages < voltIndex)
						info.numBaseVoltages = voltIndex;
				}
			}
		}
	}

	/*info.numPstates = 1;
	info.numClocks = 1;
	info.pstates[0].bIsEditable = 1;
	info.pstates[0].pstateId = NV_GPU_PERF_PSTATE_ID::NVAPI_GPU_PERF_PSTATE_P0;
	info.pstates[0].baseVoltages[0].bIsEditable = 1;
	info.pstates[0].baseVoltages[0].domainId = NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE;
	info.pstates[0].baseVoltages[0].voltDelta_uV.value = dwBaseVoltageDelta;*/

	status = NvAPI_GPU_SetPstates20(pData->phPhysicalGpu[dwGpuIndex], &info);
	LOGFILE("status:%d NvAPI_GPU_SetPstates20 pstates:%d baseVolt:%d voltDelta:%d\n", status, info.numPstates, info.numBaseVoltages, info.pstates[0].baseVoltages[0].voltDelta_uV.value);
	if (status == NVAPI_OK)
	{
		/// Overwrite save to our list
		for (size_t i = 0; i < info.numPstates; i++)
		{
			for (size_t j = 0; j < info.numBaseVoltages; j++)
			{
				if (info.pstates[i].baseVoltages[j].bIsEditable &&
					info.pstates[i].baseVoltages[j].domainId == NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE)
				{
					pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].voltDelta_uV = info.pstates[i].baseVoltages[j].voltDelta_uV;
					LOGFILE("Save pstates info base voltage voltDelta:%d\n", pData->pstPStatesInfo[dwGpuIndex].pstates[i].baseVoltages[j].voltDelta_uV);
				}
			}
		}
	}

	return status;
}

// It can get editable voltage delta aftering GetPStatesInfo fill pData->allGpusPStatesInfo[dwGpuIndex]
__declspec(dllexport) NvAPI_Status GetPStatesInfoOverVoltageDelta(NvU32 dwGpuIndex, NvS32 *pdwOverVoltDelta)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	*pdwOverVoltDelta = 0;

	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].ov.numVoltages; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].bIsEditable == 1 &&
			pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].domainId == NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE)
		{
			*pdwOverVoltDelta = pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].voltDelta_uV.value;
		}
	}

	return status;
}

/// Over voltage only can be plus numbers
__declspec(dllexport) NvAPI_Status SetPStatesInfoOverVoltageDelta(NvU32 dwGpuIndex, NvS32 dwOverVoltDelta)
{
	NvAPI_Status status = CheckPStatesInfoExists(&dwGpuIndex);
	if (status != NVAPI_OK)
		return status;

	NV_GPU_PERF_PSTATES20_INFO info = { 0, };
	info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
	info.bIsEditable = 1;
	NvU32 voltIndex = 0;
	/// Build given over voltage info depends on exists over voltage info ability
	for (size_t i = 0; i < pData->pstPStatesInfo[dwGpuIndex].ov.numVoltages; i++)
	{
		if (pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].bIsEditable == 1 &&
			pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].domainId == NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE)
		{
			info.ov.voltages[voltIndex].bIsEditable = 1;
			info.ov.voltages[voltIndex].domainId = NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE;
			info.ov.voltages[voltIndex].voltDelta_uV.value = dwOverVoltDelta;
			//LOGFILE("Build info:%d pstate:%d baseVolt:%d domain:%d delta:%d\n", pstateIndex, info.pstates[pstateIndex].pstateId, voltIndex, info.pstates[pstateIndex].baseVoltages[voltIndex].domainId, info.pstates[pstateIndex].baseVoltages[voltIndex].voltDelta_uV.value);
			/// Increase editable over voltages count
			voltIndex++;
			/// Save to numbers of pstates
			info.ov.numVoltages = voltIndex;
		}
	}

	status = NvAPI_GPU_SetPstates20(pData->phPhysicalGpu[dwGpuIndex], &info);
	LOGFILE("status:%d NvAPI_GPU_SetPstates20 ovs:%d voltDelta:%d\n", status, info.ov.numVoltages, info.ov.voltages[0].voltDelta_uV.value);
	if (status == NVAPI_OK)
	{
		/// Overwrite save to our list
		for (size_t i = 0; i < info.ov.numVoltages; i++)
		{
			if (info.ov.voltages[i].bIsEditable &&
				info.ov.voltages[i].domainId == NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID::NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE)
			{
				pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].voltDelta_uV = info.ov.voltages[i].voltDelta_uV;
				LOGFILE("Save pstates info over voltage voltDelta:%d\n", pData->pstPStatesInfo[dwGpuIndex].ov.voltages[i].voltDelta_uV);
			}
		}
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetDynamicPStatesInfo(NvU32 dwGpuIndex, NvU32 *pdwGpuUtilization, NvU32 *pdwFameBufferUtilization)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwGpuUtilization = 0;
		*pdwFameBufferUtilization = 0;
		return status;
	}

	//NvU32 present = 0;
	//NvU32 utilization = 0;
	NV_GPU_DYNAMIC_PSTATES_INFO_EX info = { 0, };
	info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;

	status = NvAPI_GPU_GetDynamicPstatesInfoEx(pData->phPhysicalGpu[dwGpuIndex], &info);
	LOGFILE("status:%d NvAPI_GPU_GetDynamicPstatesInfoEx info:0x%p\n", status, info);
	if (status == NVAPI_OK)
	{
		LOGFILE(" domain:%d present:%d percentage:%d\n", NVAPI_GPU_UTILIZATION_DOMAIN_GPU, info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].bIsPresent, info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].percentage);
		if (info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwGpuUtilization = info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].percentage;
		else
			pData->pstGpuStatus[dwGpuIndex].dwGpuUtilization = 0;

		LOGFILE(" domain:%d present:%d percentage:%d\n", NVAPI_GPU_UTILIZATION_DOMAIN_FB, info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_FB].bIsPresent, info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_FB].percentage);
		if (info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_FB].bIsPresent > 0)
			pData->pstGpuStatus[dwGpuIndex].dwFrameBufferUtilization = info.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_FB].percentage;
		else
			pData->pstGpuStatus[dwGpuIndex].dwFrameBufferUtilization = 0;
	
		*pdwGpuUtilization = pData->pstGpuStatus[dwGpuIndex].dwGpuUtilization;
		*pdwFameBufferUtilization = pData->pstGpuStatus[dwGpuIndex].dwFrameBufferUtilization;
	}
	else
	{
		*pdwGpuUtilization = 0;
		*pdwFameBufferUtilization = 0;
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetThermalSettings(NvU32 dwGpuIndex, NvU32 dwSensorIndex, NvS32 *pdwGpuCurrentTemperature)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwGpuCurrentTemperature = 0;
		return status;
	}

	NV_GPU_THERMAL_SETTINGS thermal = { 0, };
	thermal.version = NV_GPU_THERMAL_SETTINGS_VER;

	status = NvAPI_GPU_GetThermalSettings(pData->phPhysicalGpu[dwGpuIndex], dwSensorIndex, &thermal);
	LOGFILE("status:%d NvAPI_GPU_GetThermalSettings count:%d\n", status, thermal.count);
	if (status == NVAPI_OK)
	{
		for (size_t i = 0; i < thermal.count; i++)
		{
			LOGFILE(" index:%d target:%d temperature:%dC\n", i, thermal.sensor[i].target, thermal.sensor[i].currentTemp);
			if (thermal.sensor[i].target == NVAPI_THERMAL_TARGET_GPU)
			{
				pData->pstGpuStatus[dwGpuIndex].dwGpuCurrentTemperature = thermal.sensor[i].currentTemp;
			}
		}

		*pdwGpuCurrentTemperature = pData->pstGpuStatus[dwGpuIndex].dwGpuCurrentTemperature;
	}
	else
	{
		*pdwGpuCurrentTemperature = 0;
	}

	return status;
}

__declspec(dllexport) NvAPI_Status GetTachometerReading(NvU32 dwGpuIndex, NvU32 *pdwTachometerValue)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwTachometerValue = 0;
		return status;
	}

	NvU32 tach = 0;
	status = NvAPI_GPU_GetTachReading(pData->phPhysicalGpu[dwGpuIndex], &tach);
	LOGFILE("status:%d NvAPI_GPU_GetTachReading value:%dRPM\n", status, tach);
	if (status == NVAPI_OK)
		pData->pstGpuStatus[dwGpuIndex].dwTachometerValue = tach;
	else
		pData->pstGpuStatus[dwGpuIndex].dwTachometerValue = 0;
	
	*pdwTachometerValue = pData->pstGpuStatus[dwGpuIndex].dwTachometerValue;

	return status;
}

__declspec(dllexport) NvAPI_Status GetCoolerSettings(NvU32 dwGpuIndex, NvU32 dwCoolerIndex, NvU32 *pdwCoolerCurrentLevel, NvU32 *pdwTachometerValue)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwCoolerCurrentLevel = 0;
		*pdwTachometerValue = 0;
		return status;
	}

	NV_GPU_GETCOOLER_SETTINGS cooler = { 0, };
	cooler.version = NV_GPU_GETCOOLER_SETTINGS_VER;
	
	status = NvAPI_GPU_GetCoolerSettings(pData->phPhysicalGpu[dwGpuIndex], dwCoolerIndex, &cooler);
	LOGFILE("status:%d NvAPI_GPU_GetCoolerSettings count:%d\n", status, cooler.count);
	if (status != NVAPI_OK)
	{
		*pdwCoolerCurrentLevel = 0;
		*pdwTachometerValue = 0;
		return status;
	}

	/// Flag status if data not match method's purpose
	status = NVAPI_DATA_NOT_FOUND;

	for (size_t i = 0; i < cooler.count; i++)
	{
		if (cooler.cooler[i].currentPolicy == NV_COOLER_POLICY::NVAPI_COOLER_POLICY_MANUAL)
		{
			LOGFILE(" index:%d target:%d policy:0x%x(manual)", i, cooler.cooler[i].target, cooler.cooler[i].currentPolicy);
		}
		else if (cooler.cooler[i].currentPolicy == NV_COOLER_POLICY::NVAPI_COOLER_POLICY_DEFAULT)
		{
			LOGFILE(" index:%d target:%d policy:0x%x(default)", i, cooler.cooler[i].target, cooler.cooler[i].currentPolicy);
		}
		else
		{
			LOGFILE(" index:%d target:%d policy:0x%x( )", i, cooler.cooler[i].target, cooler.cooler[i].currentPolicy);
		}
		if (GET_NVAPI_VERSION(cooler.version) >= 3)
		{
			LOGFILE(" level:%d tach:%d rpm:%d\n", cooler.cooler[i].currentLevel, cooler.cooler[i].tachometer.bSupported, cooler.cooler[i].tachometer.speedRPM);
		}
		else
		{
			LOGFILE(" active:%d level:%d\n", cooler.cooler[i].active, cooler.cooler[i].currentLevel);
		}

		if (cooler.cooler[i].target == NVAPI_COOLER_TARGET_GPU || 
			cooler.cooler[i].target == NVAPI_COOLER_TARGET_ALL)
		{
			pData->pstGpuStatus[dwGpuIndex].dwCoolerCurrentLevel = cooler.cooler[i].currentLevel;
			/// Get tachometer reading RPM value if setting version support
			if (GET_NVAPI_VERSION(cooler.version) >= 3 ||
				cooler.cooler[i].tachometer.bSupported > 0)
			{
				pData->pstGpuStatus[dwGpuIndex].dwTachometerValue = cooler.cooler[i].tachometer.speedRPM;
			}
			/// Otherwise try to use GetTachometerReading method to get reading value
			else
			{
				NvU32 rpmReading = 0;
				if (GetTachometerReading(dwGpuIndex, &rpmReading) == NVAPI_OK)
					pData->pstGpuStatus[dwGpuIndex].dwTachometerValue = rpmReading;
			}
			/// Get default min/max level and default policy for SetCoolerLevels method
			if (pData->pstGpuStatus[dwGpuIndex].dwCoolerMinLevel == pData->pstGpuStatus[dwGpuIndex].dwCoolerMaxLevel ||
				pData->pstGpuStatus[dwGpuIndex].dwCoolerMaxLevel == 0)
			{
				pData->pstGpuStatus[dwGpuIndex].dwCoolerMinLevel = cooler.cooler[i].defaultMinLevel;
				pData->pstGpuStatus[dwGpuIndex].dwCoolerMaxLevel = cooler.cooler[i].defaultMaxLevel;
			}
			/// Raise succeeded flag if cooler target is match
			status = NVAPI_OK;
		}
	}

	*pdwCoolerCurrentLevel = pData->pstGpuStatus[dwGpuIndex].dwCoolerCurrentLevel;
	*pdwTachometerValue = pData->pstGpuStatus[dwGpuIndex].dwTachometerValue;

	return status;
}

__declspec(dllexport) NvAPI_Status SetCoolerLevels(NvU32 dwGpuIndex, NvU32 dwCoolerIndex, bool bIsManual, NvU32 dwCoolerLevel)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		return status;
	}

	/// Get cooler default value from GetCoolerSettings method before set cooler levels
	if (pData->pstGpuStatus[dwGpuIndex].dwCoolerMinLevel == pData->pstGpuStatus[dwGpuIndex].dwCoolerMaxLevel ||
		pData->pstGpuStatus[dwGpuIndex].dwCoolerMaxLevel == 0)
	{
		NvU32 currentLevel = 0;
		NvU32 tachoValue = 0;
		status = GetCoolerSettings(dwGpuIndex, dwCoolerIndex, &currentLevel, &tachoValue);
		if (status != NVAPI_OK)
			return status;
	}

	NV_GPU_SETCOOLER_LEVEL level = { 0, };
	level.version = NV_GPU_SETCOOLER_LEVEL_VER;
	/// Uses manual or default policy
	if (bIsManual == true)
	{
		level.cooler[0].currentPolicy = NV_COOLER_POLICY::NVAPI_COOLER_POLICY_MANUAL;
		level.cooler[0].currentLevel = max(min(dwCoolerLevel, pData->pstGpuStatus[dwGpuIndex].dwCoolerMaxLevel), pData->pstGpuStatus[dwGpuIndex].dwCoolerMinLevel);
	}
	else
	{
		level.cooler[0].currentPolicy = NV_COOLER_POLICY::NVAPI_COOLER_POLICY_DEFAULT;
	}
	status = NvAPI_GPU_SetCoolerLevels(pData->phPhysicalGpu[dwGpuIndex], dwCoolerIndex, &level);
	LOGFILE("status:%d NvAPI_GPU_SetCoolerLevels policy:0x%x level:%d\n", status, level.cooler[0].currentPolicy, level.cooler[0].currentLevel);

	return status;
}

/// Notice: method does not verified
NvAPI_Status QueryIlluminationSupport(NvU32 dwGpuIndex, bool *pbIsSupportLogo, bool *pbIsSupportSli)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pbIsSupportLogo = false;
		*pbIsSupportSli = false;
		return status;
	}

	NV_GPU_QUERY_ILLUMINATION_SUPPORT_PARM support = { 0, };
	support.version = NV_GPU_QUERY_ILLUMINATION_SUPPORT_PARM_VER;
	support.hPhysicalGpu = pData->phPhysicalGpu[dwGpuIndex];
	support.Attribute = NV_GPU_ILLUMINATION_ATTRIB::NV_GPU_IA_LOGO_BRIGHTNESS;

	status = NvAPI_GPU_QueryIlluminationSupport(&support);
	LOGFILE("status:%d NvAPI_GPU_QueryIlluminationSupport index:%d attr:%d support:%d\n", status, dwGpuIndex, support.Attribute, support.bSupported);
	if (status == NVAPI_OK)
		*pbIsSupportLogo = (support.bSupported == 1);
	else
		*pbIsSupportLogo = false;

	support.version = NV_GPU_QUERY_ILLUMINATION_SUPPORT_PARM_VER;
	support.hPhysicalGpu = pData->phPhysicalGpu[dwGpuIndex];
	support.Attribute = NV_GPU_ILLUMINATION_ATTRIB::NV_GPU_IA_SLI_BRIGHTNESS;
	support.bSupported = 0;

	status = NvAPI_GPU_QueryIlluminationSupport(&support);
	LOGFILE("status:%d NvAPI_GPU_QueryIlluminationSupport index:%d attr:%d support:%d\n", status, dwGpuIndex, support.Attribute, support.bSupported);
	if (status == NVAPI_OK)
		*pbIsSupportSli = (support.bSupported == 1);
	else
		*pbIsSupportSli = false;

	return status;
}

/// Notice: method does not verified
NvAPI_Status GetIllumination(NvU32 dwGpuIndex, NvU32 dwAttribute, NvU32 *pdwValue)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pdwValue = 0;
		return status;
	}
	
	if (dwAttribute >= 2)
	{
		status = NVAPI_INVALID_ARGUMENT;
	}

	NV_GPU_GET_ILLUMINATION_PARM param = { 0, };
	param.version = NV_GPU_GET_ILLUMINATION_PARM_VER;
	param.hPhysicalGpu = pData->phPhysicalGpu[dwGpuIndex];
	if (dwAttribute == 0)
		param.Attribute = NV_GPU_ILLUMINATION_ATTRIB::NV_GPU_IA_LOGO_BRIGHTNESS;
	if (dwAttribute == 1)
		param.Attribute = NV_GPU_ILLUMINATION_ATTRIB::NV_GPU_IA_SLI_BRIGHTNESS;

	status = NvAPI_GPU_GetIllumination(&param);
	LOGFILE("status:%d NvAPI_GPU_GetIllumination index:%d attr:%d value:%d\n", status, dwGpuIndex, param.Attribute, param.Value);
	if (status == NVAPI_OK)
		*pdwValue = param.Value;
	else
		*pdwValue = 0;

	return status;
}

/// Notice: method does not verified
NvAPI_Status SetIllumination(NvU32 dwGpuIndex, NvU32 dwAttribute, NvU32 dwValue)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		return status;
	}

	if (dwAttribute >= 2)
	{
		status = NVAPI_INVALID_ARGUMENT;
	}

	NV_GPU_SET_ILLUMINATION_PARM param = { 0, };
	param.version = NV_GPU_SET_ILLUMINATION_PARM_VER;
	param.hPhysicalGpu = pData->phPhysicalGpu[dwGpuIndex];
	if (dwAttribute == 0)
		param.Attribute = NV_GPU_ILLUMINATION_ATTRIB::NV_GPU_IA_LOGO_BRIGHTNESS;
	if (dwAttribute == 1)
		param.Attribute = NV_GPU_ILLUMINATION_ATTRIB::NV_GPU_IA_SLI_BRIGHTNESS;
	param.Value = dwValue;

	status = NvAPI_GPU_SetIllumination(&param);
	LOGFILE("status:%d NvAPI_GPU_SetIllumination index:%d attr:%d value:%d\n", status, dwGpuIndex, param.Attribute, param.Value);

	return status;
}

NvAPI_Status GetCurrentSliState(NvU32* pdwCurrentAFRIndex, NvU32* pdwAFRGroups, NvU32* pdwMaxAFRGroups)
{
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL FeatureLevelsSupported;
	ID3D11Device *pDevice = NULL;
	ID3D11DeviceContext *pDeviceContext = NULL;
	HRESULT hr = D3D11CreateDevice(NULL, 
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, 
		0, 
		&FeatureLevelsRequested, 
		1, 
		D3D11_SDK_VERSION, 
		&pDevice, 
		&FeatureLevelsSupported, 
		&pDeviceContext);

	LOGFILE("result:0x%x d3ddevice:0x%p context:0x%p\n", hr, pDevice, pDeviceContext);

	NvAPI_Status status = NVAPI_OK;
	if (SUCCEEDED(hr))
	{
		NV_GET_CURRENT_SLI_STATE sli = { 0, };
		/// Use V1 struct
		sli.version = NV_GET_CURRENT_SLI_STATE_VER1;
		status = NvAPI_D3D_GetCurrentSLIState(pDevice, &sli);
		LOGFILE("status:%d NvAPI_D3D_GetCurrentSLIState sli:0x%p\n", status, sli);
		if (status == NVAPI_OK)
		{
			LOGFILE(" sliisnew:%d afrndex:%d afrgrop:%d maxgrop:%d vrgpu:%d\n", sli.bIsCurAFRGroupNew, sli.currentAFRIndex, sli.numAFRGroups, sli.maxNumAFRGroups, sli.numVRSLIGpus);
		}

		pDeviceContext->Release();
		pDevice->Release();
	}
	
	return status;
}

// nvml feature begin

// It may needs bus info for match with nvml library
NvAPI_Status GetBusInfo(NvU32 dwGpuIndex, unsigned short *pwBusType, NvU32 *pdwBusId, NvU32 *pdwSlotId)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		*pwBusType = 0;
		*pdwBusId = 0;
		*pdwSlotId = 0;
		return status;
	}

	NV_GPU_BUS_TYPE type = NV_GPU_BUS_TYPE::NVAPI_GPU_BUS_TYPE_UNDEFINED;
	status = NvAPI_GPU_GetBusType(pData->phPhysicalGpu[dwGpuIndex], &type);
	LOGFILE("status:%d NvAPI_GPU_GetBusType type:%d(3=pcie)\n", status, type);
	if (status == NVAPI_OK)
		*pwBusType = type;
	else
		*pwBusType = 0;

	// Note: each PCIE physical graphic card reserved a bus (currently GTX 10X0 and GTX 9X0 seems like that)
	NvU32 busId = 0;
	status = NvAPI_GPU_GetBusId(pData->phPhysicalGpu[dwGpuIndex], &busId);
	LOGFILE("status:%d NvAPI_GPU_GetBusId id:%d\n", status, busId);
	if (status == NVAPI_OK)
		*pdwBusId = busId;
	else
		*pdwBusId = 0;
	// Note: each GPU on PCIE physical graphic card reserved a slot (but currently does not have multiple GPUs graphic card can test)
	NvU32 slotId = 0;
	status = NvAPI_GPU_GetBusSlotId(pData->phPhysicalGpu[dwGpuIndex], &slotId);
	LOGFILE("status:%d NvAPI_GPU_GetBusSlotId id:%d\n", status, slotId);
	if (status == NVAPI_OK)
		*pdwSlotId = slotId;
	else
		*pdwSlotId = 0;

	return status;
}

// nvml needs device handle
nvmlReturn_t GetNvMLDeviceHandle(NvU32 dwGpuIndex)
{
	NvAPI_Status status = CheckPhysicalGpuExists(&dwGpuIndex);
	if (status != NVAPI_OK)
	{
		return nvmlReturn_t::NVML_ERROR_NOT_FOUND;
	}

	unsigned short type = 0;
	NvU32 busId = 0;
	NvU32 slotId = 0;
	// Get bus id to get nvml device
	status = GetBusInfo(dwGpuIndex, &type, &busId, &slotId);
	if (status == NVAPI_OK)
	{
		// Bus Id format: 00000000:01:00.0 (00000000:bus:device.func)
		std::string strBusId = std::string((busId < 10) ? "00000000:0" : "00000000:") + std::to_string(busId) + std::string(":00.0");
		//std::string strBusId = "00000000:01:00.0";
		nvmlDevice_t device;
		nvmlReturn_t result = nvmlDeviceGetHandleByPciBusId(strBusId.c_str(), &device);
		LOGFILE("result:%d nvmlDeviceGetHandleByPciBusId busId:%s handle:0x%p\n", result, strBusId.c_str(), device);
		if (result == NVML_SUCCESS)
		{
			pData->pstGpuDevice[dwGpuIndex] = device;
		}
		return result;
	}
	else
		return nvmlReturn_t::NVML_ERROR_INVALID_ARGUMENT;
}

__declspec(dllexport) nvmlReturn_t GetDevicePowerLimit(NvU32 dwGpuIndex, unsigned int *pdwPowerLimit, unsigned int *pdwDefaultLimit, unsigned int *pdwEnforcedLimit)
{
	if (pData->pstGpuDevice[dwGpuIndex] == NULL)
	{
		*pdwPowerLimit = 0;
		*pdwDefaultLimit = 0;
		*pdwEnforcedLimit = 0;
		return nvmlReturn_t::NVML_ERROR_NOT_FOUND;
	}

	nvmlEnableState_t mode;
	nvmlReturn_t result = nvmlDeviceGetPowerManagementMode(pData->pstGpuDevice[dwGpuIndex], &mode);
	LOGFILE("result:%d nvmlDeviceGetPowerManagementMode mode:%d\n", result, mode);
	if (result == NVML_SUCCESS)
	{
		if (mode == nvmlEnableState_t::NVML_FEATURE_ENABLED)
		{
			unsigned int limit = 0;
			result = nvmlDeviceGetPowerManagementLimit(pData->pstGpuDevice[dwGpuIndex], &limit);
			LOGFILE("result:%d nvmlDeviceGetPowerManagementLimit limit:%d\n", result, limit);
			if (result == NVML_SUCCESS)
			{
				*pdwPowerLimit = limit;
			}
		}
		else
		{
			result = nvmlReturn_t::NVML_ERROR_NOT_SUPPORTED;
		}

		unsigned int def = 0;
		nvmlReturn_t result2 = nvmlDeviceGetPowerManagementDefaultLimit(pData->pstGpuDevice[dwGpuIndex], &def);
		LOGFILE("result:%d nvmlDeviceGetPowerManagementDefaultLimit default:%d\n", result2, def);
		if (result2 == NVML_SUCCESS)
			*pdwDefaultLimit = def;
		else
			*pdwDefaultLimit = 0;

		// Not all GPU supports enforced limit
		unsigned int enforced = 0;
		result2 = nvmlDeviceGetEnforcedPowerLimit(pData->pstGpuDevice[dwGpuIndex], &enforced);
		LOGFILE("result:%d nvmlDeviceGetEnforcedPowerLimit enforced:%d\n", result2, enforced);
		if (result2 == NVML_SUCCESS)
			*pdwEnforcedLimit = enforced;
		else
			*pdwEnforcedLimit = 0;
	}

	if (result != NVML_SUCCESS)
		*pdwPowerLimit = 0;
	return result;
}

__declspec(dllexport) nvmlReturn_t SetDevicePowerLimit(NvU32 dwGpuIndex, unsigned int dwPowerLimit)
{
	if (pData->pstGpuDevice[dwGpuIndex] == NULL)
	{
		return nvmlReturn_t::NVML_ERROR_NOT_FOUND;
	}

	nvmlEnableState_t mode;
	nvmlReturn_t result = nvmlDeviceGetPowerManagementMode(pData->pstGpuDevice[dwGpuIndex], &mode);
	//LOGFILE("result:%d nvmlDeviceGetPowerManagementMode mode:%d\n", result, mode);
	if (result == NVML_SUCCESS)
	{
		if (mode == nvmlEnableState_t::NVML_FEATURE_ENABLED)
		{
			result = nvmlDeviceSetPowerManagementLimit(pData->pstGpuDevice[dwGpuIndex], dwPowerLimit);
			LOGFILE("result:%d nvmlDeviceSetPowerManagementLimit limit:%d\n", result, dwPowerLimit);
		}
		else
			result = NVML_ERROR_NOT_SUPPORTED;
	}

	return result;
}

// Not finish yet because memory increase for processinfo array
nvmlReturn_t GetRunningProcess(NvU32 dwGpuIndex, unsigned int *pdwProcessCount)
{
	if (pData->pstGpuDevice[dwGpuIndex] == NULL)
	{
		*pdwProcessCount = 0;
		return nvmlReturn_t::NVML_ERROR_NOT_FOUND;
	}

	unsigned int count = 100;
	nvmlProcessInfo_t info[100] = { 0, };
	nvmlReturn_t result = nvmlDeviceGetGraphicsRunningProcesses(pData->pstGpuDevice[dwGpuIndex], &count, info);
	LOGFILE("result:%d count:%d info:%p\n", result, count, info);
	for (unsigned int i = 0; i < count; i++)
	{
		char name[512] = "";
		result = nvmlSystemGetProcessName(info[i].pid, name, 512);
		LOGFILE(" info:%d pid:%d name:%s\n", i, info[i].pid, name);
	}

	nvmlProcessUtilizationSample_t util[100] = { 0, };
	count = 100;
	result = nvmlDeviceGetProcessUtilization(pData->pstGpuDevice[dwGpuIndex], util, &count, 0);
	LOGFILE("result:%d count:%d util:%p\n", result, count, util);
	for (unsigned int i = 0; i < count; i++)
	{
		char name[512] = "";
		result = nvmlSystemGetProcessName(util[i].pid, name, 512);
		LOGFILE(" util:%d pid:%d name:%s sm:%d time:%llu\n", i, util[i].pid, name, util[i].smUtil, util[i].timeStamp);
	}

	if (result != NVML_SUCCESS)
		*pdwProcessCount = 0;

	return result;
}

nvmlReturn_t GetHighestProcessUtilization(NvU32 dwGpuIndex, unsigned int *pdwProcessCount, unsigned int *pdwHighestUtilization, unsigned int *pdwHighestPid, char szHighestPidName[256])
{
	if (pData->pstGpuDevice[dwGpuIndex] == NULL)
	{
		*pdwProcessCount = 0;
		*pdwHighestPid = 0;
		*pdwHighestUtilization = 0;
		return nvmlReturn_t::NVML_ERROR_NOT_FOUND;
	}

	//nvmlProcessUtilizationSample_t util[100] = { 0, };
	unsigned int count = 100;
	nvmlReturn_t result = nvmlDeviceGetProcessUtilization(pData->pstGpuDevice[dwGpuIndex], pData->pstUtilization, &count, 0);
	LOGFILE("result:%d count:%d util:%p\n", result, count, pData->pstUtilization);
	if (result == NVML_SUCCESS)
	{
		*pdwProcessCount = count;
		for (unsigned int i = 0; i < count; i++)
		{
			// Get process name
			char name[256] = "";
			result = nvmlSystemGetProcessName(pData->pstUtilization[i].pid, name, 256);
			LOGFILE(" util:%d pid:%d name:%s sm:%d time:%llu\n", i, pData->pstUtilization[i].pid, name, pData->pstUtilization[i].smUtil, pData->pstUtilization[i].timeStamp);
			if (*pdwHighestUtilization < pData->pstUtilization[i].smUtil)
			{
				*pdwHighestUtilization = pData->pstUtilization[i].smUtil;
				*pdwHighestPid = pData->pstUtilization[i].pid;
				if (result == NVML_SUCCESS)
					strcpy_s(szHighestPidName, 256, name);
				else
					szHighestPidName = "";
			}
		}
	}
	else
	{
		*pdwProcessCount = 0;
		*pdwHighestPid = 0;
		*pdwHighestUtilization = 0;
	}

	return result;
}

__declspec(dllexport) nvmlReturn_t GetProcessUtilization(NvU32 dwGpuIndex, unsigned int *pdwProcessCount, nvmlProcessUtilizationSample_t *pstSample)
{
	if (pData->pstGpuDevice[dwGpuIndex] == NULL)
	{
		*pdwProcessCount = 0;
		return nvmlReturn_t::NVML_ERROR_NOT_FOUND;
	}

	//nvmlProcessUtilizationSample_t util[100] = { 0, };
	unsigned int count = 100;
	nvmlReturn_t result = nvmlDeviceGetProcessUtilization(pData->pstGpuDevice[dwGpuIndex], pData->pstUtilization, &count, 0);
	LOGFILE("result:%d count:%d util:%p\n", result, count, pData->pstUtilization);
	if (result == NVML_SUCCESS)
	{
		// Sort by smUtil
		std::qsort(pData->pstUtilization, count, sizeof(nvmlProcessUtilizationSample_t), [](const void *a, const void *b){ return (int)(((nvmlProcessUtilizationSample_t *)b)->smUtil) - (int)(((nvmlProcessUtilizationSample_t *)a)->smUtil); });
		
		// If requested process count larger then current count, return current count
		if (*pdwProcessCount >= count)
			*pdwProcessCount = count;
		for (unsigned int i = 0; i < *pdwProcessCount; i++)
		{
			//pstSample[i] = { 0, };
			pstSample[i].pid = pData->pstUtilization[i].pid;
			pstSample[i].timeStamp = pData->pstUtilization[i].timeStamp;
			pstSample[i].smUtil = pData->pstUtilization[i].smUtil;
			pstSample[i].memUtil = pData->pstUtilization[i].memUtil;
			pstSample[i].encUtil = pData->pstUtilization[i].encUtil;
			pstSample[i].decUtil = pData->pstUtilization[i].decUtil;
			LOGFILE(" proc id:%d util:%d mem:%d enc:%d dec:%d time:%llu\n", pstSample[i].pid, pstSample[i].smUtil, pstSample[i].memUtil, pstSample[i].encUtil, pstSample[i].decUtil, pstSample[i].timeStamp);
		}
	}
	else
	{
		*pdwProcessCount = 0;
	}

	return result;
}