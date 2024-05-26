#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "logging.h"
#include "signal_handler.h"
#include "../core/config.h"

/*
 * @param int sig signal number
 * @param void (func)(int) the "real" function that handler the signal
 * The sigaction() system call is used to change the action taken by a process
 * on receipt of a specific signal (except SIGKILL and SIGSTOP)
 */
void setup_signals(int sig, void (func)(int)) {
  struct sigaction sa = {
    .sa_flags = SA_RESTART, // resume the library function or return a failure
    .sa_handler = func // set the func as a signal_handler
  };
  sigemptyset(&sa.sa_mask); // initialize a signal mask to esclude all signals
  if (sigaction(sig, &sa, NULL) == -1) {
		log_error("[%s] (%s) Failed to setup signal->action! Cause: %s\n",
               __FILE_NAME__, __func__, strerror(errno));
    perror("sigaction: ");
    exit(errno);
  }
}
