#include "../main/bot.h"
PluginInfo *ph;

int Plugin_Init(PluginInfo *Handle, char **ModName, char **FancyName, char **Version) {
  ph = Handle;
  *ModName = "test";
  *FancyName = "Test Plugin";
  *Version = "0.1";
  puts("test plugin initialized");
  return 1;
}
void Plugin_Deinit() {
  puts("test plugin deinitialized");
} 
