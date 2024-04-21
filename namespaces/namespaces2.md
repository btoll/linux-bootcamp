# Linux Namespaces

This lesson makes heavy use of the [`unshare`](https://www.man7.org/linux/man-pages/man1/unshare.1.html) command which is part of the [`util-linux` package](https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/).

---

- [Examples](#examples)
   + [`mount`](#mount)
    + [`net`](#net)
    + [`user`](#user)

---

## Examples

### `mount`

```bash
$ sudo unshare --mount
# mkdir source target
# touch source/HELLO
# mount --bind source target
# ls target
HELLO
```

Unfortunately, the "container" can still list all of the host's mount points.  Why is that?

The `mount` command gets its information from `/proc`, and we're sharing the host's `pid` namespace.  To remedy this, we'd need to create the "container" by unsharing the `pid` namespace.

```bash
$ sudo unshare --pid --fork --mount chroot rootfs bash
```

> Note that running `mount` on the host will **not** list the bind mount we just created in the container because we are not sharing the `mount` namespace.

### `net`

```bash
$ sudo unshare --net bash
```

### `user`

```bash
$ unshare --user
$ id
uid=65534(nobody) gid=65534(nogroup) groups=65534(nogroup)
```

> Depending on the value of `PS1`, your prompt may reference the `nobody` user.

On the host, run the following:

```bash
$ sudo echo "0 1000 1" >> /proc/2713100/uid_map
```

Then, in the "container", run the `id` command again:

```bash
$ id
uid=0(root) gid=65534(nogroup) groups=65534(nogroup)
```

Oh, snap!

Here's an easier way to do it:

```bash
$ unshare --map-root-user bash
# id
uid=0(root) gid=0(root) groups=0(root),65534(nogroup)
# cat /proc/$$/uid_map
         0       1000
```

## References

- [On Unsharing Namespaces, Part One](https://benjamintoll.com/2022/08/08/on-unsharing-namespaces-part-one/)
- [On Unsharing Namespaces, Part Two](https://benjamintoll.com/2022/12/14/on-unsharing-namespaces-part-two/)
- [`linux-networking` GitHub Repository](https://github.com/btoll/linux-networking)

