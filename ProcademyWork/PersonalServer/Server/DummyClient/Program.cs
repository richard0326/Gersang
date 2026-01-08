using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace DummyClient
{
    class Program
    {
        static void Main(string[] args)
        {
            //string host = Dns.GetHostName();
            //IPHostEntry ipHost = Dns.GetHostEntry(host);
            //IPAddress ipAddr = ipHost.AddressList[0];
            //IPEndPoint endPoint = new IPEndPoint(ipAddr, 7777);
            IPEndPoint endPoint = new IPEndPoint(IPAddress.Loopback, 7777);

            Socket socket =  new Socket(endPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            try
            {
                socket.Connect(endPoint);
                Console.WriteLine($"Connected To {socket.RemoteEndPoint.ToString()}");

                // 보낸다.
                byte[] sendBuf = Encoding.UTF8.GetBytes("Hello World");
                int sendbytes = socket.Send(sendBuf);

                // 받는다.
                byte[] recvBuf = new byte[1024];
                int recvBytes = socket.Receive(recvBuf);
                string recvData = Encoding.UTF8.GetString(recvBuf, 0, recvBytes);
                Console.WriteLine($"[From Server] {recvData}");

                socket.Shutdown(SocketShutdown.Both);
                socket.Close();
            }
            catch(Exception e)
            {
                Console.WriteLine(e.ToString());
            }           
        }
    }
}
