#define __GNU_SOURCE
#include "pidfile.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static char g_pidpath[256];

static const char *pidfile_path(void) {
  if (g_pidpath[0]) {
    return g_pidpath;
  }
  const char *xdg = getenv("XDG_RUNTIME_DIR");
  if (xdg && xdg[0]) {
    snprintf(g_pidpath, sizeof(g_pidpath), "%s/quadcastrgb.pid", xdg);
  } else {
    snprintf(g_pidpath, sizeof(g_pidpath), "/tmp/quadcastrgb.%d.pid",
             (int)getuid());
  }
  return g_pidpath;
}
void pidfile_init(void) { (void)pidfile_path(); }

static int read_pid(pid_t *out) {
  FILE *f = fopen(pidfile_path(), "r");
  if (!f) {
    return -1;
  }
  long v = -1;
  if (fscanf(f, "%ld", &v) != 1) {
    fclose(f);
    return -1;
  }
  fclose(f);
  if (v <= 1) {
    return -1;
  }
  *out = (pid_t)v;
  return 0;
}

static int proc_alive(pid_t pid) { return (pid > 1) && (kill(pid, 0) == 0); }

static void sleep_ms(int ms) {
  struct timespec ts = {.tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000L};
  nanosleep(&ts, NULL);
}

int pidfile_takeover(int timeout_ms) {
  pid_t oldpid;

  // NOTE: no file
  if (read_pid(&oldpid) != 0) {
    return 0;
  }
  if (!proc_alive(oldpid)) {
    unlink(pidfile_path());
    return 0;
  }

  kill(oldpid, SIGTERM);

  int waited = 0;
  while (waited < timeout_ms && proc_alive(oldpid)) {
    sleep_ms(50);
    waited += 50;
  }
  if (proc_alive(oldpid)) {
    kill(oldpid, SIGKILL);
    sleep_ms(100);
  }
  unlink(pidfile_path());
  return 1;
}

void pidfile_write_self(void) {
  FILE *f = fopen(pidfile_path(), "w");
  if (!f)
    return;
  fprintf(f, "%ld\n", (long)getpid());
  fclose(f);
}

void pidfile_remove(void) { unlink(pidfile_path()); }

static void sig_cleanup(int sig) {
  pidfile_remove();
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  raise(sig);
}

void pidfile_install_signal_cleanup(void) {
  signal(SIGINT, sig_cleanup);
  signal(SIGTERM, sig_cleanup);
}
