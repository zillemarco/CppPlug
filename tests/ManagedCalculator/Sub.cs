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
            return ModuleTools.PluginToPtr<Sub>(new Sub(deps));
        }

        public static void Destroy(IntPtr plugin)
        {
            Console.WriteLine("Free the managed plugin");
            ModuleTools.FreePlugin(plugin);
        }

        public static bool OnMessage(IntPtr plugin, string messageName, IntPtr messageData)
        {
            Sub p = ModuleTools.PtrToPlugin<Sub>(plugin);
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
