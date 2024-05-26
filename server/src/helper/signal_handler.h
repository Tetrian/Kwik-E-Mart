#ifndef SIGNAL_HANDLER_H_
#define SIGNAL_HANDLER_H_

/** Helper function used to setup a signal handler to stop the execution gracefully */
void setup_signals(int, void (func)(int));

#endif
