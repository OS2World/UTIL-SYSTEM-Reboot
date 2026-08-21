# UTIL-SYSTEM-Reboot
Reboot command line utility for OS/2.

Reboots the system immediately, as if you had pressed CTRL-ALT-DEL,
by issuing a DosShutdown() followed by an undocumented reboot ioctl
(0xd5/0xab) against the DOS$ device driver.

VERSION 1.01
===============
* Shows a "Reboot 1.01" banner at startup.
* Fixed: the "(y/N)" confirmation prompt was written through block-buffered
  stdout while keyboard input is read directly via KbdCharIn(), so nothing
  appeared on screen until the program exited (or the machine rebooted).
  stdout is now unbuffered and all prompts/messages display immediately.
* Cleaned up the /? help text.
* Built with Open Watcom as an OS/2 32-bit text-mode executable.

USAGE
===============
REBOOT [/y[es]] [/?]

  REBOOT          Asks "Rudely reboot system? (y/N)". Y reboots,
                  N or Enter aborts.
  REBOOT /y       Reboots immediately without asking.
  REBOOT /yes     Same as /y.
  REBOOT /?       Shows the usage text.

WARNING: this is a rude reboot. Unsaved work in all running programs
is lost, exactly as if you had pressed CTRL-ALT-DEL.

REQUIREMENTS
===============
The DOS$ device driver must be loaded. Check that CONFIG.SYS contains:

  DEVICE=C:\OS2\BOOT\DOS.SYS

If the line is missing, add it and reboot once for it to take effect.
Without it, the program prints "**DOS.SYS not installed, can't reboot."

COMPILE TOOLS
===============
* Open Watcom C/C++ (tested with Watcom 2.0.1), target OS/2 32-bit.
* Run COMPILE.CMD (in an OS/2 session) to build REBOOT.EXE with
  MAKEFILE.WAT, or by hand:

    wcl386 -bt=os2 -l=os2v2 -ox -fe=reboot.exe reboot.c

* Original toolchain (see comment in REBOOT.C):
  icc /G4 /O+ /Gs+ /W3 /Kb /B"/RUNFROMVDM" /B"/STACK:16384" /Rn reboot.c

LICENSE
===============
* BSD 3 Clauses

AUTHORS
===============
* Mark Kimes
* Hector

LINKS
===============
*
