/** FFuseLab
 * 
 * This file is started with fuselib/example/happy.c
 * 
 * Compile this file with command:
 *      gcc -Wall main.c lib/rbtree.c `pkg-config fuse3 --cflags --libs` -o ffuse
*/

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include "basic.h"

#define BLOCKSIZE (1024UL * 4)

static struct options 
{
	const char *filename;
	const char *contents;
	int show_help;
}   options;

#define OPTION(t, p) { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = 
{
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *FFL_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 0;
	return NULL;
}

static int FFL_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    addLog("getattr",path);
	(void) fi;
	memset(stbuf, 0, sizeof(struct stat));
    struct FFL_node *node=__search(&Root,path);

	if (node==NULL) return -ENOENT;
    if (!node->leaf_flag)
    {
        stbuf->st_mode=__S_IFDIR | 0755;
        stbuf->st_nlink=2;
    }
    else 
    {
        stbuf->st_mode=__S_IFREG | 0444;
        stbuf->st_nlink=1;
        stbuf->st_size=strlen(node->data);
    }
	return 0;
}

static int FFL_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    (void) flags;
    (void) offset;
    (void) fi;
    addLog("readdir",path);
	filler(buf, ".", NULL, 0, 0);
    if(strcmp(path, "/") != 0) filler(buf, "..", NULL, 0, 0);
	struct FFL_node* node=__search(&Root,path);
    if(node==NULL) return -ENOENT;
    if(node->leaf_flag) return -ENOTDIR;

    struct rb_node *nodeN=NULL; //node_next
    for(nodeN=rb_next(&node->node);nodeN;nodeN=rb_next(nodeN))
    {
        const struct FFL_node *nodeT=rb_entry(nodeN,struct FFL_node,node);
        const char* tail=getTail(path,nodeT->path);
        if(tail==NULL) break;
        if(strchr(tail+1,'/')>0) continue;
        filler(buf,tail+1,NULL,0,0);
    }
	return 0;
}

static int FFL_open(const char *path, struct fuse_file_info *fi)
{
    addLog("open",path);
    struct FFL_node *node=__search(&Root,path);
    if(node==NULL) 
    {
        if((fi->flags & O_ACCMODE) == O_RDONLY || !(fi->flags & O_CREAT)) return -EPERM;
        else
        {
            node=newNode(path,EMPTY_FILE,1);
            __insert(&Root,node);
        }
    }
    else if(!node->leaf_flag) return -EISDIR;
    fi->fh=(unsigned long)node;
	return 0;
}

static int FFL_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    //len:      The true length of the content
    //size:     The output length of the content
    //offset:   The start point of the text
    //buf:      The output text

    addLog("read",path);
    struct FFL_node* node=__search(&Root,path);
    if(node==NULL) return -ENOENT;
    assert(node == (struct FFL_node*)fi->fh);

    size_t len=strlen(node->data);
    if(len<offset) return 0;
    size = (offset+size>len) ? len-offset : size;
    memcpy(buf,node->data+offset,size);
	return size;
}

static int FFL_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) 
{
    addLog("write",path);
    //common write part
    // struct FFL_node*  node=__search(&Root,path);
    struct FFL_node* node=(struct FFL_node*)fi->fh;
    if(node==NULL) return -EEXIST;
    // assert(node == (struct FFL_node*)fi->fh);
    char* end=malloc(100);
    sprintf(end,"size=%ld, offset=%ld, buf=%s, data len=%ld",size,offset,buf,strlen(node->data));
    addLog(end,path);
    if(strlen(node->data)<offset+size) 
    {
        // void *newp=realloc(node->data,offset+size+1);
        void *newp=(void*)malloc((offset+size+1)*sizeof(void));
        memcpy(newp,node->data,strlen(node->data));
        node->data=newp;
    }
    memcpy(node->data+offset,buf,size);

    //echo exchange-chat part
    if(!strCheck(path)) return size;
    char *newPath=strChange(path);
    struct FFL_node* exNode=__search(&Root,newPath);
    if(exNode==NULL) return -EEXIST;
    if(strlen(exNode->data)<offset+size) 
    {
        // exNode->data=realloc(exNode->data,offset+size+1);
        void *newp=(void*)malloc((offset+size+1)*sizeof(void));
        memcpy(newp,exNode->data,strlen(exNode->data));
        exNode->data=newp;
    }
    memcpy(exNode->data+offset,buf,size);

    return size;
}

static int FFL_mknod(const char *path, mode_t mode, dev_t dev) 
{
    (void) mode;
    (void) dev;
    addLog("mknod",path);
    struct FFL_node* node=newNode(path,EMPTY_FILE,true);
    if(!__insert(&Root,node)) 
    {
        freeNode(node);
        return -EEXIST;
    }
    return 0;
}

static int FFL_mkdir(const char *path, mode_t mode) 
{
    (void) mode;
    addLog("mkdir",path);
    struct FFL_node* node=newNode(path,NULL,0);
    if(!__insert(&Root,node)) 
    {
        freeNode(node);
        return -EEXIST;
    }
    return 0;
}

static int FFL_access(const char *path, int mask)
{
    addLog("access",path);
    struct FFL_node *node=__search(&Root,path);
    if(!node) return -ENOENT;
    return 0;
}

static int FFL_utimens(const char *path, const struct timespec tv[2],struct fuse_file_info *fi)
{
    (void) tv;
    (void) fi;
    addLog("utimens",path);
    return 0;
}

static int FFL_release(const char *path, struct fuse_file_info *fi) {addLog("release",path); return 0;}

static const struct fuse_operations FFL_op = 
{
	.init       = FFL_init,
	.getattr	= FFL_getattr,
	.readdir	= FFL_readdir,
	.open		= FFL_open,
	.read		= FFL_read,
    .write      = FFL_write,
    .mknod      = FFL_mknod,
    .mkdir      = FFL_mkdir,
    .access     = FFL_access,
    .utimens    = FFL_utimens,
    .release    = FFL_release,
};

static void show_help(const char *progname)
{
    printf("This is a Fuse lab created by Fourest.\n");
	printf("usage: %s [options] <mountpoint>\n\n", progname);
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) return 1;

	if (options.show_help) 
    {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

    __insert(&Root,newNode("/",NULL,0));

    //debug
    struct FFL_node* logNode=newNode("/log",Log,1);
    __insert(&Root,logNode);

	ret = fuse_main(args.argc, args.argv, &FFL_op, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
