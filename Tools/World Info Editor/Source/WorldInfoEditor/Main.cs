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

namespace MFAWWorldInfo
{
    public partial class Main : Form
    {
        public Main()
        {
            InitializeComponent();
        }

        WorldInfos worldInformations = new WorldInfos();
        string path = "";

        private void Main_Load(object sender, EventArgs e)
        {
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog dialog = new OpenFileDialog())
            {
                string output;
                dialog.Filter = "World Info File|*.bin|All files (*.*)|*.*";
                dialog.FilterIndex = 1;
                dialog.RestoreDirectory = true;
                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    output = dialog.FileName;
                    if (worldInformations.loadFile(output))
                    {
                        refreshListBox(1);
                        shopNumberComboBox.SelectedIndex = 0;
                        mainTabControl.Enabled = true;
                        path = output;
                        this.Text = "World Info Editor (NSMBW-Codebase) - " + Path.GetFileName(output);

                        saveToolStripMenuItem.Enabled = true;
                        saveAsToolStripMenuItem.Enabled = true;
                    }
                }
            }
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            File.WriteAllBytes(path, worldInformations.saveAsBytes());
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog textDialog;
            textDialog = new SaveFileDialog();
            textDialog.Filter = "World Info File|*.bin|All files (*.*)|*.*";
            textDialog.DefaultExt = "bin";
            if (textDialog.ShowDialog() == DialogResult.OK)
            {
                //Stream things to get the saved path
                System.IO.Stream fileStream = textDialog.OpenFile();
                System.IO.StreamWriter sw = new System.IO.StreamWriter(fileStream);
                string outputPath = ((FileStream)(sw.BaseStream)).Name;
                sw.Close();
                path = outputPath;
                this.Text = "World Info Editor (NSMBW-Codebase) - " + Path.GetFileName(outputPath);

                //Save it
                saveToolStripMenuItem_Click(sender, e);
            }
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("World Info & Shops Editor (NSMBW-Codebase version)\r\nOriginally made by Asu-chan for MFAW.\r\n"
                + "Build Date (DD/MM/AAAA HH:mm): " + Properties.Resources.BuildDate + "\r\n\r\n"
                + Properties.Resources.Version, "About", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        // World Info

        public void refreshListBox(int sel)
        {
            worldsListBox.Items.Clear();
            for (int i = 0; i < worldInformations.worldInfos.Count; i++)
            {
                worldsListBox.Items.Add("World " + i);
            }
            worldsListBox.SelectedIndex = sel - 1;
        }

        bool canWorldInteract = true;

        private void worldsListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            int selectedWorld = worldsListBox.SelectedIndex;

            canWorldInteract = false;

            worldmapIDNumUpDown.Value = worldInformations.worldInfos[selectedWorld].worldmapID;
            nodeIDNumUpDown.Value = worldInformations.worldInfos[selectedWorld].nodeID;
            NWIDNumUpDown.Value = worldInformations.worldInfos[selectedWorld].NWID;
            musicIDNumUpDown.Value = worldInformations.worldInfos[selectedWorld].musicID;
            lastLevelNumUpDown.Value = worldInformations.worldInfos[selectedWorld].lastLevelID;

            FSTextColour0TextBox.Text = worldInformations.worldInfos[selectedWorld].FSTextColour0.ToString("X8");
            FSTextColour1TextBox.Text = worldInformations.worldInfos[selectedWorld].FSTextColour1.ToString("X8");
            FSHintColour0TextBox.Text = worldInformations.worldInfos[selectedWorld].FSHintColour0.ToString("X8");
            FSHintColour1TextBox.Text = worldInformations.worldInfos[selectedWorld].FSHintColour1.ToString("X8");
            HUDTextColour0TextBox.Text = worldInformations.worldInfos[selectedWorld].HUDTextColour0.ToString("X8");
            HUDTextColour1TextBox.Text = worldInformations.worldInfos[selectedWorld].HUDTextColour1.ToString("X8");

            HUDHNumUpDown.Value = worldInformations.worldInfos[selectedWorld].HUDHSL.h;
            HUDSNumUpDown.Value = worldInformations.worldInfos[selectedWorld].HUDHSL.s;
            HUDLNumUpDown.Value = worldInformations.worldInfos[selectedWorld].HUDHSL.l;

            titlescreenWNumUpDown.Value = worldInformations.worldInfos[selectedWorld].titlescreenWorld;
            titlescreenLNumUpDown.Value = worldInformations.worldInfos[selectedWorld].titlescreenLevel;

            worldNameTextBox.Text = worldInformations.worldInfos[selectedWorld].worldName;

            canWorldInteract = true;
        }

        private void addButton_Click(object sender, EventArgs e)
        {
            worldInformations.worldInfos.Add(new WorldInfo());
            refreshListBox(worldInformations.worldInfos.Count);
            mainTabControl.Enabled = true;
            saveAsToolStripMenuItem.Enabled = true;
            if (path != "")
            {
                saveToolStripMenuItem.Enabled = true;
            }
        }

        private void removeButton_Click(object sender, EventArgs e)
        {
            if(worldInformations.worldInfos.Count > 1)
            {
                worldInformations.worldInfos.RemoveAt(worldsListBox.SelectedIndex);
                refreshListBox(worldsListBox.SelectedIndex);
                if (worldInformations.worldInfos.Count == 0)
                {
                    mainTabControl.Enabled = false;
                    saveToolStripMenuItem.Enabled = false;
                    saveToolStripMenuItem.Enabled = false;
                }
            }
        }

        public void saveWorldInfoContents()
        {
            if (canWorldInteract)
            {
                int selectedWorld = worldsListBox.SelectedIndex;

                worldInformations.worldInfos[selectedWorld].worldmapID = (byte)worldmapIDNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].nodeID = (byte)nodeIDNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].NWID = (byte)NWIDNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].musicID = (byte)musicIDNumUpDown.Value;

                worldInformations.worldInfos[selectedWorld].lastLevelID = (byte)lastLevelNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].pad0 = 0;
                worldInformations.worldInfos[selectedWorld].pad1 = 0;
                worldInformations.worldInfos[selectedWorld].pad2 = 0;

                worldInformations.worldInfos[selectedWorld].FSTextColour0 = Convert.ToUInt32(FSTextColour0TextBox.Text, 16);
                worldInformations.worldInfos[selectedWorld].FSTextColour1 = Convert.ToUInt32(FSTextColour1TextBox.Text, 16);
                worldInformations.worldInfos[selectedWorld].FSHintColour0 = Convert.ToUInt32(FSHintColour0TextBox.Text, 16);
                worldInformations.worldInfos[selectedWorld].FSHintColour1 = Convert.ToUInt32(FSHintColour1TextBox.Text, 16);
                worldInformations.worldInfos[selectedWorld].HUDTextColour0 = Convert.ToUInt32(HUDTextColour0TextBox.Text, 16);
                worldInformations.worldInfos[selectedWorld].HUDTextColour1 = Convert.ToUInt32(HUDTextColour1TextBox.Text, 16);

                worldInformations.worldInfos[selectedWorld].HUDHSL.h = (short)HUDHNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].HUDHSL.s = (sbyte)HUDSNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].HUDHSL.l = (sbyte)HUDLNumUpDown.Value;

                worldInformations.worldInfos[selectedWorld].titlescreenWorld = (ushort)titlescreenWNumUpDown.Value;
                worldInformations.worldInfos[selectedWorld].titlescreenLevel = (ushort)titlescreenLNumUpDown.Value;

                worldInformations.worldInfos[selectedWorld].worldName = worldNameTextBox.Text;
            }
        }

        private void worldInfoNumUpDown_ValueChanged(object sender, EventArgs e)
        {
            saveWorldInfoContents();
        }

        List<char> hexChars = new List<char>(new char[] { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f' });
        string prevValue = "";

        private void filterHexColorTextBox(object sender)
        {
            if ((sender as TextBox).Text.Any(x => !hexChars.Contains(x)))
            {
                (sender as TextBox).Text = prevValue;
                MessageBox.Show("Please input an hex color code in the RRGGBBAA format.", "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void colorTextBoxes_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            prevValue = (sender as TextBox).Text;
            if (!hexChars.Contains(e.KeyChar) && e.KeyChar >= 0x20)
            {
                MessageBox.Show("Please input an hex color code in the RRGGBBAA format.", "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
                e.Handled = true;
            }
        }

        private void colourTextBoxes_TextChanged(object sender, EventArgs e)
        {
            filterHexColorTextBox(sender);

            saveWorldInfoContents();
        }

        private void worldNameTextBox_TextChanged(object sender, EventArgs e)
        {
            saveWorldInfoContents();
        }

        // Shops

        bool canShopInteract = true;

        private void shopNumberComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            int selectedShop = shopNumberComboBox.SelectedIndex;

            canShopInteract = false;
            item1ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[0];
            item2ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[1];
            item3ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[2];
            item4ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[3];
            item5ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[4];
            item6ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[5];
            item7ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[6];
            item8ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[7];
            item9ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[8];
            item10ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[9];
            item11ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[10];
            item12ComboBox.SelectedIndex = worldInformations.shopInfos[selectedShop].items[11];
            canShopInteract = true;
        }

        private void itemComboBoxes_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (canShopInteract)
            {
                int selectedShop = shopNumberComboBox.SelectedIndex;

                worldInformations.shopInfos[selectedShop].items[0] = (byte)item1ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[1] = (byte)item2ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[2] = (byte)item3ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[3] = (byte)item4ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[4] = (byte)item5ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[5] = (byte)item6ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[6] = (byte)item7ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[7] = (byte)item8ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[8] = (byte)item9ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[9] = (byte)item10ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[10] = (byte)item11ComboBox.SelectedIndex;
                worldInformations.shopInfos[selectedShop].items[11] = (byte)item12ComboBox.SelectedIndex;
            }
        }
    }
}
