#define INCL_DOS
#define INCL_KBD

#include <os2.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>


/*
 * REBOOT - command-line reboot utility for OS/2 2.x and later.
 * Version 1.01
 *
 * Reboots the machine exactly as CTRL-ALT-DEL would: DosShutdown()
 * commits all pending file-system data, then an undocumented ioctl
 * (category 0xd5, function 0xab) sent to the DOS$ device performs
 * the warm reset.  Method per "Rebooting OS/2", EDM/2 vol.5 no.9,
 * https://www.edm2.com/0509/reboot.html.
 *
 * Requires DOS.SYS in CONFIG.SYS:
 *
 *   DEVICE=C:\OS2\BOOT\DOS.SYS
 *
 * DOS.SYS exports the "DOS$" device opened below; without that line
 * DosOpen fails and there is no way to reboot from here.
 *
 * Usage:  REBOOT [/y[es]] [/?]
 *   REBOOT          ask "Rudely reboot system? (y/N)"; N or Enter aborts
 *   REBOOT /y[es]   reboot without asking
 *   REBOOT /?       print usage text
 *
 * History:
 *   1.00  Mark Kimes, original, public domain.
 *   1.01  Unbuffered stdout so banner/prompt/errors show immediately
 *         (KbdCharIn bypasses stdio, so nothing flushed the prompt);
 *         startup banner; help text cleaned up.  Now built with Open
 *         Watcom - see MAKEFILE.WAT and COMPILE.CMD.
 */

/*
 * Show the system message help for an APIRET error code by running
 * "CMD.EXE /C HELP.CMD SYSxxxx" synchronously (EXEC_SYNC, bsedos.h:585;
 * prototype bsedos.h:574).  DosExecPgm's argument block must have the
 * double-null form  cmd\0args...\0\0  [DOC-IBM CP Reference].  One
 * buffer serves as both program name and argument block: sprintf fills
 * it with "CMD.EXE /C HELP.CMD SYSxxxx", the NUL written at strlen+1
 * supplies the terminating pair, and runme[7] then splits program name
 * from arguments by overwriting the space.  pName reads up to the first
 * NUL; the OS walks the remaining strings as argv.  'object' receives
 * the name of the failing module if the exec cannot be loaded.
 */
VOID ShowHelp (APIRET rc) {

  static RESULTCODES rt;
  static CHAR        object[32],runme[CCHMAXPATH];

  sprintf(runme,"CMD.EXE /C HELP.CMD SYS%04u",rc);
  runme[strlen(runme) + 1] = 0;
  runme[7] = 0;
  DosExecPgm(object,sizeof(object),EXEC_SYNC,(PVOID)runme,NULL,&rt,(PSZ)runme);
}


/* Uppercase an ASCII letter; any other byte passes through unchanged. */
INT to_upper (INT key) {

  if(key >= 'a' && key <= 'z')
    return ((key) + 'A' - 'a');
  return key;
}


int main (int argc,char *argv[]) {

  HFILE  hf;
  APIRET rc;
  ULONG  action;
  CHAR   key = 'N';
  INT    x;
  BOOL   clerr = FALSE;

  /* stdout is block-buffered; KbdCharIn bypasses stdio, so nothing */
  /* would ever flush the prompt to the screen. Make it synchronous. */
  setvbuf(stdout,NULL,_IONBF,0);

  printf("Reboot 1.01\n");

  for(x = 0;x < argc;x++) {
    switch(*argv[x]) {
      case '/':
      case '-':
        switch(to_upper(argv[x][1])) {
          case '?':
            printf("\n  Usage:  REBOOT [/y[es]]\n"
                   "\nReboots the system as if you'd pressed CTRL-ALT-DEL.\n");
            return 0;
          case 'Y':
            key = 'Y';
            break;
          default:
            printf("\n **\07Unknown command line switch '%s'\n",argv[x]);
            DosSleep(1000L);
            clerr = TRUE;
            break;
        }
        break;
    }
  }

  if(clerr)
    key = 'N';

  /*
   * "DOS$" is exported by DOS.SYS (DEVICE=C:\OS2\BOOT\DOS.SYS); see
   * header comment.  Write access suffices - the driver only needs an
   * open handle to receive the reboot ioctl further down.
   */
  rc = DosOpen("DOS$", &hf, &action, 0L, FILE_NORMAL, FILE_OPEN,
               OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE |
               OPEN_FLAGS_FAIL_ON_ERROR, 0L);
  if(!rc){
    if(key != 'Y') {  /* if /y not on command line */

      KBDKEYINFO kbd;

      /*
       * Drain the type-ahead queue so keystrokes typed before the
       * prompt cannot answer it.  Poll with IO_NOWAIT (bsesub.h:122);
       * each success consumed one event.  A nonzero return means the
       * queue is empty.  Stop early when an event is not a "final"
       * character: KBDKEYINFO.fbStatus bit 6 set marks final chars,
       * clear means interim/shift-only reports [DOC-IBM CP Reference -
       * KbdCharIn].
       */
      while(!KbdCharIn(&kbd,IO_NOWAIT,0)) {
        if(!(kbd.fbStatus & 0x40))
          break;
      }
      printf("\n\07Rudely reboot system? (y/N) ");
      /* Accept only final characters as the answer (see bit 6 above). */
      if(!KbdCharIn(&kbd,IO_WAIT,0)) {
        if(kbd.fbStatus & 0x40)
          key = to_upper(kbd.chChar);
      }
      printf("%s\n",(key == 'Y') ? "Y" : "N\n  **Aborted.");
    }
    if(key == 'Y') {  /* we got the green light */
      /*
       * Flush all file-system lazy writes first (bsedos.h:1790), then
       * hand the open DOS$ handle to the undocumented warm-reset call:
       * category 0xd5 / function 0xab, no parameter or data packet.
       * On success the CPU restarts before anything past the ioctl
       * (including DosClose) ever runs.
       */
      rc = DosShutdown(0L);
      if(!rc) {
        rc = DosDevIOCtl(hf,0xd5,0xab,NULL,0,NULL,NULL,0,NULL);
        if(rc)
          printf("\n **\07DosDevIOCtl 0xd5/0xab failed, can't reboot.\n");
      }
      else
        printf("\n **\07DosShutdown failed, won't reboot.\n");
    }
    DosClose(hf);
  }
  else
    printf("\n **\07DOS.SYS not installed, can't reboot.\n");
  if(rc)
    ShowHelp(rc);
  return rc;
}
