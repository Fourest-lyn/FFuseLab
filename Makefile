top:
	gcc -Wall main.c lib/rbtree.c `pkg-config fuse3 --cflags --libs` -o ffuse

initial:
	mkdir mnt
	./ffuse mnt

stop:
	fusermount -u mnt
	rmdir mnt

end:
	umount mnt
	rmdir mnt

test:
	echo "test">mnt/test.txt