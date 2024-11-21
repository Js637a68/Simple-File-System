# Simple-File-System
cse-30341-fa19-project06，一个简短的文件系统

测试：功能一样，读写磁盘块次数差了一点
```shell
➜  cse-30341-fa19-project06 git:(master) make test-shell         
Compiling src/library/fs.o
Linking   lib/libsfs.a
Linking   bin/sfssh
Testing debug on data/image.5 ... Success
Testing debug on data/image.20 ... Success
Testing debug on data/image.200 ... Success
Testing format on data/image.5.formatted ... Success
Testing format on data/image.20.formatted ... Success
Testing format on data/image.200.formatted ... Success
Testing mount on data/image.5 ... Success
Testing mount-mount on data/image.5 ... Success
Testing mount-format on data/image.5 ... Success
Testing bad-mount on /tmp/tmp.Y2bJawwsyD/image.5 ... Success
Testing bad-mount on /tmp/tmp.Y2bJawwsyD/image.5 ... Success
Testing bad-mount on /tmp/tmp.Y2bJawwsyD/image.5 ... Success
Testing bad-mount on /tmp/tmp.Y2bJawwsyD/image.5 ... Success
Testing bad-mount on /tmp/tmp.Y2bJawwsyD/image.5 ... Success
Testing stat on data/image.5 ... Success
Testing stat on data/image.20 ... Success
Testing stat on data/image.200 ... Success
Testing create in data/image.5.create ... Success
Testing copyout in data/image.5 ... Success
Testing copyout in data/image.20 ... Success
Testing copyout in data/image.200 ... Success
Testing cat on data/image.5 ... Success
Testing cat on data/image.20 ... Success
Testing copyin in /tmp/tmp.nViaTiOYqb/image.5 ... Success
Testing copyin in /tmp/tmp.nViaTiOYqb/image.20 ... Success
Testing copyin in /tmp/tmp.nViaTiOYqb/image.200 ... Success
Testing remove in /tmp/tmp.w5C6gcZaD5/image.5 ... Success
Testing remove in /tmp/tmp.w5C6gcZaD5/image.5 ... False
--- /dev/fd/63  2024-11-22 01:52:03.786442118 +0800
+++ /dev/fd/62  2024-11-22 01:52:03.786442118 +0800
@@ -54,5 +54,5 @@
 Inode 2:
     size: 965 bytes
     direct blocks: 4
-30 disk block reads
-13 disk block writes
+27 disk block reads
+10 disk block writes
Testing remove in /tmp/tmp.w5C6gcZaD5/image.20 ... False
--- /dev/fd/63  2024-11-22 01:52:03.794442126 +0800
+++ /dev/fd/62  2024-11-22 01:52:03.794442126 +0800
@@ -42,4 +42,4 @@
     indirect block: 9
     indirect data blocks: 13 14
 41 disk block reads
-19 disk block writes
+18 disk block writes
Testing valgrind on /tmp/tmp.2VWv1YmO70/image.200 ... Success
make: *** [Makefile:58: test-shell] Error 2
```
