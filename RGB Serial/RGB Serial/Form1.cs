using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace RGB_Serial
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
          
            Thread x = new Thread(new ThreadStart(T));
            x.Start();
        }

        void T()
        {
            SerialPort _serialPort = new SerialPort("COM5", 128000, Parity.None, 8, StopBits.One);
            _serialPort.Handshake = Handshake.None;

            try
            {
                if (!(_serialPort.IsOpen))
                    _serialPort.Open();
                bool b = false;
                byte[] toSend = new byte[25 * 3];
                while (true)
                {
                    for (int i = 0; i < 25; i++)
                    {
                        
                        if (b)
                        {
                            toSend[i * 3 + 0] = (byte)i;
                            toSend[i * 3 + 1] = 0x2;
                            toSend[i * 3 + 2] = 0x2;
                            toSend[i * 3 + 0] = 0x2;

                        }
                        else
                        {
                            toSend[i * 3 + 0] = (byte)i;
                            toSend[i * 3 + 1] = 0x1;
                            toSend[i * 3 + 2] = 0x1;
                            toSend[i * 3 + 0] = 0x1;
                        }
                       
                        
                    }
                    b = !b;
                    _serialPort.Write(toSend, 0, toSend.Length);
                    Thread.Sleep(10);
                }

                //MessageBox.Show(_serialPort.ReadChar().ToString() + _serialPort.ReadChar().ToString() + _serialPort.ReadChar().ToString() + _serialPort.ReadChar().ToString());
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error opening/writing to serial port :: " + ex.Message, "Error!");
            }
        }
    }
}
