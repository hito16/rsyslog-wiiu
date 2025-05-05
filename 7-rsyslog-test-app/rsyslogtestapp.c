/* An example of headless logging with some added realiablity and ease of use.

UDP logging is lacking.  The best way to debug requires buying a special USB
cable.

As a middle ground, we add a rsyslog hook that redirects stdout  to a
rsyslogd TCP server running in a supplied Docker container.

  1. Per ../rsyslog/README.md, start the Dockerfile.rsyslogd and
  2. wiiload <this file>.rpx
  4. Per ./rsyslog/README.md, open a termial to the docker instance and tail the
generated output.
 */

#include <coreinit/thread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

// normally you only include rsyslog-wiiu.h,  call init_rsyslogger(),
// then rely on printf()
#include "rsyslog-wiiu.h"
// but for this example, we want to show off rsyslog_send_tcp()
#include "rsyslog.h"

/* Force a draw immediately after WHBLogPrintf
   Nothing will be written to the screen without WHBLogConsoleDraw
*/

int main(int argc, char **argv)
{
    WHBProcInit();

    /*  Use the Console backend for WHBLog - this one draws text with OSScreen
        Don't mix with other graphics code! */
    WHBLogConsoleInit();

#ifdef SYSLOG_SERVER
    WHBLogPrintf("You passed in -DSYSLOG_SERVER %s with a CFLAG \n", SYSLOG_SERVER);
    WHBLogPrint("We could have used this, but we will use detection instead");
    WHBLogConsoleDraw();
#endif

    /*  Initialize logging - find syslog ip address, and configure stdout redirection
        Since this is meant for debugging, client will block until Syslog server is reached
        Typically you'd gate logging, such as
    #ifdef DEBUG
        if (init_rsyslogger() == 0) {
             ...
        }
    #endif
       Once initialized, printf()'s will just go to your remote syslog.
    */

    if (init_rsyslogger() == 0)
    {
        WHBLogPrintf("Discovered syslog IP %s", SYSLOG_IP);
        WHBLogConsoleDraw();
    }
    else
    {
        WHBLogPrint("No IP discovered, Shutting Down...");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(5000));
        WHBLogConsoleFree();
        WHBProcShutdown();
    }

    WHBLogPrintf("== Starting rsyslog test [%s]...", SYSLOG_IP);
    WHBLogPrint("   by now you're also tailing your Docker instance logs");
    WHBLogPrint("   ex.  tail -f /var/log/remote/*");
    WHBLogPrint("");
    WHBLogConsoleDraw();

    int times_left = 5;
    while (WHBProcIsRunning() && times_left > 0)
    {
        WHBLogPrintf("== Logging with RAW api rsyslog_send_tcp()");

        rsyslog_send_tcp(SYSLOG_IP, 9514, 14, "Wrote from  rsyslog_send_tcp()");
        times_left--;
        WHBLogPrintf("== done. Running again (attempts = %d)", times_left);
        WHBLogPrintf("");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(1000));
    }

    /*  The big test.  Where does printf() go??  */

    times_left = 5;
    while (WHBProcIsRunning() && times_left > 0)
    {
        WHBLogPrint("== Logging with printf...");
        WHBLogConsoleDraw();
        printf("Wrote from printf() -  %d\n", times_left);
        fprintf(stdout, "fprintf(stdout, ...) %d\n", times_left);
        fprintf(stderr, "fprintf(stderr, ...) %d\n", times_left);
        times_left--;
        OSSleepTicks(OSMillisecondsToTicks(3000));
    }

    /*  If we get here, ProcUI said we should quit. */
    WHBLogPrintf("Shutting Down...");
    WHBLogConsoleDraw();

    WHBLogConsoleFree();
    WHBProcShutdown();

    return 0;
}
