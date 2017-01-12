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
        CCreatedPlugin stdCalc;

        Sub(CModuleDependenciesCollection deps)
        {
            CModuleInfo info = deps.Find("StandardCalculator");
            stdCalc = info.CreatePlugin("Sum", IntPtr.Zero);
            stdCalc.SendMessage("sayHi", IntPtr.Zero);
        }

        public static IntPtr Create(IntPtr depsCollection, IntPtr creationData)
        {
            CModuleDependenciesCollection deps = (CModuleDependenciesCollection)Marshal.PtrToStructure(depsCollection, typeof(CModuleDependenciesCollection));

            Sub plugin = new Sub(deps);
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
