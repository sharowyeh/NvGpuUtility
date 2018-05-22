using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading.Tasks;

namespace NvGpuUtilityLib
{
    public class GpuManager
    {
        private static GpuManager instance = null;
        public static GpuManager Initialization()
        {
            if (instance != null)
                return instance;

            NvApiLib.Initialize();
            uint gpuCount = 0;
            uint primaryIndex = 0;
            NvApiLib.GetGpuCountAndPrimaryIndex(out gpuCount, out primaryIndex);
            instance = new GpuManager(gpuCount, primaryIndex);
            return instance;
        }

        public static void UnInitialization()
        {
            instance = null;
            NvApiLib.UnInitialize();
        }

        public uint GpuCount { get; private set; }
        public uint PrimaryGpuIndex { get; private set; }

        /// <summary>
        /// Use static method Initialization to get instance
        /// </summary>
        private GpuManager(uint gpuCount, uint primaryIndex)
        {
            GpuCount = gpuCount;
            PrimaryGpuIndex = primaryIndex;
        }

        public bool GetGpuInformation(uint index, out GpuInfo info)
        {
            StringBuilder name = new StringBuilder(64);
            StringBuilder bios = new StringBuilder(64);
            uint pid = 0;
            uint sid = 0;
            var result = NvApiLib.GetGpuInformation(index, name, bios, out pid, out sid);
            if (result == 0)
                info = new GpuInfo(index, name.ToString(), pid, sid, bios.ToString());
            else
                info = null;
            return result == 0;
        }

        public bool GetPStateInfo(ref GpuInfo info)
        {
            var pstate = new PStateProp();
            // Get PState info editable abilities
            var result = NvApiLib.GetPStatesInfo(info.Index,
                out pstate.PStateInfoEditable,
                out pstate.PStateEditable,
                out pstate.GpuFreqEditable,
                out pstate.MemFreqEditable,
                out pstate.BaseVoltEditable,
                out pstate.OverVoltEditable);
            if (result == 0)
                info.PStateInfo = pstate;
            else
                info.PStateInfo = PStateProp.GetDefault();
            return result == 0;
        }

        public bool GetFreqDelta(ref GpuInfo info)
        {
            // Get current GPU core and memory frequency delta values
            int gpuDelta = 0;
            int memDelta = 0;
            ushort pStateId = 0;
            var result = NvApiLib.GetPStatesInfoGpuFrequencyDelta(info.Index, out gpuDelta, out pStateId);
            if (result == 0)
                info.GpuFreqDelta = (double)gpuDelta / 1000;
            else
                info.GpuFreqDelta = 0;
            result = NvApiLib.GetPStatesInfoMemFrequencyDelta(info.Index, out memDelta, out pStateId);
            if (result == 0)
                info.MemFreqDelta = (double)memDelta / 1000;
            else
                info.MemFreqDelta = 0;
            return result == 0;
        }

        public bool SetFreqDelta(double gpuDelta, double memDelta, ref GpuInfo info)
        {
            if (gpuDelta != info.GpuFreqDelta)
            {
                var result = NvApiLib.SetPStatesInfoGpuFrequencyDelta(info.Index, (int)(gpuDelta * 1000));
                if (result == 0)
                    info.GpuFreqDelta = gpuDelta;
                else
                    return false;
            }
            if (memDelta != info.MemFreqDelta)
            {
                var result = NvApiLib.SetPStatesInfoMemFrequencyDelta(info.Index, (int)(memDelta * 1000));
                if (result == 0)
                    info.MemFreqDelta = memDelta;
                else
                    return false;
            }
            //Set value succeeded or nothing changed
            return true;
        }

        public bool GetCoolerInfo(ref GpuInfo info)
        {
            // Get fan duty
            uint duty = 0;
            uint rpm = 0;
            var result = NvApiLib.GetCoolerSettings(info.Index, 0, out duty, out rpm);
            if (result == 0)
            {
                info.CoolerLevels = duty;
                info.CoolerFanRpm = rpm;
            }
            else
            {
                info.CoolerLevels = 0;
                info.CoolerFanRpm = 0;
            }
            return result == 0;
        }

        /// <summary>
        /// Set cooler levels(fan duty), set auto fan if levels is 0
        /// </summary>
        /// <param name="levels"></param>
        /// <param name="info"></param>
        /// <returns></returns>
        public bool SetCoolerInfo(uint levels, ref GpuInfo info)
        {
            if (levels == 0)
            {
                var result = NvApiLib.SetCoolerLevels(info.Index, 0, false, 0);
                return result == 0;
            }
            if (info.CoolerLevels != levels)
            {
                var result = NvApiLib.SetCoolerLevels(info.Index, 0, true, levels);
                if (result == 0)
                    info.CoolerLevels = levels;
                else
                    return false;
            }
            //Set value succeeded or nothing changed
            return true;
        }

        public bool GetPowerLimit(ref GpuInfo info)
        {
            // Get power limitation
            uint pl = 0;
            uint defpl = 0;
            uint enfpl = 0;
            var result = NvApiLib.GetDevicePowerLimit(info.Index, out pl, out defpl, out enfpl);
            if (result == 0)
            {
                info.PowerLimit = (double)pl / 1000;
                info.DefaultPowerLimit = (double)defpl / 1000;
                info.EnforcedPowerLimit = (double)enfpl / 1000;
            }
            else
            {
                info.PowerLimit = 0;
                info.DefaultPowerLimit = 0;
                info.EnforcedPowerLimit = 0;
            }
            return result == 0;
        }

        /// <summary>
        /// Set power limit must be larger then 0
        /// </summary>
        /// <param name="pl"></param>
        /// <param name="info"></param>
        /// <returns></returns>
        public bool SetPowerLimit(double pl, ref GpuInfo info)
        {
            if (pl <= 0)
                return false;
            if (pl != info.PowerLimit)
            {
                var result = NvApiLib.SetDevicePowerLimit(info.Index, (uint)(pl * 1000));
                if (result == 0)
                    info.PowerLimit = pl;
                else
                    return false;
            }
            //Set value succeeded or nothing changed
            return true;
        }

        public bool GetUtilizationSample(uint index, out uint procId, out uint procUtil)
        {
            uint procCount = 0;
            StringBuilder procName = new StringBuilder(256);
            procCount = 3;
            ProcessUtilizationSample[] samples = new ProcessUtilizationSample[3];
            var result = NvApiLib.GetProcessUtilization(index, out procCount, samples);
            if (result == 0)
            {
                procId = samples[0].pid;
                procUtil = samples[0].smUtil;
            }
            else
            {
                procUtil = 0;
                procId = 0;
            }
            return result == 0;
        }

        public void SaveInfoState(InfoState state)
        {
            // Temperary use lib's log folder
            var path = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData) + "\\NvApiHelper";
            if (Directory.Exists(path) == false)
                Directory.CreateDirectory(path);
            var fileName = state.DisplayName.Replace(' ', '_') + ".json";
            var ser = new DataContractJsonSerializer(typeof(InfoState));
            var stream = new MemoryStream();
            ser.WriteObject(stream, state);
            //show stream json output
            stream.Position = 0;
            var json = "";
            using (var sr = new StreamReader(stream))
                json = sr.ReadToEnd();
            Console.WriteLine(json);
            using (var sw = new StreamWriter(path + "\\" + fileName, false, Encoding.UTF8))
            {
                sw.WriteLine(json);
            }
            stream.Dispose();
        }

        public InfoState LoadInfoState(string displayName)
        {
            // Temperary use lib's log folder
            var path = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData) + "\\NvApiHelper";
            var fileName = displayName.Replace(' ', '_') + ".json";
            if (File.Exists(path + "\\" + fileName) == false)
                return null;

            InfoState result = null;
            using (var fs = new FileStream(path + "\\" + fileName, FileMode.Open, FileAccess.Read))
            {
                MemoryStream stream = new MemoryStream();
                fs.CopyTo(stream);
                stream.Position = 0;
                var ser = new DataContractJsonSerializer(typeof(InfoState));
                result = (InfoState)ser.ReadObject(stream);
                stream.Dispose();
            }
            return result;
        }
    }
}
