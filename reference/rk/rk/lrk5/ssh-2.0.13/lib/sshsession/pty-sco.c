/*

pty-sco.c

Author: Tatu Ylonen <ylo@ssh.fi>

Copyright (c) 1997 SSH Communications Security, Finland
                   All rights reserved

Allocating a pty on SCO Unix.

*/

#include "sshsessionincludes.h"
#include "pty-int.h"
#include "sshunixptystream.h"

/* Allocates a pty using a machine-specific method, and returns the
   master side pty in *ptyfd, the child side in *ttyfd, and the name of the
   device in namebuf.  Returns TRUE if successful. */

Boolean ssh_pty_internal_allocate(int *ptyfd, int *ttyfd, char *namebuf)
{
  char buf[64];
  int i;
  
  for (i = 0; i < 256; i++)	/* max 256 pty's possible */
    {
      snprintf(buf, sizeof(buf), "/dev/ptyp%d", i);
      *ptyfd = open(buf, O_RDWR|O_NOCTTY);
      if (*ptyfd < 0)
	{
          if (errno == ENODEV)	    /* all used up */
	    {
	      ssh_warning("pty_allocate: all allocated ptys (%d) used", i);
	      return FALSE;
	    }
	  continue;
	}
      
      snprintf(namebuf, SSH_PTY_NAME_SIZE, "/dev/ttyp%d", i);
      
      /* Open the slave side. */

      *ttyfd = open(namebuf, O_RDWR|O_NOCTTY);
      if (*ttyfd < 0)
	{
	  ssh_warning("%.100s: %.100s", namebuf, strerror(errno));
	  close(*ptyfd);
	  return FALSE;
	}
      return TRUE;
    }
  ssh_warning("Failed to allocate pty.");
  return FALSE;
}

/* Makes the given tty the controlling tty of the current process.
   This may close and reopen the original file descriptor.  When called,
   *ttyfd should be a valid file descriptor for the slave side, and ttyname
   should contain its name (e.g., "/dev/ttyp3").  Returns FALSE if the
   controlling tty could not be set. */

Boolean ssh_pty_internal_make_ctty(int *ttyfd, const char *ttyname)
{
  int fd;
  
  /* First disconnect from the old controlling tty. */
#ifdef TIOCNOTTY
  fd = open("/dev/tty", O_RDWR|O_NOCTTY);
  if (fd >= 0)
    {
      (void)ioctl(fd, TIOCNOTTY, NULL);
      close(fd);
    }
#endif /* TIOCNOTTY */
  
  /* Verify that we are successfully disconnected from the controlling tty. */
  fd = open("/dev/tty", O_RDWR|O_NOCTTY);
  if (fd >= 0)
    {
      ssh_warning("Failed to disconnect from controlling tty.");
      close(fd);
    }

  /* Make it our controlling tty. */
#ifdef TIOCSCTTY
  ssh_debug("Setting controlling tty using TIOCSCTTY.");
  /* We ignore errors from this, because HPSUX defines TIOCSCTTY, but returns
     EINVAL with these arguments, and there is absolutely no documentation. */
  ioctl(*ttyfd, TIOCSCTTY, NULL);
#endif /* TIOCSCTTY */

#ifdef HAVE_SETPGID
  /* This appears to be necessary on some machines...  */
  setpgid(0, 0);
#endif

  fd = open(ttyname, O_RDWR);
  if (fd < 0)
    ssh_warning("%.100s: %.100s", ttyname, strerror(errno));
  else
    close(fd);

  /* Verify that we now have a controlling tty. */
  fd = open("/dev/tty", O_WRONLY);
  if (fd < 0)
    {
      ssh_warning("open /dev/tty failed; could not set controlling tty: %s",
		  strerror(errno));
      return FALSE;
    }
  close(fd);
#if defined(HAVE_VHANGUP) && !defined(HAVE_REVOKE)
  signal(SIGHUP, SIG_IGN);
  vhangup();
  signal(SIGHUP, SIG_DFL);
  fd = open(ttyname, O_RDWR);
  if (fd == -1)
    ssh_warning("pty_make_controlling_tty: reopening controlling tty after vhangup failed for %.100s",
		ttyname);
  close(*ttyfd);
  *ttyfd = fd;
#endif /* HAVE_VHANGUP && !HAVE_REVOKE */
  return TRUE;
}
