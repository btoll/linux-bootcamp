# Environment Variables

- [Examples](#examples)
    + [Exporting](#exporting)
    + [Listing](#listing)
    + [Sourcing](#sourcing)
- [Chroot](#chroot)

---

## Examples

### Exporting

Export an environment variable.  This will make it available to not only the current process but also any child processes.  That is, a child will inherit its parent's environment.

```bash
$ export FOO=foo
```

It's important to note, though, that the child process cannot change any values inherited from a parent.  In fact, no process can alter the environment of any other process.

Of course, you can set a variable for the current session without using the `export` function, but it will not be inherited by any child processes.

Observe:

```bash
$ kilgore=trout
$ echo $kilgore
trout
```

Now, create a child `bash` process and try to print the variable:

```bash
$ bash
$ echo $kilgore
$
```

The `kilgore` variable isn't in the environment.  If we now exit the child process, we can "see" the variable again (back in the parent process, where it was defined):

```bash
$ exit
exit
$ echo $kilgore
trout
```

> A very useful built-in environment variable is `SHLVL`.  This can be used to discern how many nested levels the current shell is in.
> ```bash
> $ echo $SHLVL
> 2
> $ bash
> $ echo $SHLVL
> 3
> $ bash
> $ echo $SHLVL
> 4
> $ exit
> exit
> $ echo $SHLVL
> 3
> $ exit
> exit
> $ echo $SHLVL
> 2
> ```

### Listing

There are several commands to view all of the environment variables available to the shell.  Of course, there are subtle differences among them, but I'm not going to cover them here.

- [`printenv`]
- [`export`]
- [`env`]

<!--These are `bash` built-ins, meaning that the functions are part of the `bash` program and are not an external process.  This-->

If you want to print the value of an individual environment variable, do either of the following:

```bash
$ echo $SHELL
/bin/bash
$ printenv SHELL
/bin/bash
```

Note that the latter does not take a dollar sign (`$`) and will fail to look up the variable if it is included.

### Sourcing

Sourcing a bash script will not create a subprocess. This means that all of the commands will execute within the context (i.e,. environment) of the current shell.

Why is this useful? Well, this method is used a lot when creating environment variables that are needed for a particular session. By sourcing the file instead of executing it, when the file is done being read the variables are still part of the session, whereas they wouldn’t be if executing the file (recall that a child cannot modify a parent’s environment).

## Chroot

If you want to play in a sandboxed environment, create a [`chroot`].

An easy way to create one is to clone the [`chroot` project] on GitHub.  This will create an environment for you where you can safely download and install software in isolation from the rest of the system.

```bash
$ git clone git@github.com:btoll/chroot.git
```

The following will create a 32-bit Debian `bookworm` `chroot` in `/srv/linux-boot-camp/envvars/chroot`.

```bash
sudo ./install.sh \
--chroot stacksmashing \
--dir /home/btoll/projects/linux-boot-camp/envvars/chroot \
--type directory \
--profile minimal \
--user btoll \
--group btoll \
--release bookworm \
--32 \
--dry-run

[./install.sh][INFO] Installing debootstrap and schroot, if missing.
[stacksmashing]
description=Debian (bookworm)
type=directory
directory=/srv/linux-boot-camp/envvars/chroot
personality=linux32
profile=minimal
users=btoll
root-users=btoll
groups=btoll
root-groups=btoll
```

This was a `dry run`, which means that it will show you what it will do without actually installing it.  After removing that flag and running it again, we can see the install begin.  At the end, we'll get a message similar to the following:

```bash
[./install.sh][SUCCESS] Chroot installed in /srv/linux-boot-camp/envvars/chroot!
[./install.sh][INFO] You can now enter the chroot by issuing the following command:

        schroot -u btoll -c stacksmashing -d /

Have fun! Weeeeeeeeeeeee
```

As the log says, we can now enter the `chroot` isolated environment by issuing the [`schroot`] command:

```bash
$ schroot -u btoll -c stacksmashing -d /
```

We'll now see the prompt change, which indicates the command was successful:

```bash
(stacksmashing)root@kilgore-trout:/#
```

You can tell it's a 32-bit install through the [`uname`] command:

```bash
(stacksmashing)root@kilgore-trout:/# uname -m
i686
```

> To create a 64-bit `chroot` sandbox, remove the `--32` switch in the above command.

A 64-bit installation would report the following:

```bash
(stacksmashing)root@kilgore-trout:/# uname -m
x86_64
```

To update the packages from the Debian repositories and install any software, you must enter the `chroot` as `root`:

```bash
$ schroot -u root -c stacksmashing -d /
$ apt-get update
$ apt-get install build-essential gdb vim -y
```

Let's look at [a small program that will print all the environment variables] of a process:

```c
#include <stdio.h>

int main(int argc, char *argv[])
{
  int j;
  printf("Argument vector:\n");
  for (j = 0; ; j++) {
    if (argv[j] == NULL) break;
    printf("%2d: %p %s\n", j, argv[j], argv[j]);
  }
  printf("Environment vector:\n");
  for (j++; ; j++) {
    if (argv[j] == NULL) break;
    printf("%2d: %p %s\n", j, argv[j], argv[j]);
  }
  return 0;
}
```

We'll compile it with the most verbose debugger symbol option ([`-ggdb3`]):

```bash
$ gcc -ggdb3 a.out
$ gdb a.out
```

```gdb
(gdb) b 5
(gdb) r foobar
(gdb) show environment
SHELL=/bin/bash
SCHROOT_CHROOT_NAME=stacksmashing
SCHROOT_COMMAND=-bash
SCHROOT_SESSION_ID=stacksmashing-798cbe73-7014-44d1-8609-e38bf53e5156
PWD=/foo
LOGNAME=root
SCHROOT_ALIAS_NAME=stacksmashing
SCHROOT_GROUP=btoll
SCHROOT_USER=btoll
HOME=/root
TERM=screen-256color
USER=root
SHLVL=1
SCHROOT_GID=1000
SCHROOT_UID=1000
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
_=/usr/bin/gdb
OLDPWD=/
LINES=32
COLUMNS=137
(gdb) p (char *) getenv("USER")
$7 = 0x7fffffffef73 "root"
(gdb) p (char *) getenv("SCHROOT_CHROOT_NAME")
$8 = 0x7fffffffee62 "stacksmashing"
```

Or, just print the value for a single environment variable:

```gdb
(gdb) show env TERM
TERM = screen-256color
```

Here's another way to get the environment.  First, get the PID:

```gdb
(gdb) info proc
process 471165
cmdline = '/foo/a.out foobar'
cwd = '/foo'
exe = '/foo/a.out'
```

Then, print all the variables:

```gdb
(gdb) shell xargs -0 printf %s\\n < /proc/471165/environ
SHELL=/bin/bash
SCHROOT_CHROOT_NAME=stacksmashing
SCHROOT_COMMAND=-bash
SCHROOT_SESSION_ID=stacksmashing-798cbe73-7014-44d1-8609-e38bf53e5156
PWD=/foo
LOGNAME=root
_=/usr/bin/gdb
SCHROOT_ALIAS_NAME=stacksmashing
SCHROOT_GROUP=btoll
SCHROOT_USER=btoll
LINES=32
HOME=/root
COLUMNS=137
TERM=screen-256color
USER=root
SHLVL=1
SCHROOT_GID=1000
SCHROOT_UID=1000
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
OLDPWD=/
```

> [`show environment`] shows the `gdb` environment, **not** the environment of the program being debugged.

## References

- [Shells and Shell Scripting](https://benjamintoll.com/2023/01/22/on-the-lpic-1-exam-102-shells-and-shell-scripting/)
- [Bash Builtin Commands](https://www.gnu.org/software/bash/manual/html_node/Bash-Builtins.html)
- [On Stack Smashing, Part One](https://benjamintoll.com/2019/04/09/on-stack-smashing-part-one/)
- [On Stack Smashing, Part Two](https://benjamintoll.com/2019/04/10/on-stack-smashing-part-two/)

[`chroot`]: https://wiki.archlinux.org/title/Chroot
[`chroot` project]: https://github.com/btoll/chroot
[`uname`]: https://www.man7.org/linux/man-pages/man1/uname.1.html
[`schroot`]: https://man.archlinux.org/man/schroot.1
[`show environment`]: https://getdocs.org/Gdb/docs/latest/gdb/Environment
[a small program that will print all the environment variables]: https://stackoverflow.com/a/71981983
[`-ggdb3`]: http://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
[`printenv`]: https://www.man7.org/linux/man-pages/man1/printenv.1.html
[`export`]: https://www.man7.org/linux/man-pages/man1/export.1p.html
[`env`]: https://man7.org/linux/man-pages/man1/env.1.html

