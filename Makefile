CC      = gcc
# Добавляем -static для полной статической линковки
CFLAGS  = -Wall -Wextra -O2 -std=c11 $(shell sdl2-config --cflags)
# Указываем библиотеки для Windows, чтобы линкер не искал их в системе
LIBS    = -static $(shell sdl2-config --libs) -lwinmm -limm32 -lole32 -loleaut32 -lversion -lpropsys -lsetupapi -lcfgmgr32 -lm

SRCDIR  = .
OBJDIR  = obj
SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET  = bytefall.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

# Зависимости
$(OBJS): app.h