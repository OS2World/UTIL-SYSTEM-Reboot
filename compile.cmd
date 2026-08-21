@echo off
rem Build UTIL-SHUTDOWN-Reboot with Open Watcom.
rem Run from the project directory. Needs on PATH: wcl386/wlink (Watcom env
rem set up), GNU make, tee. Log lands in compile.log.
rem
rem The EMXOMFLD_* variables below configure emxomfld (GCC -Zomf builds).
rem They are inert for this pure-Watcom makefile; kept so the same script
rem shape works across projects.
set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe
set EMXOMFLD_PRELINK=0
make -f makefile.wat clean
make -f makefile.wat %1 2>&1 | tee compile.log
