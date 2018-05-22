using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using NvGpuUtilityLib;

namespace NvGpuUtility
{
    public partial class Form1 : Form
    {
        private static List<GpuInfo> gpuInfoList = new List<GpuInfo>();
        private static GpuManager manager;
        private static bool isAutoLoad = false;
        private static bool hasAutoLoaded = false;

        private void UpdateValues(double gpu, double mem, uint duty, double pwr, ref GpuInfo gpuInfo)
        {
            var result = manager.SetFreqDelta(gpu, mem, ref gpuInfo);
            applyResultLabel.Text = "freq:" + result.ToString();

            result = manager.SetCoolerInfo(duty, ref gpuInfo);
            applyResultLabel.Text += " duty:" + result.ToString();

            result = manager.SetPowerLimit(pwr, ref gpuInfo);
            applyResultLabel.Text += " pwr:" + result.ToString();
            
            // Update input shows apply succeeded value or failed original value
            textBox1.Text = gpuInfo.GpuFreqDelta.ToString();
            textBox2.Text = gpuInfo.MemFreqDelta.ToString();
            textBox3.Text = gpuInfo.CoolerLevels.ToString();
            textBox4.Text = gpuInfo.PowerLimit.ToString();
        }

        public Form1()
        {
            var args = Environment.GetCommandLineArgs();
            if (args.Contains("-a") || args.Contains("--auto"))
                isAutoLoad = true;

            InitializeComponent();

            gpuInfoList.Clear();
            gpuInfoListComboBox.Items.Clear();
            manager = GpuManager.Initialization();
            for (uint i = 0; i < manager.GpuCount; i++)
            {
                GpuInfo gpuInfo = null;
                var result = manager.GetGpuInformation(i, out gpuInfo);
                if (result == false)
                    continue;
                manager.GetPStateInfo(ref gpuInfo);
                manager.GetFreqDelta(ref gpuInfo);
                manager.GetCoolerInfo(ref gpuInfo);
                manager.GetPowerLimit(ref gpuInfo);
                        
                gpuInfoList.Add(gpuInfo);
                gpuInfoListComboBox.Items.Add(gpuInfo.DisplayName);

                //Test get highest utilization of process id
                uint procId = 0;
                uint procUtil = 0;
                if (manager.GetUtilizationSample(gpuInfo.Index, out procId, out procUtil) == true)
                    utilResultLabel.Text = string.Format("proc id:{0} util:{1}", procId, procUtil);
                else
                    utilResultLabel.Text = "";
            }
            
            Timer timer = new Timer();
            timer.Interval = 2000;
            timer.Tick += (sender, e) =>
            {
                if (gpuInfoListComboBox.SelectedIndex < 0)
                    return;
                if (isAutoLoad == true && hasAutoLoaded == false)
                {
                    for (int i = 0; i < gpuInfoList.Count; i++)
                    {
                        var info = gpuInfoList[i];
                        var state = manager.LoadInfoState(info.DisplayName);
                        UpdateValues(state.GpuFreqDelta, state.MemFreqDelta, (uint)state.CoolerLevels, state.PowerLimit, ref info);
                    }
                    hasAutoLoaded = true;
                }

                var gpuInfo = gpuInfoList[gpuInfoListComboBox.SelectedIndex];
                uint procId = 0;
                uint procUtil = 0;
                if (manager.GetUtilizationSample(gpuInfo.Index, out procId, out procUtil) == true)
                    utilResultLabel.Text = string.Format("proc id:{0} util:{1}", procId, procUtil);
                else
                    utilResultLabel.Text = "";
            };
            timer.Start();

            gpuInfoListComboBox.SelectedIndexChanged += (sender, e) =>
            {
                var gpuInfo = gpuInfoList[gpuInfoListComboBox.SelectedIndex];
                pstateInfoLabel.Text = gpuInfo.DisplayPStateInfo;
                textBox1.Text = gpuInfo.GpuFreqDelta.ToString();
                textBox2.Text = gpuInfo.MemFreqDelta.ToString();
                textBox3.Text = gpuInfo.CoolerLevels.ToString();
                textBox4.Text = gpuInfo.PowerLimit.ToString();
                label4.Text = "pwr " + gpuInfo.DefaultPowerLimit.ToString();
            };

            if (gpuInfoListComboBox.Items.Count > 0)
                gpuInfoListComboBox.SelectedIndex = 0;

            applyButton.Click += (sender, e) =>
            {
                if (gpuInfoListComboBox.Items.Count == 0 || gpuInfoListComboBox.SelectedIndex < 0)
                    return;

                int gpu = 0;
                int mem = 0;
                uint duty = 0;
                uint pwr = 0;
                if (int.TryParse(textBox1.Text, out gpu) == true &&
                    int.TryParse(textBox2.Text, out mem) == true &&
                    uint.TryParse(textBox3.Text, out duty) == true &&
                    uint.TryParse(textBox4.Text, out pwr) == true)
                {
                    var gpuInfo = gpuInfoList[gpuInfoListComboBox.SelectedIndex];
                    UpdateValues(gpu, mem, duty, pwr, ref gpuInfo);

                    var state = new InfoState();
                    state.DisplayName = gpuInfo.DisplayName;
                    state.GpuFreqDelta = gpuInfo.GpuFreqDelta;
                    state.MemFreqDelta = gpuInfo.MemFreqDelta;
                    state.CoolerLevels = (int)gpuInfo.CoolerLevels;
                    state.PowerLimit = gpuInfo.PowerLimit;
                    manager.SaveInfoState(state);
                }
            };

            button1.Click += (sender, e) =>
            {
                try
                {
                    var searcher = new System.Management.ManagementObjectSearcher("root\\CIMV2", "select * from Win32_Processor");
                    var objs = searcher.Get();
                    foreach (var obj in objs)
                    {
                        var obja = obj.Properties.Count;
                        foreach (var prop in obj.Properties)
                        {
                            if (prop.Name.Equals("Name"))
                                MessageBox.Show(prop.Value.ToString());
                        }
                        var objb = obj.GetPropertyValue("Name");
                    }
                    var cpuObject = new System.Management.ManagementObjectSearcher("root\\CIMV2", "select * from Win32_Processor")
                    .Get()
                    .Cast<System.Management.ManagementObject>()
                    .First();
                    string fullName = cpuObject["Name"] as string;
                    MessageBox.Show(fullName);
                }
                catch(Exception ex)
                {
                    MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
                }
                
            };
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);
            GpuManager.UnInitialization();
        }
    }

}
