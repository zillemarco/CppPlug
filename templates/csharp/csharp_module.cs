using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CppPlug;

namespace CSharpModule
{
    public class CSharpModule
    {
        public static CLoadModuleResult LoadModule()
        {
            return new CLoadModuleResult(ReloadModule, UnloadModule);
        }

        public static int UnloadModule()
        {
            return 0;
        }

        public static int ReloadModule()
        {
            return 0;
        }

        public static CPluginInfo[] RegisterPlugins()
        {
            return new CPluginInfo[]
            {
                new CPluginInfo("CSharpPlugin", "<plugin_service_name>", CSharpPlugin.Create, CSharpPlugin.Destroy, CSharpPlugin.OnMessage, CSharpPlugin.SaveDataForReload, CSharpPlugin.LoadDataAfterReload)
            };
        }
    }
}
