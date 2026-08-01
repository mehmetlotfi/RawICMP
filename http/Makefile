CC      = gcc
CFLAGS  = -lpthread

TARGET  = http 
SOURCE  = http.c

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin

.PHONY: all clean install

all:
	@printf "  CC      %s\n" "$(TARGET)"
	@$(CC) $(SOURCE) $(CFLAGS) -o $(TARGET)

clean:
	@printf "  CLEAN   %s\n" "$(TARGET)"
	@rm -f $(TARGET)

install: all
	@printf "  INSTALL %s -> %s\n" "$(TARGET)" "$(BINDIR)"
	@install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
