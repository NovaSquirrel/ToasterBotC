#include "../main/bot.h"
#include <fnmatch.h>
#include <signal.h>
#include <execinfo.h>

/* global variables of bot state */
NetworkInfo *FirstNetwork = NULL;
EventHook *FirstEvent = NULL;
PluginInfo *FirstPlugin = NULL;
ConfigPair *FirstConfig = NULL;
AsyncEventInfo *FirstAsyncEvent = NULL;
SDLNet_SocketSet SocketSet;
PluginInfo CorePlugin;

char *ConfigPath = "bot.ini";
int RecurseLevel = 0;

int NeedShutdown = 0;
int RebootAfterShutdown = 0;

/* core functions */
void Bot_StrCpy(char *Destination, const char *Source, int MaxLength) {
  // MaxLength is directly from sizeof() so it includes the zero
  int SourceLen = strlen(Source);
  if((SourceLen+1) < MaxLength)
    MaxLength = SourceLen + 1;
  memcpy(Destination, Source, MaxLength-1);
  Destination[MaxLength-1] = 0;
}

int Bot_MemCaseCmp(const char *Text1, const char *Text2, int Length) {
  for(;Length;Length--)
    if(tolower(*(Text1++)) != tolower(*(Text2++)))
      return 1;
  return 0;
}

void *Bot_GetGlobalPointer(const char *Name) {
  if(!strcasecmp(Name, "FirstNetwork")) return FirstNetwork;
  else if(!strcasecmp(Name, "FirstConfig")) return FirstConfig;
  else if(!strcasecmp(Name, "FirstPlugin")) return FirstPlugin;
  return NULL;
}

int Bot_WildMatch(const char *TestMe, const char *Wild) {
  char NewWild[strlen(Wild)+1];
  char NewTest[strlen(TestMe)+1];
  strcpy(NewTest, TestMe);
  strcpy(NewWild, Wild);
  int i;
  for(i=0;NewWild[i];i++)
    NewWild[i] = tolower(NewWild[i]);
  for(i=0;NewTest[i];i++)
    NewTest[i] = tolower(NewTest[i]);
  return !fnmatch(NewWild, NewTest, FNM_NOESCAPE);
}

PluginInfo *Bot_LoadPlugin(const char *Name) {
  for(PluginInfo *P = FirstPlugin;P;P=P->Next) {
    // don't load plugins that are already loaded
    if(!strcasecmp(P->Path, Name))
      return P;
  }
  void *Module;
  int (*Init)(PluginInfo *NewPlugin, char **Name, char **FancyName, char **Version);
  PluginInfo *NewPlugin = (PluginInfo *)malloc(sizeof(PluginInfo));
  Module = dlopen(Name, RTLD_NOW);
  if(Module == NULL) {
    fprintf(stderr, "Can't open plugin \"%s\" - %s\n", Name, dlerror()?:"");
    free(NewPlugin);
    return NULL;
  }
  for(int i=0;i<NUM_TIMERS;i++)
    NewPlugin->Timers[i] = NULL;

  NewPlugin->Prev = NULL;
  NewPlugin->Next = NULL;
  strcpy(NewPlugin->Path, Name);
  NewPlugin->DL = Module;
  *NewPlugin->Tag = 0;
  NewPlugin->ChannelPluginData_Free = NULL;
  NewPlugin->ChannelPluginData_Alloc = NULL;

  Init = dlsym(Module, "Plugin_Init");
  if(!Init) {
    fprintf(stderr, "Can't find Plugin_Init() in plugin \"%s\"- %s\n", Name, dlerror()?:"");
    free(NewPlugin);
    dlclose(Module);
    return NULL;
  }

  char *PluginName = NULL, *PluginFancyName = NULL, *PluginVersion = NULL;
  if(!Init(NewPlugin, &PluginName, &PluginFancyName, &PluginVersion)) {
    Bot_UnloadPlugin(NewPlugin);
    dlclose(Module);
    return NULL;
  }
  if(PluginName) Bot_StrCpy(NewPlugin->Name, PluginName, sizeof(NewPlugin->Name));
  if(PluginFancyName) Bot_StrCpy(NewPlugin->FancyName, PluginFancyName, sizeof(NewPlugin->FancyName));
  if(PluginVersion) Bot_StrCpy(NewPlugin->Version, PluginVersion, sizeof(NewPlugin->Version));

  if(!FirstPlugin) FirstPlugin = NewPlugin;
  else {
    PluginInfo *Find = FirstPlugin;
    while(Find->Next)
      Find = Find->Next;
    Find->Next = NewPlugin;
  }
  return NewPlugin;
}

int Bot_UnloadPlugin(PluginInfo *Plugin) {
  if(!Plugin) return 0;

  if(Plugin->Prev) Plugin->Prev->Next = Plugin->Next;
  if(Plugin->Next) Plugin->Next->Prev = Plugin->Prev;
  if(Plugin == FirstPlugin) FirstPlugin = Plugin->Next;

  if(Plugin!=&CorePlugin) {
    void (*Deinit)() = dlsym(Plugin->DL, "Plugin_Deinit");
    if(Deinit) Deinit();
  }

  for(int i=0;i<NUM_TIMERS;i++) {
    TimerInfo *Timer = Plugin->Timers[i];
    if(Timer) {
      if((Timer->Flags & TIMER_RUN_ON_UNLOAD) && Bot_ContextIsValid(&Timer->Context))
        Bot_StartEvent(Plugin, Timer->EventType, &Timer->Context, Timer->Flags, "%s early=1", Timer->EventText);
      free(Timer);
    }
  }

  for(EventHook *FreeHook = FirstEvent;FreeHook;) {
    EventHook *Next = FreeHook->NextType;
    for(EventHook *FreeHook2 = FreeHook;FreeHook2;) {
      EventHook *Next2 = FreeHook2->Next;
      if(FreeHook2->Plugin == Plugin) Bot_DelEventHook(FreeHook2);
      FreeHook2=Next2;
    }
    FreeHook = Next;
  }

  if(Plugin!=&CorePlugin) {
    dlclose(Plugin->DL);
    free(Plugin);
  }
  return 1;
}

PluginInfo *Bot_FindPlugin(char *Name) {
  PluginInfo *Cur = FirstPlugin;
  while(Cur) {
    if(!strcasecmp(Name, Cur->Name))
    Cur=Cur->Next;
  }
  return NULL;
}

int Bot_PluginIsPresent(char *Name) {
  return Bot_FindPlugin(Name)!=NULL;
}

void *Bot_PluginSymbol(char *Module, char *Name) {
  PluginInfo *Plugin = Bot_FindPlugin(Module);
  if(!Plugin) return NULL;
  return dlsym(Plugin->DL, Name);
}

int Bot_PluginSetHandler(PluginInfo *Plugin, const char *Name, void *Handler) {
  if(!strcasecmp(Name, "CPD Alloc")) {
    Plugin->ChannelPluginData_Alloc = Handler;
    return 1;
  } else if(!strcasecmp(Name, "CPD Free")) {
    Plugin->ChannelPluginData_Free = Handler;
    return 1;
  }
  return 0;
}

void *Bot_ChannelPluginData(PluginInfo *Plugin, ChannelInfo *Channel, unsigned int Flags) {
  unsigned char Type = Flags & 255;
  int AutoMake = Flags & CPD_AUTO_CREATE;
  void *(*Alloc)(void *, unsigned char) = Plugin->ChannelPluginData_Alloc;
  void *New;
  ChannelPluginData *CPD = Channel->PluginData;
  if(!CPD) {
    if(!AutoMake) return NULL;
    CPD = (ChannelPluginData*)malloc(sizeof(ChannelPluginData));
    CPD->Type = Type;
    CPD->Plugin = Plugin;
    CPD->Next = NULL;
    New = Alloc((void*)Channel, Type);
    if(!New) { free(CPD); return NULL;}
    CPD->Data = New;
    Channel->PluginData=CPD;
    return New;
  } else while(1) {
    if(CPD->Plugin == Plugin && CPD->Type == Type)
      return CPD->Data;
    if(CPD->Next) // keep looking
      CPD=CPD->Next;
    else { // end of the list
      if(AutoMake) {
        ChannelPluginData *NewCPD = (ChannelPluginData*)malloc(sizeof(ChannelPluginData));
        NewCPD->Type = Type;
        NewCPD->Plugin = Plugin;
        NewCPD->Next = NULL;
        New = Alloc((void*)Channel, Type);
        if(!New) { free(NewCPD); return NULL;}
        NewCPD->Data = New;
        CPD->Next=NewCPD;
        return New;
      }
      break;
    }
  }
  return NULL;
}

void Bot_FreeChannel(ChannelInfo *Channel) {
  ChannelPluginData *CPD = Channel->PluginData;
  while(CPD) {
    ChannelPluginData *Next = CPD->Next;
    PluginInfo *Plugin = CPD->Plugin;
    if(!Plugin->ChannelPluginData_Free)
      free(CPD->Data);
    else
      Plugin->ChannelPluginData_Free(CPD->Data, CPD->Type);
    free(CPD);
    CPD = Next;
  }
}

void Bot_FreeNetwork(NetworkInfo *Network) {
  ChannelInfo *Chan = (ChannelInfo*)Network->FirstChannel;
  while(Chan) {
    ChannelInfo *Next = Chan->Next;
    Bot_FreeChannel(Chan);
    free(Chan);
    Chan = Next;
  }
}

NetworkInfo *Bot_EstablishConnection(NetworkInfo *Net, const char *Tag) {
  if(!Net) return NULL;
  IPaddress ip;
  Bot_ZeroStruct(ip);
  TCPsocket tcpsock;
  Bot_ZeroStruct(tcpsock);

  char GroupName[64];
  sprintf(GroupName, "Network %s%%Network Default", Tag);

  const char *Nick = Bot_GetConfigStr(GroupName, "Nick", "ToasterBot");
  const char *User = Bot_GetConfigStr(GroupName, "Username", "toaster");
  const char *Realname = Bot_GetConfigStr(GroupName, "Realname", "Test");
  const char *Server = Bot_GetConfigStr(GroupName, "Server", "irc.novasquirrel.com");
  int Port = Bot_GetConfigInt(GroupName, "Port", 6667);

  if(SDLNet_ResolveHost(&ip,Server,Port) < 0) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return NULL;
  }

  tcpsock=SDLNet_TCP_Open(&ip);
  if(!tcpsock) {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return NULL;
  }

  char ConnectMsg[512], PassMsg[128]="";
  const char *ConnectPass = Bot_GetConfigStr(GroupName, "ConnectPass", NULL);
  if(ConnectPass)
    sprintf(PassMsg, "PASS %s\r\n", ConnectPass);
  sprintf(ConnectMsg, "%sNICK %s\r\nUSER %s 8 * :%s\r\n", PassMsg, Nick, User, Realname);
  int Len = strlen(ConnectMsg);
  if(SDLNet_TCP_Send(tcpsock,ConnectMsg,Len)<Len) {
    printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
  }

  if(SDLNet_TCP_AddSocket(SocketSet,tcpsock)==-1) {
    SDLNet_TCP_Close(tcpsock);
    printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
    return NULL;
  }

  Bot_StrCpy(Net->Nick, Nick, sizeof(Net->Nick));
  Bot_StrCpy(Net->Username, User, sizeof(Net->Username));
  Bot_StrCpy(Net->ConnectURL, Server, sizeof(Net->ConnectURL));
  Bot_StrCpy(Net->Tag, Tag, sizeof(Net->Tag));
  *Net->NickPass = 0;
  *Net->ServerName = 0;
  *Net->ConnectPass = 0;
  *Net->FullHost = 0;
  Net->IP = ip;
  Net->Port = Port;
  Net->Socket = tcpsock;
  Net->LastConnectCheck = 0;
  time(&Net->ConnectTime);
  return Net;
}

NetworkInfo *Bot_ConnectNetwork(const char *NetTag) {
  for(NetworkInfo *Net = FirstNetwork;Net;Net=Net->Next)
    if(!strcasecmp(Net->Tag, NetTag))
      return Net;

  NetworkInfo Net;
  Bot_ZeroStruct(Net);

  if(!Bot_EstablishConnection(&Net, NetTag))
    return NULL;

  // add network to list
  NetworkInfo *NewNet = (NetworkInfo*)malloc(sizeof(NetworkInfo));
  if(!NewNet) {
    SDLNet_TCP_Close(Net.Socket);
    return NULL;
  }

  *NewNet = Net;
  *NewNet->Name = 0;
  NewNet->Flags = 0;
  NewNet->FirstChannel = NULL;
  NewNet->Prev = NULL;
  NewNet->Next = NULL;

  NetworkInfo *FindNet = FirstNetwork;
  if(!FindNet) {
    FirstNetwork = NewNet;
  } else {
    while(FindNet->Next)
      FindNet = FindNet->Next;
    NewNet->Prev = FindNet;
    FindNet->Next = NewNet;
  }
  return NewNet;
}

int Bot_ReconnectNetwork(NetworkInfo *Network) {
  if(Network->Socket) {
    SDLNet_TCP_DelSocket(SocketSet,Network->Socket);
    SDLNet_TCP_Close(Network->Socket);
  }
  time(&Network->ConnectTime);
  if(!Bot_EstablishConnection(Network, Network->Tag))
    return 0;
  for(ChannelInfo *Chan = (ChannelInfo *)Network->FirstChannel;Chan;Chan=Chan->Next)
    Chan->Flags &= ~(CHANFLAG_JOINED|CHANFLAG_TRYJOIN);
  return 1;
}

void Bot_DisconnectNetwork(NetworkInfo *Network, const char *QuitMessage) {
  if(!Network) return;
  ContextInfo NetContext;
  Bot_MakeContext(&NetContext, CONTEXT_NETWORK, Network);
  if(QuitMessage)
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "QUIT :%s", QuitMessage);
  SDLNet_TCP_DelSocket(SocketSet,Network->Socket);

  Bot_FreeNetwork(Network);
  SDLNet_TCP_Close(Network->Socket);
  if(FirstNetwork == Network) FirstNetwork = Network->Next;
  if(Network->Prev) Network->Prev->Next = Network->Next;
  if(Network->Next) Network->Next->Prev = Network->Prev;
  free(Network);
}

int Bot_FindEventParam(BotEvent *Event, const char *Param) {
  char LookFor[512];
  // allow equals in param name, but ignore value given
  if(!strchr(Param, '='))
    sprintf(LookFor, "%s=", Param);
  else {
    strcpy(LookFor, Param);
    char *Temp = strchr(LookFor, '=');
    Temp[1] = 0;
  }
  int i;
  for(i=0;i<Event->WordLen;i++) {
    const char *A = LookFor;
    const char *B = Event->Word[i];
    int Okay = 1;
    for(;*A;A++,B++)
      if(tolower(*A)!=tolower(*B)) {
        Okay = 0;
        break;
      }
    if(Okay)
      return i;
  }
  return -1;
}

NetworkInfo *Bot_FindNetwork(const char *Name) {
  if(Name == NULL) return NULL;
  NetworkInfo *Cur;
  for(Cur = FirstNetwork;Cur;Cur=Cur->Next)
    if(!strcasecmp(Name, Cur->Tag))
      return Cur;
  return NULL;
}

ChannelInfo *Bot_FindChannel(NetworkInfo *Network, const char *Name) {
  if(!Network || !Name) return NULL;
  for(ChannelInfo *Cur = Network->FirstChannel;Cur;Cur=Cur->Next)
    if(!strcasecmp(Name, Cur->Name))
      return Cur;
  return NULL;
}

int Bot_GetEventInt(BotEvent *Event, const char *Param, int Default) {    
  int i = Bot_FindEventParam(Event, Param);
  if(i == -1) return Default;
  char *Value = strchr(Event->Word[i], '=');
  if(!Value) return Default; // shouldn't be possible
  return strtol(Value+1, NULL, 0);
}

const char *Bot_GetEventStr(BotEvent *Event, const char *Param, const char *Default) {
  int i = Bot_FindEventParam(Event, Param);
  if(i == -1) return Default;
  char *Value = strchr(Event->Word[i], '=');
  if(!Value) return Default; // shouldn't be possible
  return Value+1;
}

void Bot_TokenizeEvent(BotEvent *Event) {
  if(!Event || Event->Flags & EF_NO_TOKENIZATION) return;

  *Event->TextWord = 0;
  strcpy(Event->TextWord, Event->Text);
  char *Peek = Event->TextWord;
  Event->WordLen = 0;
  Event->WordEol[0] = Event->Text;
  Event->Word[0] = Event->TextWord;

  int i;
  for(i=0;i<BOTEVENT_WORDSIZE;i++) {
    // init with "" for crash protection
    Event->Word[i] = "";
    Event->WordEol[i] = "";
  }

  while(1) {
    char *Next;

    if(*Peek != '\t' || (Event->Flags & EF_NO_MULTIWORD))
      Next = strchr(Peek+1, ' ');
    else // multi word
      Next = strchr(Peek+1, '\t');
    while(*Peek == '\t') Peek++;
    Event->Word[Event->WordLen++] = Peek;
    if(!(Event->Flags & EF_KEEP_COLONS) && Event->Word[Event->WordLen-1][0] == ':')
      Event->Word[Event->WordLen-1]++;

    if(Event->WordLen >= BOTEVENT_WORDSIZE-1)
      break;

    if(!Next) break;
    *Next = 0;

    Peek = Next+1;
    while(*Peek == ' ') Peek++;
  }
  if(!Event->Word[Event->WordLen-1][0])
    Event->WordLen--;

  for(i=0;i<Event->WordLen;i++) {
    Event->WordEol[i] = Event->Text + (Event->Word[i] - Event->TextWord);
  }
}

int Bot_ChangeParams(BotEvent *NewEvent, BotEvent *Event, const char *NewParams) {
  BotEvent TempEvent;
  Bot_ZeroStruct(TempEvent);

  strcpy(TempEvent.Text, NewParams);
  Bot_TokenizeEvent(&TempEvent);
  char New[BOTEVENT_TEXTSIZE]=""; int i;
  for(i=0;i<Event->WordLen;i++)
    // add to new list if not in the change-to list
    if(-1 == Bot_FindEventParam(&TempEvent, Event->Word[i]))
      sprintf(strrchr(New, 0), "%s\t%s\t", ((*New)?" ":""), Event->Word[i]);
  for(i=0;i<TempEvent.WordLen;i++)
    sprintf(strrchr(New, 0), "%s\t%s\t", ((*New)?" ":""), TempEvent.Word[i]);
  strcpy(NewEvent->Text, New);
  Bot_TokenizeEvent(NewEvent);
  return 0;
}

void Bot_XChatTokenize(const char *Input, char *WordBuff, int WordBuffSize, const char **Word, const char **WordEol, int WordSize) {
  int i;
  for(i=0;i<WordSize;i++) {
    Word[i] = "";
    WordEol[i] = "";
  }
  strcpy(WordBuff, Input);

  for(i=0;WordBuff[i];i++) // filter tabs because I use them
    if(WordBuff[i]=='\t')
      WordBuff[i] = ' ';

  i = 0;
  char *Peek = WordBuff;
  while(1) {
    char *Next;
    Next = strchr(Peek+1, ' ');

    while(*Peek == '\t') Peek++;
    Word[i] = Peek;

    if(i >= (WordSize-1))
      break;

    if(!Next) break;
    *Next = 0;

    Peek = Next+1;
    while(*Peek == ' ') Peek++;
  }

  for(int j=0;j<i;j++) {
    WordEol[j] = Input + (Word[i] - WordBuff);
  }
}

EventHook *Bot_AddEventHook(PluginInfo *Plugin, const char *Type, int Priority, int Flags, int Need0, int Need1, void *Handler) {
  EventHook *Find = FirstEvent, *Alloc;
  EventHook NewHook;
  Bot_ZeroStruct(NewHook); // makes all pointers NULL
  strcpy(NewHook.Type, Type);
  NewHook.Priority = Priority;
  NewHook.Flags = Flags;
  NewHook.EventFlagsNeed0 = Need0;
  NewHook.EventFlagsNeed1 = Need1;
  NewHook.Handler = Handler;
  NewHook.Prev = NULL;
  NewHook.PrevType = NULL;
  NewHook.Next = NULL;
  NewHook.NextType = NULL;
  NewHook.Plugin = Plugin;

  if(!Find) { // create first node
    FirstEvent = (EventHook *)malloc(sizeof(EventHook));
    if(!FirstEvent) return NULL;
    *FirstEvent = NewHook;
    return FirstEvent;
  }
  for(;Find;Find=Find->NextType) {
    if(!strcasecmp(Find->Type, NewHook.Type)) {
      // found hook type in list
      if(NewHook.Priority > Find->Priority) {
        // replace in types list
        Alloc = (EventHook *)malloc(sizeof(EventHook));
        if(!Alloc) return NULL;
        *Alloc = NewHook;
        Alloc->PrevType = Find->PrevType;
        if(Alloc->PrevType) Alloc->PrevType->NextType = Alloc;
        Alloc->NextType = Find->NextType;
        if(Alloc->NextType) Alloc->NextType->PrevType = Alloc;
        Alloc->Next = Find;
        Find->Prev = Alloc;
      } else {
        while(Find) { // add before item
          if(NewHook.Priority > Find->Priority) {
            Alloc = (EventHook *)malloc(sizeof(EventHook));
            if(!Alloc) return NULL;
            *Alloc = NewHook;
            Alloc->Prev = Find->Prev;
            Alloc->Next = Find;
            Find->Prev = Alloc;
            if(Alloc->Prev) Alloc->Prev->Next = Alloc;
            return Alloc;
          }
          if(!Find->Next) { // add onto end
            Alloc = (EventHook *)malloc(sizeof(EventHook));
            if(!Alloc) return NULL;
            *Alloc = NewHook;
            Find->Next = Alloc;
            Alloc->Prev = Find;
            return Alloc;
          }
          Find = Find->Next;
        }
        return NULL;
      }
    } else if(Find->NextType == NULL) {
      // add onto the end of the hook types
      Alloc = (EventHook *)malloc(sizeof(EventHook));
      if(!Alloc) return NULL;
      *Alloc = NewHook;
      Find->NextType = Alloc;
      Alloc->PrevType = Find;
      return Alloc;
    }
  }

  return NULL;
}

int Bot_DelEventHook(EventHook *Hook) {
  if(!Hook) return 0;
  EventHook *Find = FirstEvent;
  if(FirstEvent == Hook) {
    if(Hook->Next) {
      FirstEvent = Hook->Next;
      FirstEvent->NextType = Hook->NextType;
      if(FirstEvent->NextType) FirstEvent->NextType->PrevType = FirstEvent;
      FirstEvent->Prev = NULL;
    } else {
      FirstEvent = Hook->NextType;
    }
    free(Hook);
    return 1;
  }

  for(;Find;Find=Find->NextType)
    if(!strcasecmp(Find->Type, Hook->Type)) {
      for(;Find;Find=Find->Next)
        if(Find == Hook) {
          if(!Find->Prev) { // first entry in list?
            if(!Find->Next) { // delete event type altogether?
              if(Find->PrevType) Find->PrevType->NextType = Find->NextType;
              if(Find->NextType) Find->NextType->PrevType = Find->PrevType;
            } else { // only delete first entry, but keep second one
              if(Find->PrevType) Find->PrevType->NextType = Find->Next;
              if(Find->NextType) Find->NextType->PrevType = Find->Next;
              Find->Next->PrevType = Find->PrevType;
              Find->Next->NextType = Find->NextType;
              Find->Next->Prev = NULL;
            }
            free(Find);
            return 1;
          }
          if(Find->Prev) Find->Prev->Next = Find->Next;
          if(Find->Next) Find->Next->Prev = Find->Prev;
          free(Find);
          return 1;
        }
      return 0;
    }
  return 0;
}

void Bot_FreeAsyncEvents(int Join) {
// todo: write
  AsyncEventInfo *Info = FirstAsyncEvent;
  while(Info) {
    AsyncEventInfo *Next = Info->Next;
    if(Join)
      pthread_join(Info->Thread,NULL);
    if(Info->Finished) {
      if(Info == FirstAsyncEvent)
        FirstAsyncEvent = Info->Next;
      if(Info->Prev) Info->Prev->Next = Info->Next;
      if(Info->Next) Info->Next->Prev = Info->Prev;
      free(Info);
    }
    Info = Next;
  }
}

void *Bot_AsyncRunEvent(void *Info2) {
//  puts("Running async event");
  AsyncEventInfo *Info = (AsyncEventInfo*)Info2;
  BotEvent *Event = &Info->Event;
  if(!Event) return NULL;
  EventHook *Hook = FirstEvent;
  for(;Hook;Hook=Hook->NextType)
    if(!strcasecmp(Hook->Type, Event->Type)) {
      for(;Hook;Hook=Hook->Next) {
        int Return = Hook->Handler(Event);
        switch(Return) {
          case ER_NORMAL:
            break;
          case ER_HANDLED:
            Event->Flags |= EF_ALREADY_HANDLED;
            break;
          case ER_DELETE:
            Info->Finished = 1;
            pthread_exit(NULL);
            return NULL;
        }
      }
      Info->Finished = 1;
      pthread_exit(NULL);
      return NULL;
    }
  Info->Finished = 1;
  pthread_exit(NULL);
  return NULL;
}

int Bot_RunEvent(BotEvent *Event) {
  if(!Event) return -1;
  if(Event->Flags & EF_ASYNCHRONOUS) {
    AsyncEventInfo Info;
    Bot_ZeroStruct(Info);
    Info.Event = *Event;
    Info.Finished = 0;
    Info.Prev = NULL;
    Info.Next = NULL;
    AsyncEventInfo *Find = FirstAsyncEvent;
    if(!Find) {
      Find = (AsyncEventInfo *)malloc(sizeof(AsyncEventInfo));
      FirstAsyncEvent = Find;
      *Find = Info;
    } else {
      while(Find->Next)
        Find=Find->Next;
      Find->Next = (AsyncEventInfo *)malloc(sizeof(AsyncEventInfo));
      *Find->Next = Info;
      Find->Next->Prev = Find;
    }
    if(pthread_create(&Find->Thread, NULL, Bot_AsyncRunEvent, Find)) {
      fprintf(stderr, "Thread creation failed\n");
      return 0;
    }
    return 1;
  }
  EventHook *Hook = FirstEvent;
  for(;Hook;Hook=Hook->NextType)
    if(!strcasecmp(Hook->Type, Event->Type)) {
      for(;Hook;Hook=Hook->Next) {
        if((Event->Flags & Hook->EventFlagsNeed1) != Hook->EventFlagsNeed1)
          continue;
        if(Event->Flags & Hook->EventFlagsNeed0)
          continue;
        int Return = Hook->Handler(Event);
        switch(Return) {
          case ER_NORMAL:
            break;
          case ER_HANDLED:
            Event->Flags |= EF_ALREADY_HANDLED;
            break;
          case ER_DELETE:
            return 1;
        }
      }
      return 1;
    }
  return 0;
}

int Bot_IndexInStrList(const char *Search, const char* List, ...) {
  va_list Args;
  va_start(Args, List);
  for(int i=0;; i++) {
    const char *A = va_arg(Args, const char*);
    if(!strcasecmp(Search, A)) {
      va_end(Args);
      return i;
    }
  }
  va_end(Args);
  return -1;
}

int Bot_StartEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, const char* Format, ...) {
  if(Context && !Bot_ContextIsValid(Context)) {
    fprintf(stderr, "Invalid context for Bot_StartEvent()\n");
    return -1;
  }
  BotEvent Event;
  Bot_ZeroStruct(Event);
  va_list Args;
  va_start(Args, Format);
  vsprintf(Event.Text, Format, Args);
  va_end(Args);

  Event.Plugin = Plugin;
  Event.Flags = Flags;
  Event.Context = Context;
  strcpy(Event.Type, Type);
  Bot_TokenizeEvent(&Event);
  RecurseLevel++;
  int Return = Bot_RunEvent(&Event);
  RecurseLevel--;
  return Return;
}

int Bot_StartDelayedEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, unsigned long Seconds, BotEvent *Base, const char* Format, ...) {
  TimerInfo Timer;
  Bot_ZeroStruct(Timer);
  Timer.Plugin = Plugin;
  Timer.Seconds = Seconds;
  Timer.Flags = 0;
  Timer.Context = *Context;
  Timer.EventFlags = Flags;
  time(&Timer.StartTime);
  Timer.EndTime = Timer.StartTime + Seconds;

  Bot_StrCpy(Timer.EventType, Type, sizeof(Timer.EventType));

  va_list Args;
  va_start(Args, Format);
  vsprintf(Timer.EventText, Format, Args);
  va_end(Args);

  if(Base) {
    BotEvent TempEvent = *Base;
    Bot_ChangeParams(&TempEvent, &TempEvent, Timer.EventText);
    Bot_StrCpy(Timer.EventText, TempEvent.Text, sizeof(Timer.EventText));
  }

  for(int i=0;i<NUM_TIMERS;i++)
    if(!Plugin->Timers[i]) {
      TimerInfo *Alloc = (TimerInfo*)malloc(sizeof(TimerInfo));
      if(!Alloc) return -1;
      *Alloc = Timer;
      Plugin->Timers[i] = Alloc;
      return i;
    }
  return -1;
}

int Bot_ParamCopyEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, BotEvent *BaseEvent, const char* Format, ...) {
  BotEvent Event;
  Bot_ZeroStruct(Event);
  Event.Flags = 0;
  *Event.Text = 0;
  va_list Args;
  char NewArg[BOTEVENT_TEXTSIZE]="";
  va_start(Args, Format);
  vsprintf(NewArg, Format, Args);
  va_end(Args);

  strcpy(Event.Text, BaseEvent->Text);
  Bot_ChangeParams(&Event, BaseEvent, NewArg);

  Event.Plugin = Plugin;
  Event.Flags = Flags;
  Event.Context = Context;
  strcpy(Event.Type, Type);
  RecurseLevel++;
  int Return;
  Return = Bot_RunEvent(&Event);
  RecurseLevel--;
  return Return;
}

int Bot_ParseINI(FILE *File, void (*Handler)(const char *Group, const char *Item, const char *Value)) {
  char Group[512]="", *Item, *Value, Line[512]="", c, *Poke = NULL;
  if(File == NULL)
    return 0;
  int i;
  while(!feof(File)) {
    for(i=0,c=1;;i++) {
      c = fgetc(File);
      if(c=='\r'||c=='\n') {
        Line[i]=0;
        break;
      }
      Line[i] = c;
    }
    while(c=='\r'||c=='\n')
      c = fgetc(File);
    fseek(File, -1 , SEEK_CUR);
    if(!*Line)
      break;
    else if(*Line == ';'); // comment
    else if(*Line == '[') { // group
      Poke = strchr(Line, ']');
      if(Poke) *Poke = 0;
      strcpy(Group, Line+1);
    } else { // item
      Poke = strchr(Line, '=');
      if(Poke) {
        *Poke = 0;
        Item = Line;
        Value = Poke+1;
        Handler(Group, Item, Value);
      }
    }
  }
  fclose(File);
  return 1;
}

ConfigPair *Bot_FindConfigPair(ConfigPair *Find, const char *Name) {
  for(;Find;Find=Find->Next)
    if(Find->Name && Name && !strcasecmp(Find->Name, Name))
      return Find;
  return NULL;
}

const char *Bot_GetConfigStr(const char *GroupName, const char *ItemName, const char *DefaultValue) {
  char GroupName2[strlen(GroupName)+1];
  char *AltGroup = NULL;
  if(strchr(GroupName, '%')) {
    strcpy(GroupName2, GroupName);
    AltGroup = strchr(GroupName2, '%');
    *(AltGroup++) = 0;
    GroupName = GroupName2;
  }

  ConfigPair *Group = Bot_FindConfigPair(FirstConfig, GroupName);
  if(!Group) {
    if(!AltGroup) return DefaultValue;
    return Bot_GetConfigStr(AltGroup, ItemName, DefaultValue);
  }

  ConfigPair *Item = Bot_FindConfigPair(Group->Item, ItemName);
  if(!Item) {
    if(!AltGroup) return DefaultValue;
    return Bot_GetConfigStr(AltGroup, ItemName, DefaultValue);
  }
  return Item->Value;
}

int Bot_GetConfigInt(const char *GroupName, const char *ItemName, int DefaultValue) {
  const char *Get = Bot_GetConfigStr(GroupName, ItemName, NULL);
  if(Get == NULL) return DefaultValue;
  return strtol(Get, NULL, 0); 
}

void Bot_AddConfigPair(const char *GroupName, const char *ItemName, const char *Value) {
  ConfigPair NewPair, *FindEnd;
  Bot_StrCpy(NewPair.Name, ItemName, sizeof(NewPair.Name));
  NewPair.Value = NULL;
  NewPair.Next = NULL;
  NewPair.Item = NULL;
  // find group, and create it if it doesn't already exist
  ConfigPair *Group = Bot_FindConfigPair(FirstConfig, GroupName);
  if(!Group) {
    Group = (ConfigPair *)malloc(sizeof(ConfigPair));
    if(!Group) return;
    if(!FirstConfig)
      FirstConfig = Group;
    else {
      for(FindEnd = FirstConfig; FindEnd->Next; FindEnd=FindEnd->Next);
      FindEnd->Next = Group;
    }
    Bot_StrCpy(Group->Name, GroupName, sizeof(Group->Name));
    Group->Value = NULL;
    Group->Next = NULL;
    Group->Item = NULL;
  }

  // find item, and create it if it doesn't already exist
  ConfigPair *Alloc = Bot_FindConfigPair(Group, ItemName);
  if(!Alloc) {
    Alloc = (ConfigPair *)malloc(sizeof(ConfigPair));
    if(!Alloc) return;
    if(!Group->Item) {
      Group->Item = Alloc;
    } else {
      for(FindEnd = Group->Item; FindEnd->Next; FindEnd=FindEnd->Next);
      FindEnd->Next = Alloc;
    }
    Bot_StrCpy(Alloc->Name, ItemName, sizeof(Alloc->Name));
    Alloc->Next = NULL;
    Alloc->Item = NULL;
    Alloc->Value = NULL;
  }
  if(Alloc->Value) free(Alloc->Value);
  Alloc->Value = (char *)malloc(strlen(Value)+1);
  strcpy(Alloc->Value, Value);
}

void Bot_FreeConfig(ConfigPair *Free) {
  while(Free) {
    ConfigPair *Next = Free->Next;
    if(Free->Item) Bot_FreeConfig(Free->Item);
    if(Free->Value) free(Free->Value);
    free(Free);
    Free = Next;
  }
}

void Bot_ReloadConfig(const char *Filename) {
  Bot_StartEvent(&CorePlugin, "Config Reload", NULL, 0, "%s", Filename);
  Bot_FreeConfig(FirstConfig);
  FirstConfig = NULL;
  Bot_ParseINI(fopen(Filename, "rb"), Bot_AddConfigPair);

  ConfigPair *Pair = Bot_FindConfigPair(FirstConfig, "Modules");
  if(Pair) {
    Pair = Pair->Item;
    while(Pair) {
      PluginInfo *P = Bot_LoadPlugin(Pair->Value);
      if(P) strcpy(P->Tag, Pair->Name);
      Pair=Pair->Next;
    }
  } else {
    fprintf(stderr, "No [Modules] group found\n");
  }

  ConfigPair *Networks = FirstConfig;
  while(Networks) {
    if(!Bot_MemCaseCmp(Networks->Name, "Network ", 8)) {
      if(Bot_GetConfigInt(Networks->Name, "Autoconnect", 0))
        Bot_ConnectNetwork(Networks->Name+8);
    }
    Networks = Networks->Next;
  }
}

int Bot_SplitUserHost(const char *FullHost, char *Buffer, char **User, char **Host) {
  *User = NULL;
  *Host = NULL;
  strcpy(Buffer, FullHost);
  if(strchr(Buffer, '!') && strchr(Buffer, '@')) {
    *Host = strchr(Buffer, '@') + 1;
    Host[0][-1] = 0;
    *User = strchr(Buffer, '!') + 1;
    User[0][-1] = 0;
    return 1;
  }
  return 0;
}

const char *Bot_GetInfo(ContextInfo *Context, const char *Info) {
// http://xchat.org/docs/plugin20.html#xchat_get_info
  if(!Context) return NULL;
  NetworkInfo *Network = Context->Target.Network;
  if(Context->Type == CONTEXT_CHANNEL)
    Network = Context->Target.Channel->Network;
  if(!strcasecmp(Info, "Network")) return Network->Name;
  if(!strcasecmp(Info, "Server")) return Network->ServerName;
  if(!strcasecmp(Info, "Host")) return Network->ConnectURL;
  if(!strcasecmp(Info, "NetTag")) return Network->Tag;
  if(!strcasecmp(Info, "Nick")) return Network->Nick;
  if(!strcasecmp(Info, "Username")) return Network->Username;
  if(!strcasecmp(Info, "NickServ")) return Network->NickPass;
  if(!strcasecmp(Info, "ConnectPass")) return Network->ConnectPass;
  if(!strcasecmp(Info, "FullHost")) return Network->FullHost;

  if(Context->Type != CONTEXT_CHANNEL) return NULL;
  ChannelInfo *Channel = Context->Target.Channel;
  if(!strcasecmp(Info, "Channel")) return Channel->Name;
  return NULL;
}

ContextInfo *Bot_MakeContext(ContextInfo *Context, int Type, void *Pointer) {
  memset(Context, 0, sizeof(ContextInfo));
  Context->Type = Type;
  Context->Target.Generic = Pointer;
  Context->User = NULL;
  return Context;
}

int Bot_ContextIsValid(ContextInfo *Context) {
  for(NetworkInfo *Net = FirstNetwork; Net; Net=Net->Next) {
    if(Context->Target.Network == Net)
      return 1;
    for(ChannelInfo *Chan = Net->FirstChannel; Chan; Chan=Chan->Next) {
      if(Context->Target.Channel == Chan)
        return 1;
    }
  }
  return 0;
}

int Default_YouJoin(BotEvent *Event) {
  NetworkInfo *Network = Event->Context->Target.Network;
  const char *ChannelName = Bot_GetEventStr(Event, "Channel", NULL);
  if(!ChannelName) return ER_NORMAL;

  ChannelInfo NewChannel;
  Bot_StrCpy(NewChannel.Name, ChannelName, sizeof(NewChannel.Name));
  NewChannel.Flags = CHANFLAG_JOINED;
  NewChannel.Network = Network;
  NewChannel.Prev = NULL;
  NewChannel.Next = NULL;
  NewChannel.Scrollback = NULL;
  NewChannel.PluginData = NULL;

  ChannelInfo *FindChannel = Network->FirstChannel;
  if(!FindChannel) {
    ChannelInfo *Alloc = (ChannelInfo*)malloc(sizeof(ChannelInfo));
    if(!Alloc) return ER_NORMAL;
    *Alloc = NewChannel;
    Network->FirstChannel = Alloc;
    return ER_NORMAL;
  }
  while(FindChannel) {
    if(!strcasecmp(FindChannel->Name, ChannelName)) {
      FindChannel->Flags |= CHANFLAG_JOINED;
      return ER_NORMAL;
    }
    if(!FindChannel->Next) {
      ChannelInfo *Alloc = (ChannelInfo*)malloc(sizeof(ChannelInfo));
      if(!Alloc) return ER_NORMAL;
      *Alloc = NewChannel;
      FindChannel->Next = Alloc;
      Alloc->Prev = FindChannel;
      return ER_NORMAL;
    }
    FindChannel = FindChannel->Next;
  }
  return ER_NORMAL;
}
int Default_YouPartKick(BotEvent *Event) {
  NetworkInfo *Network = Event->Context->Target.Channel->Network;
  const char *ChannelName = Bot_GetEventStr(Event, "Channel", NULL);
  if(!ChannelName) return ER_NORMAL;
  
  ChannelInfo *FindChannel = Network->FirstChannel;
  while(FindChannel) {
    if(!strcasecmp(FindChannel->Name, ChannelName)) {
      if(Network->FirstChannel == FindChannel)
        Network->FirstChannel = FindChannel->Next;
      if(FindChannel->Prev) FindChannel->Prev->Next = FindChannel->Next;
      if(FindChannel->Next) FindChannel->Next->Prev = FindChannel->Prev;
      Bot_FreeChannel(FindChannel);
      free(FindChannel);
      return ER_NORMAL;
    }
    FindChannel = FindChannel->Next;
  }
  return ER_NORMAL;
}

/* core handlers for stuff */
int Default_ServerOutput(BotEvent *Event) {
  if(Event->Context->Type != CONTEXT_NETWORK) {
    fprintf(stderr, "Server Output with non-network context\n");
    return ER_NORMAL; // ???
  }
  char Text[strlen(Event->Text)+5];
  sprintf(Text, "%s\n", Event->Text);
  NetworkInfo *Network = Event->Context->Target.Network;
  printf("%s >> %s", Network->Tag, Text);
  SDLNet_TCP_Send(Network->Socket,Text,strlen(Text));
  return ER_HANDLED;
}

const char *Bot_NameFromContext(ContextInfo *Info, char *Write) {
  switch(Info->Type) {
    case CONTEXT_NETWORK:
      return Info->Target.Network->Tag;
    case CONTEXT_CHANNEL:
      return Info->Target.Channel->Name;
    case CONTEXT_USER:
      strcpy(Write, Info->User);
      char *Poke = strchr(Write, '!');
      if(Poke) *Poke = 0;
      return Write;
    default:
      return NULL;
  }
}

void Bot_PrintChannels(const char *put) {
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

int Default_ClientCommand(BotEvent *Event) {
  Bot_PrintChannels("1");
  NetworkInfo *Network = Event->Context->Target.Network;
  if(Event->Context->Type == CONTEXT_CHANNEL) Network = Event->Context->Target.Channel->Network;
  ContextInfo NetContext;
  Bot_MakeContext(&NetContext, CONTEXT_NETWORK, Network);
  char SourceBuf[64]="";

  const char *Cmd = Event->Word[0];
  const char *Arg = Event->WordEol[1];
  char NewArg[strlen(Arg)+16];

  if(!strcasecmp(Cmd, "me")) {
    Cmd = "say";
    sprintf(NewArg, "\1ACTION %s\1", Arg);
    Arg = NewArg;
  }

  if(!strcasecmp(Cmd, "say")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "PRIVMSG %s :%s", Bot_NameFromContext(Event->Context, SourceBuf), Arg);
    return ER_HANDLED;
  } /* else if(!strcasecmp(Cmd, "msg")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "PRIVMSG %s :%s", Event->Word[1], Event->WordEol[2]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "join")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "JOIN %s", Event->WordEol[1]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "part")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "PART %s :%s", Event->Word[1], Event->WordEol[2]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "kick")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "KICK %s :%s", Event->Word[1], Event->WordEol[2]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "mode")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "MODE %s", Event->WordEol[1]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "notice")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "NOTICE %s :%s", Event->Word[1], Event->WordEol[2]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "topic")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "%s", Event->WordEol[1]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "quote")) {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "%s", Event->WordEol[1]);
    return ER_HANDLED;
  }*/ else if(!strcasecmp(Cmd, "shutdown")) {
    NeedShutdown = 1;
    RebootAfterShutdown = 0;
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "reboot")) {
    NeedShutdown = 1;
    RebootAfterShutdown = 1;
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "modload")) {
    PluginInfo *P = Bot_LoadPlugin(Event->WordEol[2]);
    if(P) Bot_StrCpy(P->Tag, Event->Word[1], sizeof(P->Tag));
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "modunload")) {
    for(PluginInfo *P = FirstPlugin;P;)
      if(!strcasecmp(Event->WordEol[1], P->Tag))
        Bot_UnloadPlugin(P);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "modreload")) {
    for(PluginInfo *P = FirstPlugin;P;)
      if(!strcasecmp(Event->WordEol[1], P->Tag)) {
        char Path[sizeof(P->Path)+1];
        Bot_UnloadPlugin(P);
        P = Bot_LoadPlugin(Path);
        if(P) Bot_StrCpy(P->Tag, Event->Word[1], sizeof(P->Tag));
      }
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "connect")) {
    Bot_ConnectNetwork(Event->Word[1]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "disconnect")) {
    for(NetworkInfo *Net = FirstNetwork;Net;Net=Net->Next)
      if(!strcasecmp(Net->Tag,Event->Word[1]))
        Bot_DisconnectNetwork(Net, Event->WordEol[2]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "reconnect")) {
    for(NetworkInfo *Net = FirstNetwork;Net;Net=Net->Next)
      if(!strcasecmp(Net->Tag,Event->Word[1]))
        Bot_DisconnectNetwork(Net, Event->WordEol[2]);
    Bot_ConnectNetwork(Event->Word[1]);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "rehash")) {
    Bot_ReloadConfig(ConfigPath);
    return ER_HANDLED;
  } else if(!strcasecmp(Cmd, "r")) {
    NetworkInfo *Net = Bot_FindNetwork(Event->Word[0]);
    ContextInfo RContext;
    if(Net) {
      if(Event->Word[1][0] == '#') {
         Bot_MakeContext(&RContext, CONTEXT_CHANNEL, Bot_FindChannel(Net, Event->Word[1]));
         if(RContext.Target.Channel)
           Bot_StartEvent(&CorePlugin, "Client Command", &RContext, 0, "%s", Event->WordEol[2]);
      } else {
         Bot_MakeContext(&RContext, CONTEXT_NETWORK, Network);
         Bot_StartEvent(&CorePlugin, "Client Command", &RContext, 0, "%s", Event->WordEol[2]);
      }
    }
    return ER_HANDLED;
  } else {
    Bot_StartEvent(&CorePlugin, "Server Output", &NetContext, 0, "%s", Event->WordEol[0]);
    return ER_HANDLED;
  }
  return ER_NORMAL;
}

int Default_ServerInput(BotEvent *Event) {
  char *FixReturn = strrchr(Event->Text, '\r');
  if(FixReturn) *FixReturn = 0;
  FixReturn = strrchr(Event->Word[Event->WordLen-1], '\r');
  if(FixReturn) *FixReturn = 0;

  char *RawSource = Event->Word[0];
  char *Command = Event->Word[1];
  char Source[256]="", *User=NULL, *Host=NULL;
  char Temp[768];
  char *MyNick = Event->Context->Target.Network->Nick;
  NetworkInfo *Network = Event->Context->Target.Network;;
  ContextInfo NewContext, *SameContext = Event->Context;
  Network->PongDeadline = 0;

  char GroupName[64], *Peek, *Peek2;
  sprintf(GroupName, "Network %s%%Network Default", Network->Tag);
  Bot_SplitUserHost(RawSource, Source, &User, &Host);
  int SourceIsMe = !strcasecmp(Source, MyNick);

  if((strlen(Command) == 3) && isdigit(*Command)) { // numeric reply
    switch(strtol(Command, NULL, 10)) {
      case RPL_WELCOME: // "Welcome to the Internet Relay Network <nick>!<user>@<host>"
        Bot_StrCpy(Network->Nick, Event->Word[2], sizeof(Network->Nick));
        Bot_StrCpy(Network->FullHost, Event->Word[Event->WordLen-1], sizeof(Network->FullHost));
        Bot_StartEvent(&CorePlugin, "Server Output", SameContext, 0, "MODE %s +B", Network->Nick);

        Bot_StrCpy(Temp, Bot_GetConfigStr(GroupName, "Channels", ""), sizeof(Temp));
        Peek = strtok_r(Temp," ", &Peek2);
        while(Peek != NULL) {
          Bot_StartEvent(&CorePlugin, "Server Output", SameContext, 0, "JOIN %s", Peek);
          Peek = strtok_r(NULL, " ", &Peek2);
        }
        Bot_StartEvent(&CorePlugin, "Server Connected", SameContext, 0, "fullhost=%s", Network->FullHost);
        break;
      case RPL_YOURHOST: // "Your host is <servername>, running version <ver>"
        break;
      case RPL_MYINFO: // "<servername> <version> <available user modes> <available channel modes>"
        break;
     case RPL_CHANNELMODEIS: // "<channel> <mode> <mode params>"
       break;
     case RPL_NOTOPIC: // "<channel> :No topic is set"
       break;
     case RPL_TOPIC: // "<channel> :<topic>"
       Bot_MakeContext(&NewContext, CONTEXT_CHANNEL, Bot_FindChannel(Network, Event->Word[2]));
       if(NewContext.Target.Channel)
         Bot_StartEvent(&CorePlugin, "Channel Topic Is", &NewContext, 0, "channel=%s \ttext=%s\t", Event->Word[2], Event->WordEol[3]);
       break;
     case RPL_WHOREPLY: // "<channel> <user> <host> <server> <nick> ( "H" / "G" > ["*"] [ ( "@" / "+" ) ] :<hopcount> <real name>"
       break;
     case RPL_ENDOFWHO: // "<name> :End of WHO list"
       break;
     case RPL_NAMREPLY: // "( "=" / "*" / "@" ) <channel> :[ "@" / "+" ] <nick> *( " " [ "@" / "+" ] <nick> )
       break;
     case RPL_ENDOFNAMES: // <channel>
       break;
     case RPL_YOUREOPER:
       break;
     case ERR_NOSUCHNICK: // <nickname>
       break;
     case ERR_NOSUCHSERVER: // <server name>
       break;
     case ERR_NOSUCHCHANNEL: // <channel name>
       break;
     case ERR_CANNOTSENDTOCHAN: // <channel name>
       break;
     case ERR_TOOMANYCHANNELS: // <channel name>
       break;
     case ERR_UNKNOWNCOMMAND: // <command>
       break;
     case ERR_ERRONEUSNICKNAME: // <nickname>
       break;
     case ERR_NICKNAMEINUSE: // <nickname>
       break;
     case 372: // skip the motd
       return ER_NORMAL;
    }
  }
  printf("%s << %s\n", Network->Tag, Event->Text);
  if(!strcasecmp("PING",Event->Word[0])) {
    Bot_StartEvent(&CorePlugin, "Server Output", SameContext, 0, "PONG :%s", Event->WordEol[1]);
  } else if(!strcasecmp(Command, "PRIVMSG")) {
    if(Event->Word[2][0] == '#') {
      Bot_MakeContext(&NewContext, CONTEXT_CHANNEL, Bot_FindChannel(Network, Event->Word[2]));
      if(NewContext.Target.Channel) {
        Bot_StartEvent(&CorePlugin, "Channel Message", &NewContext, 0, "channel=%s nick=%s fullhost=%s \ttext=%s\t", Event->Word[2], Source, RawSource,Event->WordEol[3]);
      } else
        fprintf(stderr, "Privmsg on channel that doesn't exist (%s)", Event->Word[2]);
    } else {
      Bot_MakeContext(&NewContext, CONTEXT_USER, Network);
      NewContext.User = RawSource;
      Bot_StartEvent(&CorePlugin, "Private Message", &NewContext, 0, "nick=%s fullhost=%s \ttext=%s\t", Source, RawSource, Event->WordEol[3]);
    }
  } else if(!strcasecmp(Command, "JOIN")) {
    if(!SourceIsMe)
      Bot_StartEvent(&CorePlugin, "Channel Join", Bot_MakeContext(&NewContext, CONTEXT_CHANNEL, Bot_FindChannel(Network, Event->Word[2])), 0, "channel=%s nick=%s fullhost=%s", Event->Word[2], Source, RawSource);
    else
      Bot_StartEvent(&CorePlugin, "Server You Join", SameContext, 0, "channel=%s", Event->Word[2]);
  } else if(!strcasecmp(Command, "PART")) {
    Bot_StartEvent(&CorePlugin, (!SourceIsMe)?"Channel Part":"Channel You Part", Bot_MakeContext(&NewContext, CONTEXT_CHANNEL, Bot_FindChannel(Network, Event->Word[2])), 0, "channel=%s nick=%s fullhost=%s text=\t%s\t", Event->Word[2], Source, RawSource, Event->WordEol[3]);
  } else if(!strcasecmp(Command, "NICK")) {
    Bot_StartEvent(&CorePlugin, (!SourceIsMe)?"Server Nick":"Server You Nick", SameContext, 0, "nick=%s newnick=%s fullhost=%s", Source, Event->Word[2], RawSource);
  } else if(!strcasecmp(Command, "TOPIC")) {
    Bot_StartEvent(&CorePlugin, (!SourceIsMe)?"Channel Topic":"Channel You Topic", Bot_MakeContext(&NewContext, CONTEXT_CHANNEL, Bot_FindChannel(Network, Event->Word[2])), 0, "nick=%s channel=%s fullhost=%s \ttext=%s\t", Source, Event->Word[2], RawSource, Event->WordEol[3]);
  } else if(!strcasecmp(Command, "KICK")) {
    Bot_StartEvent(&CorePlugin, (!strcasecmp(Event->Word[3], MyNick))?"Channel Kick":"Channel You Kicked", Bot_MakeContext(&NewContext, CONTEXT_CHANNEL, Bot_FindChannel(Network, Event->Word[2])), 0, "nick=%s channel=%s fullhost=%s target=%s \ttest=%s\t", Source, Event->Word[2], RawSource, Event->Word[3], Event->WordEol[4]);
  } else if(!strcasecmp(Command, "QUIT")) {
    Bot_StartEvent(&CorePlugin, (!SourceIsMe)?"Server Quit":"Server You Quit", SameContext, 0, "nick=%s fullhost=%s \ttext=%s\t", Source, RawSource, Event->WordEol[2]);
  }
  return ER_NORMAL;
}

void Bot_Segfault(int sig) {
  puts("segfault");
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

/* everything else */
int main(int argc, char *argv[]) {
  if(argc>=2)
    ConfigPath = argv[1];
  if(SDLNet_Init() < 0) {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  signal(SIGSEGV, Bot_Segfault);

  SocketSet = SDLNet_AllocSocketSet(Bot_GetConfigInt("Bot", "SocketSetSize", 10));
  if(!SocketSet) {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return 0;
  }

  FirstEvent = NULL;
  FirstPlugin = NULL;
  FirstConfig = NULL;
  FirstAsyncEvent = NULL;
  FirstNetwork = NULL;
  Bot_ReloadConfig(ConfigPath);
  Bot_ZeroStruct(CorePlugin);

  ContextInfo Context;
  Bot_AddEventHook(&CorePlugin, "Server Input", PRI_LOWEST, 0, 0, 0, Default_ServerInput);
  Bot_AddEventHook(&CorePlugin, "Server You Join", PRI_HIGHER, 0, 0, 0, Default_YouJoin);
  Bot_AddEventHook(&CorePlugin, "Channel You Part", PRI_LOWEST, 0, 0, 0, Default_YouPartKick);
  Bot_AddEventHook(&CorePlugin, "Channel You Kicked", PRI_LOWEST, 0, 0, 0, Default_YouPartKick);
  Bot_AddEventHook(&CorePlugin, "Client Command", PRI_LOWEST, 0, EF_ALREADY_HANDLED, 0, Default_ClientCommand);
  Bot_AddEventHook(&CorePlugin, "Server Output", PRI_LOWEST, 0, EF_ALREADY_HANDLED, 0, Default_ServerOutput);

  while(!NeedShutdown) {
    Bot_FreeAsyncEvents(0);
    for(PluginInfo *Mod=FirstPlugin;Mod;Mod=Mod->Next) {
      for(int i=0;i<NUM_TIMERS;i++) {
        TimerInfo *Timer = Mod->Timers[i];
        if(Timer && (Timer->EndTime <= time(NULL))) {
          if(Bot_ContextIsValid(&Timer->Context))
            Bot_StartEvent(Mod, Timer->EventType, &Timer->Context, Timer->EventFlags, "%s", Timer->EventText);
          else
            fprintf(stderr, "Invalid context for a timer\n");
          free(Timer);
          Mod->Timers[i] = NULL;
        }
      }
    }
    for(NetworkInfo *Net=FirstNetwork;Net;) {
      NetworkInfo *Next = Net->Next;
      if(time(NULL) >= (Net->ConnectTime+25)) {
        if(Net->PongDeadline) {
          if(time(NULL) > Net->PongDeadline) {
            char Tag[strlen(Net->Tag) + 1];
            strcpy(Tag, Net->Tag);
            Bot_ReconnectNetwork(Net);
          }
        } else {
          if((time(NULL)>(Net->LastConnectCheck))&&!((time(NULL) - Net->ConnectTime) % 30)) {
            time(&Net->LastConnectCheck);
            ContextInfo Context;
            Bot_StartEvent(&CorePlugin, "Server Output", Bot_MakeContext(&Context, CONTEXT_NETWORK, Net), 0, "PING a");
            Net->PongDeadline = time(NULL) + 10;
          }
        }
      }
      Net=Next;
    }
    if(SDLNet_CheckSockets(SocketSet, 500))
      for(NetworkInfo *Nets = FirstNetwork;Nets;Nets=Nets->Next)
        if(SDLNet_SocketReady(Nets->Socket)) {
          char ServerRead[1<<14]="";
          int Size = SDLNet_TCP_Recv(Nets->Socket,ServerRead,sizeof(ServerRead)-1);
          if(Size) {
            ServerRead[Size] = 0;
            char *Store, *Peek = strtok_r(ServerRead,"\n", &Store);
            while(Peek != NULL) {
              Bot_StartEvent(&CorePlugin, "Server Input", Bot_MakeContext(&Context, CONTEXT_NETWORK, Nets), 0, "%s", Peek);
              Peek = strtok_r(NULL, "\n", &Store);
            }
          }
        }
  }

  for(NetworkInfo *Net = FirstNetwork;Net;) {
    NetworkInfo *Next = Net->Next;
    Bot_DisconnectNetwork(Net, "shutting down");
    Net = Next;
  }

  for(PluginInfo *P = FirstPlugin;P;) {
    PluginInfo *Next = P->Next;
    Bot_UnloadPlugin(P);
    P = Next;
  }
  Bot_FreeAsyncEvents(1);

  Bot_UnloadPlugin(&CorePlugin);
  Bot_FreeConfig(FirstConfig);
  FirstConfig = NULL;
  SDLNet_Quit();

  SDLNet_FreeSocketSet(SocketSet);
  return EXIT_SUCCESS;
}
