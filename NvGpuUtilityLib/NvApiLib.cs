using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace NvGpuUtilityLib
{
    class NvApiLib
    {
        /// <summary>
        /// The initialization flag for NVAPI<para></para>
        /// It must be call Initialize() method if value is false and save back result<para></para>
        /// It must be call UnInitialize() method if value is true before exit application
        /// </summary>
        internal static bool IsInitialized = false;

        /// <summary>
        /// Before using any methods of NVAPI, it must be call Initialize() at first.<para></para>
        /// Save result to IsInitialized flag and make sure NVAPI only needs initialize once<para></para>
        /// If initialization succeeded, it must be call UnInitialize() method before exit application
        /// </summary>
        /// <returns>Succeeded or Failed</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool Initialize();

        /// <summary>
        /// Before exit application, it must be call UnInitialize() if NVAPI been initialized.
        /// </summary>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void UnInitialize();

        /// <summary>
        /// Before using other GPU related methods of NVAPI, it must be get GPU count and primary(output connected to display) GPU.<para></para>
        /// The GPU index is depends on GPU count likes an array.(eg, 2 GPU counts includes first GPU at index 0 and second GPU at index 1, and etc)<para></para>
        /// In multiple GPUs SKU, if output GPU has been changed by driver or system, it may needs re-call the method again.
        /// </summary>
        /// <param name="pdwPhysicalGpuCount">[out] Physical GPUs count</param>
        /// <param name="pdwPrimaryGpuIndex">[out] GPU index which output connected to display</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetGpuCountAndPrimaryIndex(out uint pdwPhysicalGpuCount, out uint pdwPrimaryGpuIndex);

        /// <summary>
        /// Get GPU name, BIOS version number, hardware ID with given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="szFullName">[out] GPU name, maximum char length is 16</param>
        /// <param name="szBiosVersion">[out] BIOS number, maximum char length is 16</param>
        /// <param name="pdwDeviceId">[out] VendorId + DeviceId</param>
        /// <param name="pdwSubSystemId">[out] SubSystemId + SubVendorId</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetGpuInformation(uint dwGpuIndex, StringBuilder szFullName, StringBuilder szBiosVersion, out uint pdwDeviceId, out uint pdwSubSystemId);

        /// <summary>
        /// Get current core and memory clocks frequencies by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwCoreClock">[out] Current core frequency(KHz)</param>
        /// <param name="pdwMemoryClock">[out] Current memory frequency(KHz)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetCurrentClockFrequencies(uint dwGpuIndex, out uint pdwCoreClock, out uint pdwMemoryClock);

        /// <summary>
        /// Get base core and memory clocks frequencies by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwCoreClock">[out] Base core frequency(KHz)</param>
        /// <param name="pdwMemoryClock">[out] Base memory frequency(KHz)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetBaseClockFrequencies(uint dwGpuIndex, out uint pdwCoreClock, out uint pdwMemoryClock);

        /// <summary>
        /// Get boost core and memory clocks frequencies by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwCoreClock">[out] Boost core frequency(KHz)</param>
        /// <param name="pdwMemoryClock">[out] Boost memory frequency(KHz)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetBoostClockFrequencies(uint dwGpuIndex, out uint pdwCoreClock, out uint pdwMemoryClock);

        /// <summary>
        /// Get current PState by given GPU index<para></para>
        /// There are P0(highest) ~ P15(lowest) states for GPU performance level.<para></para>
        /// Normally represent ran at P0, P2, P5, P8 within system operated usage. 
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pwCurrentPState">[out] Current PState</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetCurrentPState(uint dwGpuIndex, out ushort pwCurrentPState);

        /// <summary>
        /// Get PState limitation by given GPU index<para></para>
        /// Normally represent P0 (No limit), or one of P0(highest) ~ P15(lowest)
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pwHardwareLimitPState">[out] Hardware defined PState limitation</param>
        /// <param name="pwSoftwareLimitPState">[out] Software defined PState limitation</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetPStateLimits(uint dwGpuIndex, out ushort pwHardwareLimitPState, out ushort pwSoftwareLimitPState);

        /// <summary>
        /// Get PStates info data structor by given GPU index<para></para>
        /// Because PStates info has huge data represent each PState definition which of clocks frequencies, base and over voltages,<para></para>
        /// so currently only return GPU's ability from PStates info
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="isInfoEditable">[out] PStates info is editable</param>
        /// <param name="isPStateEditable">[out] PStates info includes editable PState</param>
        /// <param name="isGpuFreqEditable">[out] PState includes editable core frequency</param>
        /// <param name="isMemFreqEditable">[out] PState includes editable memory frequency</param>
        /// <param name="isBaseVoltEditable">[out] PState includes editable base voltage</param>
        /// <param name="isOverVoltEditable">[out] PStates info includes edtiable over voltage</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetPStatesInfo(uint dwGpuIndex, out bool isInfoEditable, out bool isPStateEditable, out bool isGpuFreqEditable, out bool isMemFreqEditable, out bool isBaseVoltEditable, out bool isOverVoltEditable);

        /// <summary>
        /// Get core frequency of current maximum increment/decrement of editable PStates
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwGpuFreqDelta">[out] Maximum core frequency increment/decrement(KHz)</param>
        /// <param name="pwEditablePState">[out] Editable PState (P0~P15) contains maximum value</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetPStatesInfoGpuFrequencyDelta(uint dwGpuIndex, out int pdwGpuFreqDelta, out ushort pwEditablePState);

        /// <summary>
        /// Set core frequency increment/decrement to editable PStates by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwGpuFreqDelta">[in] Core frequency increment/decrement(KHz)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetPStatesInfoGpuFrequencyDelta(uint dwGpuIndex, int dwGpuFreqDelta);

        /// <summary>
        /// Get memory frequency of current maximum increment/decrement of editable PStates
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwMemFreqDelta">[out] Maximum memory frequency increment/decrement(KHz)</param>
        /// <param name="pwEditablePState">[out] Editable PState (P0~P15) contains maximum value</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetPStatesInfoMemFrequencyDelta(uint dwGpuIndex, out int pdwMemFreqDelta, out ushort pwEditablePState);

        /// <summary>
        /// Set memory frequency increment/decrement to editable PStates by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwMemFreqDelta">[in] Memory frequency increment/decrement(KHz)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetPStatesInfoMemFrequencyDelta(uint dwGpuIndex, int dwMemFreqDelta);

        /// <summary>
        /// Get base voltage of current maximum increment/decrement of editable PStates
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwBaseVoltDelta">[out] Maximum base voltage increment/decrement(uV)</param>
        /// <param name="pwEditablePState">[out] Editable PState (P0~P15) contains maximum value</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetPStatesInfoBaseVoltageDelta(uint dwGpuIndex, out int pdwBaseVoltDelta, out ushort pwEditablePState);

        /// <summary>
        /// Set base voltage increment/decrement to editable PStates by given GPU index<para></para>
        /// It seems like 6250uV per each steps and higher PState can not lower then lower PState<para></para>
        /// (Most GPU only can be change P0 State, that this method only can given increment voltage)
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwBaseVoltDelta">[in] Base voltage increment/decrement(uV)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetPStatesInfoBaseVoltageDelta(uint dwGpuIndex, int dwBaseVoltDelta);

        /// <summary>
        /// Get voltage increment from editable over voltage
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwOverVoltDelta">[out] Over voltage increment(uV)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetPStatesInfoOverVoltageDelta(uint dwGpuIndex, out int pdwOverVoltDelta);

        /// <summary>
        /// Set voltage increment to editable over voltage by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwOverVoltDelta">[in] Over voltage increment(uV)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetPStatesInfoOverVoltageDelta(uint dwGpuIndex, int dwOverVoltDelta);

        /// <summary>
        /// Get current GPU and buffer loading by given GPU index
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwGpuUtilization">[out] Current core utilization(%)</param>
        /// <param name="pdwFameBufferUtilization">[out] Current frame buffer utilization(%)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDynamicPStatesInfo(uint dwGpuIndex, out uint pdwGpuUtilization, out uint pdwFameBufferUtilization);

        /// <summary>
        /// Get current core temperature by given GPU index and sensor index(usually is 0)
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwSensorIndex">[in] Given sensor index</param>
        /// <param name="pdwGpuTemperature">[out] Current core temperature('C)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetThermalSettings(uint dwGpuIndex, uint dwSensorIndex, out int pdwGpuTemperature);

        /// <summary>
        /// Get current fan duty and its speed by given GPU index and cooler index(usually is 0)
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwCoolerIndex">[in] Given cooler index</param>
        /// <param name="pdwCoolerCurrentLevel">[out] Current fan duty</param>
        /// <param name="pdwTachometerValue">[out] Current fan speed(RPM)</param>
        /// <returns>NVAPI Status</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetCoolerSettings(uint dwGpuIndex, uint dwCoolerIndex, out uint pdwCoolerCurrentLevel, out uint pdwTachometerValue);

        /// <summary>
        /// Set current fan duty with manual fan or default fan policy by given GPU index and cooler index(usually is 0)
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwCoolerIndex">[in] Given cooler index</param>
        /// <param name="bIsManual">[in] Is manual fan or reset default fan policy</param>
        /// <param name="dwCoolerLevel">[in] Given manual fan duty, ignored if bIsManual is false</param>
        /// <returns></returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetCoolerLevels(uint dwGpuIndex, uint dwCoolerIndex, bool bIsManual, uint dwCoolerLevel);

        /// <summary>
        /// Get illumination support by given GPU index, only one GPU will support SLI illumination in SLI bridge<para></para>
        /// Notice: method does not verified
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pbIsSupportLogo">[out] Is LOGO illumination support</param>
        /// <param name="pbIsSupportSli">[out] Is SLI illumination support</param>
        /// <returns></returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int QueryIlluminationSupport(uint dwGpuIndex, out bool pbIsSupportLogo, out bool pbIsSupportSli);

        /// <summary>
        /// Get illumination value by given GPU index and illumination attribute<para></para>
        /// Notice: method does not verified
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwAttribute">[in] Given illumination, 0=LOGO 1=SLI</param>
        /// <param name="pdwValue">[out] Illumination brightness value (0-100)</param>
        /// <returns></returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetIllumination(uint dwGpuIndex, uint dwAttribute, out uint pdwValue);

        /// <summary>
        /// Set illumination value by given GPU index and illumination attribute<para></para>
        /// Notice: method does not verified
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwAttribute">[in] Given illumination, 0=LOGO 1=SLI</param>
        /// <param name="dwValue">[in] Given illumination brightness value (0-100)</param>
        /// <returns></returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetIllumination(uint dwGpuIndex, uint dwAttribute, uint dwValue);

        // NVML begin: copy nvml.dll from nvidia program files folder to exe file path

        /// <summary>
        /// Get device power limit if power management mode is enable, otherwise return 0
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwPowerLimit">[out] Current power limitation(mW)</param>
        /// <param name="pdwDefaultLimit">[out] Default power limitation(mW)</param>
        /// <param name="pdwEnforcedLimit">[out] Enforced power limitation(mW) not all GPU supports this feature</param>
        /// <returns>nvmlReturn_t</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDevicePowerLimit(uint dwGpuIndex, out uint pdwPowerLimit, out uint pdwDefaultLimit, out uint pdwEnforcedLimit);

        /// <summary>
        /// Set device power limit if power management mode is enable
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="dwPowerLimit">[in] Given power limitation(mW)</param>
        /// <returns>nvmlReturn_t</returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetDevicePowerLimit(uint dwGpuIndex, uint dwPowerLimit);

        /// <summary>
        /// Get process utilization sample list by given number of counts
        /// </summary>
        /// <param name="dwGpuIndex">[in] Given GPU index</param>
        /// <param name="pdwProcessCount">[in/out] Set process sample counts and get retrieved sample counts, Maximum is 100</param>
        /// <param name="pStSamples">[in/out] Set sample structure array and get retrieved sample data, Given array size must be consistent with process counts</param>
        /// <returns></returns>
        [DllImport("nvapihelper.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetProcessUtilization(uint dwGpuIndex, out uint pdwProcessCount, [In, Out] ProcessUtilizationSample[] pStSamples);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ProcessUtilizationSample
    {
        public uint pid;
        public ulong timestamp;
        public uint smUtil;
        public uint memUtil;
        public uint encUtil;
        public uint decUtil;
    }
}
