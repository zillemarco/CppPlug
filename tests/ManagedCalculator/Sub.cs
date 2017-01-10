using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

using CppPlug;

namespace ManagedCalculator
{
    public struct Sub
    {
        public static IntPtr Create(IntPtr deps, int depsCount, IntPtr creationData)
        {
            Sub plugin = new Sub();
            IntPtr pnt = Marshal.AllocHGlobal(Marshal.SizeOf(plugin));

            Marshal.StructureToPtr(plugin, pnt, false);

            return pnt;
        }

        public static void Destroy(IntPtr plugin)
        {
            Console.WriteLine("Free the managed plugin");
            Marshal.FreeHGlobal(plugin);
        }

        public static bool OnMessage(IntPtr plugin, string messageName, IntPtr messageData)
        {
            Sub p = (Sub)Marshal.PtrToStructure(plugin, typeof(Sub));
            return p.OnMessage(messageName, messageData);
        }

        public bool OnMessage(string messageName, IntPtr messageData)
        {
            if (messageName == "SaySomething")
            {
                string str = Marshal.PtrToStringAnsi(messageData);
                Console.WriteLine(String.Format("Message received: {0}", str));
            }
            
            return true;
        }
    }
}
