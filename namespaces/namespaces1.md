# Linux Namespaces

This lesson makes heavy use of the [`unshare`](https://www.man7.org/linux/man-pages/man1/unshare.1.html) command which is part of the [`util-linux` package](https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/).

---

- [List Namespaces](#list-namespaces)
    + [`lsns`](#lsns)
    + [`ps`](#ps)
    + [`ip-netns`](#ip-netns)
    + [`/proc`](#proc)
- [Examples](#examples)
    + [`uts`](#uts)
    + [`pid`](#pid)

---

## List Namespaces

### `lsns`

[`lsns`](https://www.man7.org/linux/man-pages/man8/lsns.8.html)

List the namespaces for the effective user:

```bash
$ lsns
```

List the namespaces for a privileged user:

```bash
$ sudo lsns
```

Use the `--type` option to specify a particular namespace:

```bash
$ lsns -t uts
```

### `ps`

[`ps`](https://www.man7.org/linux/man-pages/man1/ps.1.html)

View process (`pid) namespaces:

```bash
$ ps -o pid,pidns,args
```

### `ip-netns`

[`ip-netns`](https://man7.org/linux/man-pages/man8/ip-netns.8.html)

```bash
$ ip netns list
```

### `/proc`

[`/proc`](https://man7.org/linux/man-pages/man5/proc.5.html)

List a process' namespaces:

```bash
$ sudo ls -l /proc/$$/ns
```

## Examples

### `uts`

```bash
# On host, the following command puts us in a new Bourne-Again shell.
$ sudo unshare --uts bash

# So, now we're inside the "container" process.
$ hostname
kilgore-trout
$ hostname derp
$ hostname
derp
$ exit

# Now, back on host.
$ hostname
kilgore-trout
```

Note that if a command isn't specified, it will default to the value of the `$SHELL` environment variable.

### `pid`

First, we need to download a `rootfs` that we'll then use as a `chroot`:

```bash
$ sudo su -
# mkdir rootfs
# curl http://dl-cdn.alpinelinux.org/alpine/v3.9/releases/x86_64/alpine-minirootfs-3.9.0-x86_64.tar.gz \
| tar -xz -C rootfs/
```

After we create the `rootfs`, let's enter the `chroot`:

```bash
# unshare --pid --fork chroot rootfs sh
/ # ls
bin    dev    etc    home   lib    media  mnt    opt    proc   root   run    sbin   srv    sys    tmp    usr    var
```

Run another process in the `chroot` from which we'll get more information:

```bash
/ # sleep 1000
```

Let's get the `pid` from both the view of host and that of the "container":

Host (in another terminal):

```bash
$ ps -C sleep
    PID TTY          TIME CMD
 115065 pts/4    00:00:00 sleep
```

"Container":

```bash
/ # jobs -p
2
```

They have different numbers, which makes sense because the `sleep` process is running in a different `net` namespace from that of the host.  Also, it's number is much lower, which also makes sense.

So, how can we prove to ourselves that the "container" is truly running in a `chroot`?  Let's look at what it sees as its `root`:

```bash
$ sudo ls -l /proc/115065/root
lrwxrwxrwx 1 root root 0 Apr 21 20:28 /proc/115065/root -> /root/rootfs
```

Yes, just as we instructed it in the commands above, it's view of the world is restricted to `/root/rootfs`.

In contrast, the shell that was invoked on the host sees the host root filesystem as its `root`:

```bash
$ sudo ls -l /proc/$$/root
lrwxrwxrwx 1 btoll btoll 0 Apr 21 20:39 /proc/115820/root -> /
```

However, this is still a problem, which is that we can't use the `ps` command in the "container", which relies on the `proc` pseudo-filesystem for its information:

```bash
/ # ps
PID   USER     TIME  COMMAND
/ # ls -l /proc
total 0
```

We need to mount it:

```bash
/ # mount -t proc proc /proc
```

Now, we can get information about the `sleep` process:

```bash
/ # ls /proc/2
arch_status         cpuset              limits              numa_maps           schedstat           task
attr                cwd                 loginuid            oom_adj             sessionid           timens_offsets
autogroup           environ             map_files           oom_score           setgroups           timers
auxv                exe                 maps                oom_score_adj       smaps               timerslack_ns
cgroup              fd                  mem                 pagemap             smaps_rollup        uid_map
clear_refs          fdinfo              mountinfo           patch_state         stack               wchan
cmdline             gid_map             mounts              personality         stat
comm                io                  mountstats          projid_map          statm
coredump_filter     ksm_merging_pages   net                 root                status
cpu_resctrl_groups  ksm_stat            ns                  sched               syscall
```

Before moving on, we need to clean up the mount point on the host machine.  Even though we mounted the `/proc` filesystem in the "container", since the process is sharing the `mount` namespace of the host, it will appear as an entry in its filesystem table.

Observe (on the host):

```bash
$ mount
...
proc on /root/rootfs/proc type proc (rw,relatime)
```

Simply remove it using the following statement (still on the host):

```bash
$ sudo umount proc
```

## References

- [On Unsharing Namespaces, Part One](https://benjamintoll.com/2022/08/08/on-unsharing-namespaces-part-one/)
- [On Unsharing Namespaces, Part Two](https://benjamintoll.com/2022/12/14/on-unsharing-namespaces-part-two/)
- [`linux-networking` GitHub Repository](https://github.com/btoll/linux-networking)

