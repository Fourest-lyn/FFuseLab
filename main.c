/** FFuseLab
 * 
 * This file is started with fuselib/example/happy.c
 * 
 * Compile this file with command:
 *      gcc -Wall main.c `pkg-config fuse3 --cflags --libs` -o ffuse
*/


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

static struct options 
{
	const char *filename;
	const char *contents;
	int show_help;
}   options;

#define OPTION(t, p) { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = 
{
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *FFL_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

//This function is not important for our project now, I guess.
static int FFL_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) //The path is empty
    {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} 
    else if (strcmp(path+1, options.filename) == 0) //The path is the same with the file
    {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} 
    else res = -ENOENT;

	return res;
}

static int FFL_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

    //The path is not empty, return error code[2]: No such file or directory
	if (strcmp(path, "/") != 0) return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, options.filename, NULL, 0, 0);

	return 0;
}

static int FFL_open(const char *path, struct fuse_file_info *fi)
{
    //Check if exist such file, if not, return error code[2]: No such file or directory
	if (strcmp(path+1, options.filename) != 0) return -ENOENT;

    //If flag us not read-only, return error code[13]: Permission denied
	if ((fi->flags & O_ACCMODE) != O_RDONLY) return -EACCES;

	return 0;
}

//This function should be banned
static int FFL_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    //len:      The true length of the content
    //size:     The output length of the content
    //offset:   The start point of the text
    //buf:      The output text
	size_t len;
	(void) fi;
    //Check if exist such file, if not, return error code[2]: No such file or directory
	if(strcmp(path+1, options.filename) != 0) return -ENOENT;

	len = strlen(options.contents);
	if (offset < len) 
    {
		if (offset + size > len) size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} 
    else size = 0;

	return size;
}

static int FFL_mkdir(const char *path, mode_t mode) {return 0;}

static int FFL_mknod(const char *path, mode_t mode, dev_t dev) {return 0;}

static int FFL_unlink(const char *path) {return 0;}

static const struct fuse_operations FFL_op = 
{
	.init       = FFL_init,
	.getattr	= FFL_getattr,
	.readdir	= FFL_readdir,
	.open		= FFL_open,
	.read		= FFL_read,
    .mkdir      = FFL_mkdir,
    .mknod      = FFL_mknod,
    .unlink     = FFL_unlink,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"FFuseLab\" file\n"
	       "                        (default: \"FFuseLab\")\n"
	       "    --contents=<s>      Contents \"FFuseLab\" file\n"
	       "                        (default \"This is Fourest's Fuse Lab!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    //Initial the filename of the project. However, our project don't need this
	options.filename = strdup("FFuseLab");
	options.contents = strdup("This is Fourest's Fuse Lab!\n");

	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) return 1;

	if (options.show_help) 
    {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0); //This line is the help represent api with libfuse.
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &FFL_op, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
