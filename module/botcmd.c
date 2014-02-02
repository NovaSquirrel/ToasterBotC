#include "../main/bot.h"
PluginInfo *ph;

int BotCmd_AnyMessage(BotEvent *Event) {
  const char *Prefix = Bot_GetConfigStr("Bot", "Prefix", "!");
  const char *Text = Bot_GetEventStr(Event, "Text", "?");

  char Text2[strlen(Text)+1];
  strcpy(Text2, Text);
  Text = Text2;

  if(Bot_MemCaseCmp(Text, Prefix, strlen(Prefix))) return ER_NORMAL;
  const char *CmdName = Text + strlen(Prefix);
  if(!*CmdName) return ER_NORMAL;

  char *ArgPtr = strchr(CmdName, ' ');
  if(ArgPtr) {
    *(ArgPtr++) = 0;
    while(*ArgPtr && *ArgPtr == ' ') ArgPtr++;
  }

  if(CmdName[0] == '&')
    Bot_ParamCopyEvent(ph, "Async Bot Command", Event->Context, EF_ASYNCHRONOUS, Event, "\tcmdname=%s\t \tcmdarg=%s\t", CmdName, ArgPtr?:"");
  else
    Bot_ParamCopyEvent(ph, "Bot Command", Event->Context, 0, Event, "\tcmdname=%s\t \tcmdarg=%s\t", CmdName, ArgPtr?:"");
  return ER_NORMAL;
}

int BotCmd_BotReply(BotEvent *Event) {
  const char *Reply = Bot_GetEventStr(Event, "Reply", "Bot Reply with missing text");
  char *CommandToUse = "say";
  if(!Bot_MemCaseCmp(Reply, "/me ", 4)) {
    CommandToUse = "me";
    Reply+=4;
  }
  Bot_StartEvent(ph, "Client Command", Event->Context, 0, "%s %s", CommandToUse, Reply);
  return ER_NORMAL;
}

int BotCmd_ShowHelp(BotEvent *Event) {
  const char *HelpSite = Bot_GetConfigStr("Bot", "HelpSite", NULL);
  if(HelpSite)
    Bot_ParamCopyEvent(ph, "Bot Reply", Event->Context, 0, Event, "\treply=Invalid command? Check %s\t", HelpSite);
//  printf("Test: Handling command %s\n", CmdName);
  return ER_NORMAL;
}

int Plugin_Init(PluginInfo *Handle, char **ModName, char **FancyName, char **Version) {
  ph = Handle;
  *ModName = "botcmd";
  *FancyName = "Bot command handler";
  *Version = "0.1";
  if(!Bot_AddEventHook(ph, "Bot Reply", PRI_LOWEST, 0, 0, 0, BotCmd_BotReply)) return 0;
  if(!Bot_AddEventHook(ph, "Channel Message", PRI_LOWEST, 0, 0, 0, BotCmd_AnyMessage)) return 0;
  if(!Bot_AddEventHook(ph, "Private Message", PRI_LOWEST, 0, 0, 0, BotCmd_AnyMessage)) return 0;
  if(!Bot_AddEventHook(ph, "Bot Command", PRI_LOWEST, 0, EF_ALREADY_HANDLED, 0, BotCmd_ShowHelp)) return 0;
  if(!Bot_AddEventHook(ph, "Async Bot Command", PRI_LOWEST, 0, EF_ALREADY_HANDLED, 0, BotCmd_ShowHelp)) return 0;
  return 1;
}

void Plugin_Deinit() {

}
