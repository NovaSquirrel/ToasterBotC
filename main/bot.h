#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h> // for execl
#include <dlfcn.h>  // for libdl
#include <pthread.h>
#include <SDL/SDL_net.h>
#include "../main/numeric.h"
#define BOTEVENT_WORDSIZE 32
#define BOTEVENT_TEXTSIZE 4096

/* enum definitions */
enum ContextTypes {
  CONTEXT_NONE,
  CONTEXT_NETWORK,
  CONTEXT_CHANNEL,
  CONTEXT_USER,
};

enum Priorities {
  PRI_DISABLED = 0,
  PRI_LOWEST = 1,
  PRI_LOWER = 5,
  PRI_LOW = 10,
  PRI_NORMAL = 25,
  PRI_HIGH = 50,
  PRI_HIGHER = 75,
  PRI_HIGHEST = 100,
};

enum EventFlags {
  EF_NO_TOKENIZATION = 1, // don't try to tokenize the event text
  EF_NO_MULTIWORD    = 2, // don't allow to make multi-word attribute values
  EF_ALREADY_HANDLED = 4, // still present, but don't handle by anything else
  EF_KEEP_COLONS     = 8, // keep colons, don't hide them
  EF_ASYNCHRONOUS   = 16, // use a separate thread to handle this event
};

enum EventReturnValue {
  ER_NORMAL,  // don't change anything
  ER_HANDLED, // don't delete, but mark already handled
  ER_DELETE,  // delete event, stop anything at all from reacting to it
};

/* struct definitions */
typedef struct ConfigPair {
  char Name[40];
  char *Value;
  struct ConfigPair *Next;
  struct ConfigPair *Item;
} ConfigPair;

#define NUM_TIMERS 12
enum TimerFlags {
  TIMER_RUN_ON_UNLOAD = 1,
};

typedef struct PluginInfo {
  char Name[32];
  char FancyName[64];
  char Version[32];
  char Tag[32];
  char Path[64];
  void *DL;
  void *Timers[NUM_TIMERS];
  void *(*ChannelPluginData_Alloc)(void *ChannelInfo, unsigned char Type);
  void (*ChannelPluginData_Free)(void *Data, unsigned char Type);
  struct PluginInfo *Prev;
  struct PluginInfo *Next;
} PluginInfo;

typedef struct ScrollbackInfo {
  int NumLines;
  char *Line[700];
} ScrollbackInfo;

enum NetFlags {
  NETFLAG_HAD_PING = 1,
};

typedef struct NetworkInfo {
  unsigned long Flags;
  char Name[80];        // name as given by the network
  char Nick[80];        // bot's nick
  char Username[40];    // username to use
  char NickPass[80];    // bot nickserv password
  char ServerName[80];  // name given by server
  char ConnectURL[128]; // for reconnecting
  char ConnectPass[80]; // password to connect to server (for ZNC)
  char FullHost[256];   // full *!*@* host as given by the server
  char Tag[16];         // short network identifier
  IPaddress IP;
  TCPsocket Socket;
  unsigned short Port;
  void *FirstChannel;
  time_t ConnectTime, PongDeadline, LastConnectCheck;
  struct NetworkInfo *Prev, *Next;
} NetworkInfo;

typedef struct ChannelPluginData {
  struct ChannelPluginData *Next;
  PluginInfo *Plugin;
  void *Data;
  unsigned char Type;
} ChannelPluginData;

enum ChanFlags {
  CHANFLAG_JOINED = 1,
  CHANFLAG_TRYJOIN = 2,
};

typedef struct ChannelInfo {
  unsigned long Flags;
  char Name[50];
  NetworkInfo *Network; 
  struct ChannelInfo *Prev, *Next;
  ScrollbackInfo *Scrollback;
  ChannelPluginData *PluginData;
} ChannelInfo;

typedef struct ContextInfo {
  int Type;
  union {
    void *Generic;
    NetworkInfo *Network;
    ChannelInfo *Channel;
  } Target;
  char *User;
} ContextInfo;

typedef struct TimerInfo {
  PluginInfo *Plugin;
  time_t StartTime, EndTime;
  unsigned int Seconds;
  unsigned long Flags;
  ContextInfo Context;
  unsigned long EventFlags;
  char EventType[64];
  char EventText[BOTEVENT_TEXTSIZE];
} TimerInfo;

typedef struct BotEvent {
  ContextInfo *Context;
  char Type[64];
  unsigned long Flags;
  PluginInfo *Plugin;
  char Text[BOTEVENT_TEXTSIZE];     // stores a message
  char TextWord[BOTEVENT_TEXTSIZE]; // same but with spaces replaced with zeros
  int WordLen;                      // number of words
  char *Word[BOTEVENT_WORDSIZE];    // specific word
  char *WordEol[BOTEVENT_WORDSIZE]; // specific word to end of line
} BotEvent;

typedef struct EventHook {
  char Type[64];
  unsigned int Priority;
  unsigned long Flags;
  unsigned long EventFlagsNeed0; // these flags need to be 0 to trigger hook
  unsigned long EventFlagsNeed1; // these flags need to be 1 to trigger hook
  PluginInfo *Plugin;
  int (*Handler)(BotEvent *Event);
  struct EventHook *Prev, *Next;
  struct EventHook *PrevType, *NextType;
} EventHook;

typedef struct AsyncEventInfo {
  pthread_t Thread;
  BotEvent Event;
  unsigned char Finished;
  struct AsyncEventInfo *Prev;
  struct AsyncEventInfo *Next;
} AsyncEventInfo;

/* function prototypes */
int Bot_StartEvent(PluginInfo *Plugin, const char *Type, ContextInfo *WhatContext, unsigned long Flags, const char* Format, ...) __attribute__ ((format (printf, 5, 6)));
int Bot_ParamCopyEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, BotEvent *BaseEvent, const char* Format, ...)  __attribute__ ((format (printf, 6, 7)));
int Bot_StartDelayedEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, unsigned long Seconds, BotEvent *BaseEvent, const char* Format, ...) __attribute__ ((format (printf, 7, 8)));
int Bot_ParseINI(FILE *File, void (*Handler)(const char *Group, const char *Item, const char *Value));
int Bot_RunEvent(BotEvent *Event);
void Bot_FreeChannel(ChannelInfo *Channel);
void Bot_FreeNetwork(NetworkInfo *Network);
void Bot_StrCpy(char *Destination, const char *Source, int MaxLength);
void Bot_TokenizeEvent(BotEvent *Event);
int Bot_FindEventParam(BotEvent *Event, const char *Param);
int Bot_GetEventInt(BotEvent *Event, const char *Param, int Default);
const char *Bot_GetEventStr(BotEvent *Event, const char *Param, const char *Default);
int Bot_MemCaseCmp(const char *Text1, const char *Text2, int Length);
NetworkInfo *Bot_FindNetwork(const char *Name);
ChannelInfo *Bot_FindChannel(NetworkInfo *Network, const char *Name);
int Bot_ChangeParams(BotEvent *Event, BotEvent *NewEvent, const char *NewParams);
int Bot_SplitUserHost(const char *FullHost, char *Buffer, char **User, char **Host);
ContextInfo *Bot_MakeContext(ContextInfo *Context, int Type, void *Pointer);
const char *Bot_GetInfo(ContextInfo *Context, const char *Info);
EventHook *Bot_AddEventHook(PluginInfo *Plugin, const char *Type, int Priority, int Flags, int Need0, int Need1, void *Handler);
int Bot_DelEventHook(EventHook *Hook);
void Bot_AddConfigPair(const char *Group, const char *Item, const char *Value);
ConfigPair *Bot_FindConfigPair(ConfigPair *Find, const char *Name);
void Bot_ReloadConfig(const char *Filename);
void Bot_FreeConfig(ConfigPair *Free);
PluginInfo *Bot_LoadPlugin(const char *Name);
int Bot_UnloadPlugin(PluginInfo *Plugin);
int Bot_PluginIsPresent(char *Module);
void *Bot_PluginSymbol(char *Module, char *Name);
PluginInfo *Bot_FindPlugin(char *Name);
const char *Bot_GetConfigStr(const char *GroupName, const char *ItemName, const char *DefaultValue);
int Bot_GetConfigInt(const char *GroupName, const char *ItemName, int DefaultValue);
int Bot_IndexInStrList(const char *Search, const char* List, ...);
void *Bot_AsyncRunEvent(void *Event2);
void Bot_XChatTokenize(const char *Input, char *WordBuff, int WordBuffSize, const char **Word, const char **WordEol, int WordSize);
void Bot_FreeAsyncEvents(int Join);
NetworkInfo *Bot_ConnectNetwork(const char *NetTag);
void Bot_DisconnectNetwork(NetworkInfo *Network, const char *QuitMessage);
int Bot_ReconnectNetwork(NetworkInfo *Network);
NetworkInfo *Bot_EstablishConnection(NetworkInfo *Net, const char *Tag);
const char *Bot_NameFromContext(ContextInfo *Info, char *Write);
int Bot_PluginSetHandler(PluginInfo *Plugin, const char *Name, void *Handler);
#define CPD_AUTO_CREATE 256
void *Bot_ChannelPluginData(PluginInfo *Plugin, ChannelInfo *Channel, unsigned int Flags);
int Bot_WildMatch(const char *TestMe, const char *Wild);
void *Bot_GetGlobalPointer(const char *Name);
int Bot_ContextIsValid(ContextInfo *Context);
#define Bot_ZeroStruct(x) memset(&x, sizeof(x), 0);
