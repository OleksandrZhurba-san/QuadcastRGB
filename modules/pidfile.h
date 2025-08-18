#pragma once
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initializes internal path to: $XDG_RUNTIME_DIR/quadcastrgb.pid
// (falls back to /tmp/quadcastrgb.$UID.pid). Call once at startup.
void pidfile_init(void);

// Returns 1 if another instance was found and killed, 0 if none existed.
// Waits up to timeout_ms for old proc to exit. Uses SIGTERM then SIGKILL.
int pidfile_takeover(int timeout_ms);

// Write current process' PID to the file.
void pidfile_write_self(void);

// Remove the pidfile (call on clean exit or from a signal handler).
void pidfile_remove(void);

// Convenience: install SIGINT/SIGTERM handlers that remove pidfile then
// re-raise.
void pidfile_install_signal_cleanup(void);

#ifdef __cplusplus
}
#endif
