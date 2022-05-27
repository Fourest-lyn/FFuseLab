# FFuse

This is a simple fuse project to implement a communication task between folders.

First, take the commands under to initial the whole C project:

```
make
make initial
```

Then choose one of the following commands to initial the bot:

```
make prepare
./initial_bot.sh
```

After this, we can get two folders `bot1` and `bot2` , the records can be find here.

If you want to send message with the identity of `bot1` , use this command:

```
echo "The things you wants to say put here." >> mnt/bot1/bot2
```

Then `bot2` can find the message from `bot1` using command:

```
cat mnt/bot2/bot1
```

If you need more users, you need to set up their folders and record address by yourself.
