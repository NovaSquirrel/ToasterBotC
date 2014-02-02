#include "../main/bot.h"
PluginInfo *ph;

void Admin_CrashPlace(const char *put) {
  char Text[1024]="";
  char Temp[1024]="";
  NetworkInfo *Net = (NetworkInfo*)Bot_GetGlobalPointer("FirstNetwork");
  for(;Net;Net=Net->Next) {
    sprintf(Temp, "(%s): ", Net->Tag);
    strcat(Text, Temp);
    for(ChannelInfo *Chan = Net->FirstChannel;Chan;Chan=Chan->Next) {
      sprintf(Temp, "%s ", Chan->Name);
      strcat(Text, Temp);
    }
  }
  printf("%s %s\n", put, Text);
}

int Admin_BotCommand(BotEvent *Event) {
  const char *CmdName = Bot_GetEventStr(Event, "CmdName", "?");
  const char *FullHost = Bot_GetEventStr(Event, "FullHost", "?");
  const char *ArgPtr = Bot_GetEventStr(Event, "CmdArg", "");

  if(!strcasecmp(CmdName, "Admin")) {
    ConfigPair *FirstConfig = (ConfigPair*)Bot_GetGlobalPointer("FirstConfig");
    ConfigPair *Pair = Bot_FindConfigPair(FirstConfig, "Admin");

    if(Pair) {
      Pair = Pair->Item;
      while(Pair) {
        if(!strcasecmp(Pair->Name, "Host") && Bot_WildMatch(FullHost, Pair->Value)) {
          Admin_CrashPlace("before cmd");
          Bot_StartEvent(ph, "Client Command", Event->Context, 0, "%s", ArgPtr);
          Admin_CrashPlace("after cmd");
          return ER_HANDLED;
        }
        Pair=Pair->Next;
      }
    } else {
      fprintf(stderr, "No [Admin] group found\n");
    }
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Hostmask isn't authorized\t");
    return ER_HANDLED;
  }
  return ER_NORMAL;
}

int Plugin_Init(PluginInfo *Handle, char **ModName, char **FancyName, char **Version) {
  ph = Handle;
  *ModName = "admin";
  *FancyName = "Administration";
  *Version = "0.1";
  if(!Bot_AddEventHook(ph, "Bot Command", PRI_NORMAL, 0, EF_ALREADY_HANDLED, 0, Admin_BotCommand)) return 0;
  return 1;
}

void Plugin_Deinit() {

}
