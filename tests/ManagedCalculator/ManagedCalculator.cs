using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CppPlug;

namespace ManagedCalculator
{
    public class ManagedCalculator
    {
        public static CLoadModuleResult LoadModule()
        {
            Console.WriteLine("Loading managed module");
            return new CLoadModuleResult(ReloadModule, UnloadModule);
        }

        public static int UnloadModule()
        {
            Console.WriteLine("Unloading managed module");
            return 0;
        }

        public static int ReloadModule()
        {
            Console.WriteLine("Reloading managed module");
            return 0;
        }

        public static CPluginInfo[] RegisterPlugins()
        {
            return new CPluginInfo[]
            {
                new CPluginInfo()
            };
        }
    }
}
