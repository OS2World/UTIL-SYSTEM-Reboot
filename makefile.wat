# makefile.wat - Open Watcom build for UTIL-SHUTDOWN-Reboot
#
# Target: reboot.exe - 32-bit OS/2 character-mode (VIO) executable.
#
# Usage:   make -f makefile.wat          (builds reboot.exe)
#          make -f makefile.wat clean
#
# Notes:
#  - Uses the wcl386 driver (one-step compile+link). Per the toolkit recipe,
#    -bt=os2 selects the OS/2 target and -l=os2v2 links the 32-bit
#    character-mode system (wlink backend, correct startup + libs).
#  - Recipes below assume a sh-style shell (GNU make from the kLIBC RPMs on
#    ArcaOS). If you drive this with Watcom's own wmake instead, change
#    'rm -f' to 'del'.
#  - reboot.c opens DOS$ (needs DEVICE=C:\OS2\BOOT\DOS.SYS in CONFIG.SYS).

CC      = wcl386
CFLAGS  = -bt=os2 -l=os2v2 -ox

TARGET  = reboot.exe
SRCS    = reboot.c
OBJS    = reboot.obj

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -fe=$@ $(SRCS)

clean:
	rm -f $(TARGET) $(OBJS) compile.log

.PHONY: all clean
