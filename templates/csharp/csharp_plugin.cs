using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

using CppPlug;

namespace CSharpModule
{
    public struct CSharpPlugin
    {
        CSharpPlugin(CModuleDependenciesCollection dependencies)
        {
        }

        public static IntPtr Create(IntPtr dependenciesCollection, IntPtr creationData)
        {
            CModuleDependenciesCollection dependencies = (CModuleDependenciesCollection)Marshal.PtrToStructure(dependenciesCollection, typeof(CModuleDependenciesCollection));
            return ModuleTools.PluginToPtr<CSharpPlugin>(new CSharpPlugin(dependencies));
        }

        public static void Destroy(IntPtr plugin)
        {
            ModuleTools.FreePlugin(plugin);
        }

        public static bool OnMessage(IntPtr plugin, string messageName, IntPtr messageData)
        {
            CSharpPlugin p = ModuleTools.PtrToPlugin<CSharpPlugin>(plugin);
            return p.OnMessage(messageName, messageData);
        }

        public static string SaveDataForReload(IntPtr plugin)
        {
            CSharpPlugin p = ModuleTools.PtrToPlugin<CSharpPlugin>(plugin);
            
            // Serialize data for the plugin to a file and return its path

            return "";
        }

        public static void LoadDataAfterReload(IntPtr plugin)
        {
            CSharpPlugin p = ModuleTools.PtrToPlugin<CSharpPlugin>(plugin);
            
            // Load the data previously serialized from the path given before
        }

        public bool OnMessage(string messageName, IntPtr messageData)
        {
            // Do something to handle the message

            // Return true to tell the caller that the message has been successfully handled
            return true;
        }
    }
}
