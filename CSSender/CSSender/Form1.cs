using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Net;
using System.Threading;
using System.Windows.Forms;
using System.Net.Sockets;

namespace CSSender
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        void T()
        {
            TcpClient ourMagicClient = new TcpClient();

            //Connect to the server - change this IP address to match your server's IP!
            ourMagicClient.Connect("192.168.1.42", 80);

            //Use a NetworkStream object to send and/or receive some data
            NetworkStream ourStream = ourMagicClient.GetStream();
            bool a = false;
            byte[] data=new byte[25*3];
            while (true)
            {
                for (int i = 0; i < 25; i++)
                {
                    
                    //Let's set up some data!
                    if (a)
                    {
                        data[i * 3 + 0] = 0x40;
                        data[i * 3 + 1] = 0x40;
                        data[i * 3 + 2] = 0x40;
                    }
                    else
                    {
                        data[i * 3 + 0] = 0x00;
                        data[i * 3 + 1] = 0x00;
                        data[i * 3 + 2] = 0x00;
                        
                    }
                    //Everyone ready? Send that bad-boy!
                     //Start at the 0'th position in our string and send until the end of the string, but we can stop there...
                    
                }

                ourStream.Write(data, 0, data.Length);
                Thread.Sleep(10);
                a = !a;
            }
            
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Thread t = new Thread(new ThreadStart(T));
            t.Start();
        }
    }
}
