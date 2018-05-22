using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace NvGpuUtilityLib
{
    /// <summary>
    /// Subset of GpuInfo for saving/loading state to json file
    /// </summary>
    [DataContract]
    public class InfoState
    {
        [DataMember]
        public string DisplayName { get; set; }
        [DataMember]
        public double GpuFreqDelta { get; set; }
        [DataMember]
        public double MemFreqDelta { get; set; }
        [DataMember]
        public int CoolerLevels { get; set; }
        [DataMember]
        public double PowerLimit { get; set; }
    }
}
