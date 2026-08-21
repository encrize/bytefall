CC = gcc
PKG_CONFIG ?= pkg-config

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
endif

TARGET := bytefall$(EXEEXT)
OBJDIR := obj
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

SDL2_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl2)
SDL2_LIBS := $(shell $(PKG_CONFIG) --libs sdl2)

CPPFLAGS += $(SDL2_CFLAGS)
CFLAGS ?= -O2
CFLAGS += -Wall -Wextra -std=c11
LDLIBS += $(SDL2_LIBS) -lm

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $@

install: $(TARGET)
	install -Dm755 $(TARGET) "$(DESTDIR)$(BINDIR)/$(TARGET)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"

clean:
	rm -rf $(OBJDIR) bytefall bytefall.exe

$(OBJS): app.h config.h
