top:
	gcc -Wall main.c lib/rbtree.c `pkg-config fuse3 --cflags --libs` -o ffuse

initial:
	mkdir mnt
	./ffuse mnt

prepare:
	mkdir mnt/bot1
	mkdir mnt/bot2
	touch mnt/bot1/bot2
	touch mnt/bot2/bot1

stop:
	fusermount -u mnt
	rmdir mnt

end:
	umount mnt
	rmdir mnt

test:
	echo "test">mnt/test.txt