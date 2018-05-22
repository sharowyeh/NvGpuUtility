using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NvGpuUtilityLib
{
    public class GpuInfo
    {
        public uint Index { get; private set; }
        public string Name { get; private set; }
        public uint PID { get; private set; }
        public uint SID { get; private set; }
        public string BIOS { get; private set; }
        public string DisplayName
        {
            get
            {
                return string.Format("GPU{0} - {1} {2:X} {3:X} {4}", Index, Name, PID, SID, BIOS);
            }
        }
        public PStateProp PStateInfo { get; set; }
        public string DisplayPStateInfo
        {
            get
            {
                return string.Format("PState Info:{0},State:{1},GpuFreq:{2},MemFreq:{3},BaseVolt:{4},OverVolt:{5}",
                    PStateInfo.PStateInfoEditable,
                    PStateInfo.PStateEditable,
                    PStateInfo.GpuFreqEditable,
                    PStateInfo.MemFreqEditable,
                    PStateInfo.BaseVoltEditable,
                    PStateInfo.OverVoltEditable);
            }
        }
        public double GpuFreqDelta { get; set; }
        public double MemFreqDelta { get; set; }
        public uint CoolerLevels { get; set; }
        public uint CoolerFanRpm { get; set; }
        public double PowerLimit { get; set; }
        public double DefaultPowerLimit { get; set; }
        public double EnforcedPowerLimit { get; set; }
        public GpuInfo(uint index, string name, uint pid, uint sid, string bios)
        {
            Index = index;
            Name = name;
            PID = pid;
            SID = sid;
            BIOS = bios;
        }
    }

    public struct PStateProp
    {
        public bool PStateInfoEditable;
        public bool PStateEditable;
        public bool GpuFreqEditable;
        public bool MemFreqEditable;
        public bool BaseVoltEditable;
        public bool OverVoltEditable;
        public static PStateProp GetDefault()
        {
            var pstate = new PStateProp();
            pstate.PStateInfoEditable = false;
            pstate.PStateEditable = false;
            pstate.GpuFreqEditable = false;
            pstate.MemFreqEditable = false;
            pstate.BaseVoltEditable = false;
            pstate.OverVoltEditable = false;
            return pstate;
        }
    }
}
