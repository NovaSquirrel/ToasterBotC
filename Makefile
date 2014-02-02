all: bot module/test.so module/botcmd.so module/cmdpack.so module/admin.so

bot: main/bot.c main/bot.h
	gcc -o bot -g -Wl,-export-dynamic main/bot.c -ldl -lSDL_net -Wall -std=gnu99

module/%.so: module/%.c main/bot.h
	gcc -o $@ -Wl,-export-dynamic -Wall -shared -std=gnu99 -fPIC $<


