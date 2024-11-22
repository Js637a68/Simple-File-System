# Simple-File-System
cse-30341-fa19-project06，一个简短的文件系统

测试：功能一样，在write读写磁盘块次数不一样是因为我这里没有磁盘块可分配时直接goto最后保存inode，没有过多的错误处理
```shell
Testing debug on data/image.5 ... Success
Testing debug on data/image.20 ... Success
Testing debug on data/image.200 ... Success
Testing format on data/image.5.formatted ... Success
Testing format on data/image.20.formatted ... Success
Testing format on data/image.200.formatted ... Success
Testing mount on data/image.5 ... Success
Testing mount-mount on data/image.5 ... Success
Testing mount-format on data/image.5 ... Success
Testing bad-mount on /tmp/tmp.uLHisvrJis/image.5 ... Success
Testing bad-mount on /tmp/tmp.uLHisvrJis/image.5 ... Success
Testing bad-mount on /tmp/tmp.uLHisvrJis/image.5 ... Success
Testing bad-mount on /tmp/tmp.uLHisvrJis/image.5 ... Success
Testing bad-mount on /tmp/tmp.uLHisvrJis/image.5 ... Success
Testing stat on data/image.5 ... Success
Testing stat on data/image.20 ... Success
Testing stat on data/image.200 ... Success
Testing create in data/image.5.create ... Success
Testing copyout in data/image.5 ... Success
Testing copyout in data/image.20 ... Success
Testing copyout in data/image.200 ... Success
Testing cat on data/image.5 ... Success
Testing cat on data/image.20 ... Success
Testing copyin in /tmp/tmp.Q4V7PdO2Tx/image.5 ... Success
Testing copyin in /tmp/tmp.Q4V7PdO2Tx/image.20 ... Success
Testing copyin in /tmp/tmp.Q4V7PdO2Tx/image.200 ... Success
Testing remove in /tmp/tmp.anUDVY9uPn/image.5 ... Success
Testing remove in /tmp/tmp.anUDVY9uPn/image.5 ... Success
Testing remove in /tmp/tmp.anUDVY9uPn/image.20 ... False
--- /dev/fd/63  2024-11-22 15:47:20.126802173 +0800
+++ /dev/fd/62  2024-11-22 15:47:20.126802173 +0800
@@ -41,5 +41,5 @@
     direct blocks: 4 5 6 7 8
     indirect block: 9
     indirect data blocks: 13 14
-33 disk block reads
-11 disk block writes
+41 disk block reads
+18 disk block writes
Testing valgrind on /tmp/tmp.1Cz6sagSxI/image.200 ... Success
```
