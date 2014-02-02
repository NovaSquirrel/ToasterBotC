-----About-----
ToasterBot is an event-based IRC bot framework written by NovaSquirrel and licensed under the GPL, version 3.


-----Configuration-----


-----Module API documentation-----
  Bot_RunEvent(BotEvent *Event);
Runs whatever handlers are registered for an event type. If the EF_ASYNCHRONOUS flag is on, event handlers will run in a separate thread that can't crash the main one

  Bot_StartEvent(PluginInfo *Plugin, const char *Type, ContextInfo *WhatContext, unsigned long Flags, const char* Format, ...)
  Bot_ParamCopyEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, BotEvent *BaseEvent, const char* Format, ...)
Create an event and start running whatever handlers are set for it

  Bot_StartDelayedEvent(PluginInfo *Plugin, const char *Type, ContextInfo *Context, unsigned long Flags, unsigned long Seconds, BotEvent *Base, const char* Format, ...)
Sets up an event to happen in a specified number of Seconds. Returns an index into the plugin's Timers array, or -1 if error. Uses Base as a base like ParamCopyEvent.

  Bot_ParseINI(FILE *File, void (*Handler)(const char *Group, const char *Item, const char *Value))
Parses a .ini file passed in the file handle. Second argument is a callback to run for every item in the .ini

  Bot_FreeChannel(ChannelInfo *Channel)
  Bot_FreeNetwork(NetworkInfo *Network)
Gets a ChannelInfo or NetworkInfo ready to be free()'d by freeing anything inside

  Bot_StrCpy(char *Destination, const char *Source, int MaxLength)
Like strcpy() but with protection against buffer overflows

  Bot_MemCaseCmp(const char *Text1, const char *Text2, int Length);
Like memcmp() but case insensitive

  Bot_TokenizeEvent(BotEvent *Event)
Tokenizes an Event's text to give easy access to the words inside
Affected by the following event flags:
  EF_NO_TOKENIZATION - don't try to tokenize at all, leave event untouched
  EF_NO_MULTIWORD    - don't support multi-word parameters
  EF_KEEP_COLONS     - keep colons at the start of words

  Bot_FindEventParam(BotEvent *Event, const char *Param);
Looks for a parameter named Param in Event, and returns its index or -1 if not found

  Bot_GetEventInt(BotEvent *Event, const char *Param, int Default);
  Bot_GetEventStr(BotEvent *Event, const char *Param, const char *Default);
Look up a parameter named Param in Event, and return it as an integer or a string, and return Default if not found

  Bot_FindNetwork(const char *Name)
Looks for a network whose tag is Name and return a pointer to it or NULL

  Bot_FindChannel(NetworkInfo *Network, const char *Name)
Find a channel named Name in Network and return a pointer to it or NULL

  Bot_ChangeParams(BotEvent *Event, BotEvent *NewEvent, const char *NewParams)
Change NewEvent's parameters to match Event's, with the list of params in NewParams added and using the value in NewParams if present in both

  Bot_SplitUserHost(const char *FullHost, char *Buffer, char **User, char **Host)
Splits a nick!user@host hostmask specified in FullHost. Stores split host to Buffer

  Bot_MakeContext(ContextInfo *Context, int Type, void *Pointer)
Makes a context in Context with the given Type and Pointer and returns it. If you want to set User you have to set it yourself.

  Bot_GetInfo(ContextInfo *Context, const char *Info)
Returns a string. Shouldn't be needed in most cases since you can just get the information elsewhere.
Recognized arguments:
  NetTag - Network tag
  Network - What the network calls itself
  Server - What the server calls itself
  Nick - Your current nick on the server
  Username - Your username on the server
  ConnectPass - 
  FullHost - Your full nick!user@host hostmask
  Host - URL that's used to connect to the network
  Channel - Channel name

  Bot_AddEventHook(PluginInfo *Plugin, const char *Type, int Priority, int Flags, int Need0, int Need1, void *Handler)
Add an EventHook to run Handler when event Type occurs, at priority Priority, but only run if all flags in Need0 are off and all flags in Need1 are on. Flags not currently used. Returns the EventHook it creates

  Bot_DelEventHook(EventHook *Hook)
Unregisters and frees an EventHook

  Bot_AddConfigPair(const char *Group, const char *Item, const char *Value)
Adds a config pair to the list. Meant to be used as a callback for Bot_ParseINI

  Bot_FindConfigPair(ConfigPair *Find, const char *Name)
Find a config pair in a list starting with Find (pass the Item member of a Config if you wnat to search inside it) with a name of Name, and return it or NULL if not found

  Bot_ReloadConfig(const char *Filename)
Reload the configuration, including loading any modules listed in it not already loaded and autoconnecting to networks

  Bot_FreeConfig(ConfigPair *Free)
Frees the ConfigPair and all siblings

  Bot_LoadPlugin(const char *Name)
Loads plugin with filename Name and return a PluginInfo pointer for it

  Bot_UnloadPlugin(PluginInfo *Plugin)
Unload a plugin

  Bot_PluginIsPresent(char *Module);
Returns 1 if a plugin named Module is present

  Bot_PluginSymbol(char *Module, char *Name);
Finds symbol Name in plugin Module and returns it

  Bot_FindPlugin(char *Name)
Finds and returns a plugin by name

  Bot_GetConfigStr(const char *GroupName, const char *ItemName, const char *DefaultValue);
  Bot_GetConfigInt(const char *GroupName, const char *ItemName, int DefaultValue);
Get a config string or integer, or return DefaultValue if not found

  Bot_IndexInStrList(const char *Search, const char* List, ...);
Returns the index number of Search in the list; list must end in NULL, and returns -1 if not found

  Bot_AsyncRunEvent(void *Event2);
Run an asynchronous event

  Bot_XChatTokenize(const char *Input, char *WordBuff, int WordBuffSize, const char **Word, const char **WordEol, int WordSize);
Do XChat-style tokenizing on some text. Spaces can be included in a word by enclosing the word in "s

  Bot_FreeAsyncEvents(int Join);
Free all asynchronous events that are marked for freeing, and join their threads if Join is nonzero

  Bot_ConnectNetwork(const char *NetTag)
Connects to a network and returns a pointer to it

  Bot_DisconnectNetwork(NetworkInfo *Network, const char *QuitMessage)
Disconnects from a network

  Bot_NameFromContext(ContextInfo *Info, char *Write);
Gets the name of a context, whether it be a user or a channel, meant for making destinations for privmsgs

  Bot_PluginSetHandler(PluginInfo *Plugin, const char *Name, void *Handler)
Recognizes:
  "CPD Alloc" - allocates channel plugin data
  "CPD Free" - free a channel's plugin data

  Bot_ChannelPluginData(PluginInfo *Plugin, ChannelInfo *Channel, unsigned int Flags)
Gets the pointer to Plugin-specific metadata for a given channel and allocates if flag CPD_AUTO_CREATE is on.

  Bot_WildMatch(const char *TestMe, const char *Wild)
Does a wildcard compare on some text. Returns 1 if matching

  Bot_GetGlobalPointer(const char *Name)
Returns a pointer to a global variable
Recognizes only FirstNetwork and FirstConfig.

  Bot_ContextIsValid(ContextInfo *Context)
Makes sure a context points to something that actually exists. Returns 1 if so
