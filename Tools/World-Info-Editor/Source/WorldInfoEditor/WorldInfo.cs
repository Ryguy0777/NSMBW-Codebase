using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MFAWWorldInfo
{
    public class WorldInfos
    {
        public ShopInfo[] shopInfos = new ShopInfo[10];
        public List<WorldInfo> worldInfos = new List<WorldInfo>();

        public WorldInfos() {
            Reset();
        }

        public void Reset()
        {
            for (int i = 0; i < shopInfos.Length; i++) {
                shopInfos[i] = new ShopInfo();
            }

            worldInfos = new List<WorldInfo>();
        }

        public bool loadFile(string path)
        {
            // Reset

            Reset();

            // Opening File

            byte[] file = new byte[0];
            try
            {
                file = File.ReadAllBytes(path);
            }
            catch (Exception e)
            {
                MessageBox.Show("Couldn't load file " + path + "\r\n" + e.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            // Magic

            uint magic = BitConverter.ToUInt32(file.Take(4).Reverse().ToArray(), 0);
            if (magic != 0x57495349) // WISI
            {
                MessageBox.Show("This isn't a valid MFAW World Info File.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            // Parsing Shops

            for (int currentShop = 0; currentShop < 10; currentShop++)
            {
                for (int currentItem = 0; currentItem < 12; currentItem++)
                {
                    shopInfos[currentShop].items[currentItem] = file[4 + (currentShop * 12) + currentItem];
                }
            }

            // Parsing World Info

            uint worldCount = BitConverter.ToUInt32(file.Skip(124).Take(4).Reverse().ToArray(), 0);
            for (int currentWorld = 0; currentWorld < worldCount; currentWorld++)
            {
                WorldInfo worldInfo = new WorldInfo();

                int worldIdx = 128 + (currentWorld * 0x2C);

                worldInfo.worldmapID = file[worldIdx];
                worldInfo.nodeID = file[worldIdx + 1];
                worldInfo.NWID = file[worldIdx + 2];
                worldInfo.musicID = file[worldIdx + 3];

                worldInfo.lastLevelID = file[worldIdx + 4];
                worldInfo.pad0 = file[worldIdx + 5];
                worldInfo.pad1 = file[worldIdx + 6];
                worldInfo.pad2 = file[worldIdx + 7];

                worldInfo.FSTextColour0 = BitConverter.ToUInt32(file.Skip(worldIdx + 8).Take(4).Reverse().ToArray(), 0);
                worldInfo.FSTextColour1 = BitConverter.ToUInt32(file.Skip(worldIdx + 0xC).Take(4).Reverse().ToArray(), 0);
                worldInfo.FSHintColour0 = BitConverter.ToUInt32(file.Skip(worldIdx + 0x10).Take(4).Reverse().ToArray(), 0);
                worldInfo.FSHintColour1 = BitConverter.ToUInt32(file.Skip(worldIdx + 0x14).Take(4).Reverse().ToArray(), 0);
                worldInfo.HUDTextColour0 = BitConverter.ToUInt32(file.Skip(worldIdx + 0x18).Take(4).Reverse().ToArray(), 0);
                worldInfo.HUDTextColour1 = BitConverter.ToUInt32(file.Skip(worldIdx + 0x1C).Take(4).Reverse().ToArray(), 0);

                worldInfo.HUDHSL.h = BitConverter.ToInt16(file.Skip(worldIdx + 0x20).Take(2).Reverse().ToArray(), 0);
                worldInfo.HUDHSL.s = (sbyte)file[worldIdx + 0x22];
                worldInfo.HUDHSL.l = (sbyte)file[worldIdx + 0x23];

                worldInfo.titlescreenWorld = (ushort)(BitConverter.ToUInt16(file.Skip(worldIdx + 0x24).Take(2).Reverse().ToArray(), 0) + 1);
                worldInfo.titlescreenLevel = (ushort)(BitConverter.ToUInt16(file.Skip(worldIdx + 0x26).Take(2).Reverse().ToArray(), 0) + 1);

                uint worldNameIdx = BitConverter.ToUInt32(file.Skip(worldIdx + 0x28).Take(4).Reverse().ToArray(), 0);
                int currentChar = 0;
                worldInfo.worldName = "";
                while (file[worldNameIdx + currentChar] > 0)
                {
                    worldInfo.worldName += (char)file[worldNameIdx + currentChar];
                    currentChar++;
                }

                worldInfos.Add(worldInfo);
            }

            return true;
        }

        public byte[] saveAsBytes()
        {
            List<byte> file = new List<byte>();

            // Magic

            file.Add(0x57);
            file.Add(0x49);
            file.Add(0x53);
            file.Add(0x49);

            // Saving Shops

            for (int currentShop = 0; currentShop < 10; currentShop++)
            {
                for (int currentItem = 0; currentItem < 12; currentItem++)
                {
                    file.Add(shopInfos[currentShop].items[currentItem]);
                }
            }

            //Saving World Info

            file.Add((byte)((worldInfos.Count & 0xFF000000) >> 24));
            file.Add((byte)((worldInfos.Count & 0x00FF0000) >> 16));
            file.Add((byte)((worldInfos.Count & 0x0000FF00) >> 8));
            file.Add((byte)((worldInfos.Count & 0x000000FF)));

            foreach (WorldInfo worldInfo in worldInfos)
            {
                file.Add(worldInfo.worldmapID);
                file.Add(worldInfo.nodeID);
                file.Add(worldInfo.NWID);
                file.Add(worldInfo.musicID);

                file.Add(worldInfo.lastLevelID);
                file.Add(worldInfo.pad0);
                file.Add(worldInfo.pad1);
                file.Add(worldInfo.pad2);

                file.Add((byte)((worldInfo.FSTextColour0 & 0xFF000000) >> 24));
                file.Add((byte)((worldInfo.FSTextColour0 & 0x00FF0000) >> 16));
                file.Add((byte)((worldInfo.FSTextColour0 & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.FSTextColour0 & 0x000000FF)));

                file.Add((byte)((worldInfo.FSTextColour1 & 0xFF000000) >> 24));
                file.Add((byte)((worldInfo.FSTextColour1 & 0x00FF0000) >> 16));
                file.Add((byte)((worldInfo.FSTextColour1 & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.FSTextColour1 & 0x000000FF)));

                file.Add((byte)((worldInfo.FSHintColour0 & 0xFF000000) >> 24));
                file.Add((byte)((worldInfo.FSHintColour0 & 0x00FF0000) >> 16));
                file.Add((byte)((worldInfo.FSHintColour0 & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.FSHintColour0 & 0x000000FF)));

                file.Add((byte)((worldInfo.FSHintColour1 & 0xFF000000) >> 24));
                file.Add((byte)((worldInfo.FSHintColour1 & 0x00FF0000) >> 16));
                file.Add((byte)((worldInfo.FSHintColour1 & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.FSHintColour1 & 0x000000FF)));

                file.Add((byte)((worldInfo.HUDTextColour0 & 0xFF000000) >> 24));
                file.Add((byte)((worldInfo.HUDTextColour0 & 0x00FF0000) >> 16));
                file.Add((byte)((worldInfo.HUDTextColour0 & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.HUDTextColour0 & 0x000000FF)));

                file.Add((byte)((worldInfo.HUDTextColour1 & 0xFF000000) >> 24));
                file.Add((byte)((worldInfo.HUDTextColour1 & 0x00FF0000) >> 16));
                file.Add((byte)((worldInfo.HUDTextColour1 & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.HUDTextColour1 & 0x000000FF)));

                file.Add((byte)((worldInfo.HUDHSL.h & 0x0000FF00) >> 8));
                file.Add((byte)((worldInfo.HUDHSL.h & 0x000000FF)));
                file.Add((byte)worldInfo.HUDHSL.s);
                file.Add((byte)worldInfo.HUDHSL.l);

                file.Add((byte)(((worldInfo.titlescreenWorld-1) & 0x0000FF00) >> 8));
                file.Add((byte)(((worldInfo.titlescreenWorld-1) & 0x000000FF)));

                file.Add((byte)(((worldInfo.titlescreenLevel-1) & 0x0000FF00) >> 8));
                file.Add((byte)(((worldInfo.titlescreenLevel-1) & 0x000000FF)));

                // Space for World Name IDX
                file.Add(0);
                file.Add(0);
                file.Add(0);
                file.Add(0);
            }

            for(int i = 0; i < worldInfos.Count; i++)
            {
                uint currentIdx = (uint)file.Count;
                file[128 + (i * 0x2C) + 0x28 + 0] = (byte)((currentIdx & 0xFF000000) >> 24);
                file[128 + (i * 0x2C) + 0x28 + 1] = (byte)((currentIdx & 0x00FF0000) >> 16);
                file[128 + (i * 0x2C) + 0x28 + 2] = (byte)((currentIdx & 0x0000FF00) >> 8);
                file[128 + (i * 0x2C) + 0x28 + 3] = (byte)((currentIdx & 0x000000FF));

                foreach(char c in worldInfos[i].worldName)
                {
                    file.Add((byte)c);
                }

                file.Add(00);
            }

            return file.ToArray();
        }
    }

    public class WorldInfo
    {
        public byte worldmapID = 0;
        public byte nodeID = 0;
        public byte NWID = 0;
        public byte musicID = 0;

        public byte lastLevelID = 0;
        public byte pad0, pad1, pad2;

        public uint FSTextColour0 = 0xFF;
        public uint FSTextColour1 = 0xFF;
        public uint FSHintColour0 = 0xFF;
        public uint FSHintColour1 = 0xFF;
        public uint HUDTextColour0 = 0xFF;
        public uint HUDTextColour1 = 0xFF;

        public HSLColor HUDHSL = new HSLColor();

        public ushort titlescreenWorld = 1;
        public ushort titlescreenLevel = 1;

        public string worldName = "";
    }

    public class ShopInfo
    {
        public byte[] items = new byte[12] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    }

    public class HSLColor
    {
        public short h = 0;
        public sbyte s = 0;
        public sbyte l = 0;
    }
}
