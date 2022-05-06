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
#include "basic.h"

static struct options 
{
	const char *filename;
	const char *contents;
	int show_help;
}   options;

#define OPTION(t, p) { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = 
{
	// OPTION("--name=%s", filename),
	// OPTION("--contents=%s", contents),
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

static int FFL_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	(void) fi;
	memset(stbuf, 0, sizeof(struct stat));
    struct FFL_node *node=__search(&Root,path);

	if (!node) return -ENOENT;
    if (node->leaf_flag)
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

static inline const char* getTail(const char *parent,const char *son)
{
    if(*(parent+1)=='\0' && *parent=='/' && *son=='/') return son;
    while(*parent!='\0' && *son!='\0' && *parent==*son) ++parent,++son;
    if(*parent=='\0' && *son=='/') return son;
    return NULL;
}


static int FFL_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    int ret=0;

	filler(buf, ".", NULL, 0, 0);
    if(strcmp(path, "/") != 0) filler(buf, "..", NULL, 0, 0);
	struct FFL_node* node=__search(&Root,path);
    if(!node) ret=-ENOENT;
    if(node->leaf_flag) ret=-ENOTDIR;

    struct rb_node *nodeN=NULL; //node_next
    for(nodeN=rb_next(&node->node);nodeN!=NULL;nodeN=rb_next(nodeN))
    {
        const struct FFL_node *nodeT=rb_entry(nodeN,struct FFL_node, node);
        const char* tail=getTail(path,nodeT->path);
        if(!tail) break;
        if(strchr(tail+1,'/')>0) continue;
        filler(buf,tail+1,NULL,0,0);
    }

	return ret;
}

static int FFL_open(const char *path, struct fuse_file_info *fi)
{
    int ret=0;
    struct FFL_node *node=__search(&Root,path);
    if(!node) 
    {
        if((fi->flags & O_ACCMODE) == O_RDONLY || !(fi->flags & O_CREAT)) ret=-ENOENT;
        else
        {
            node=newNode(path,NULL,1);
            __insert(&Root,node);
        }
    }
    else if(!node->leaf_flag) ret=-EISDIR;
    fi->fh=(unsigned long)node;
	return ret;
}

//This function should be banned
static int FFL_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    //len:      The true length of the content
    //size:     The output length of the content
    //offset:   The start point of the text
    //buf:      The output text

    struct FFL_node* node=__search(&Root,path);
    if(!node) return -ENOENT;
    assert(node == (struct FFL_node*)fi->fh);

    size_t len=strlen(node->data);
    if(len<=offset) return 0;
    size = (offset+size>len) ? len-offset : size;
    memcpy(buf,node->data+offset,size);
	return size;
}

static int FFL_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) 
{
    //common write part
    // node=(struct FFL_node*) fi->fh;
    struct FFL_node*  node=__search(&Root,path);
    if(!node) return -EEXIST;
    assert(node == (struct FFL_node*)fi->fh);
    if(strlen(node->data)<offset+size) node->data=realloc(node->data,2*(offset+size));
    memcpy(node->data+offset,buf,size);

    //echo exchange-chat part
    if(!strCheck(path)) return 0;
    char *newPath=strChange(path);
    struct FFL_node* exNode=__search(&Root,newPath);
    if(!exNode) return -EEXIST;
    if(strlen(exNode->data)<offset+size) exNode->data=realloc(exNode->data,2*(offset+size));
    memcpy(exNode->data+offset,buf,size);

    return 0;
}

static int FFL_release(const char *path, struct fuse_file_info *fi) {return 0;}

static int FFL_mkdir(const char *path, mode_t mode) 
{
    struct FFL_node* node=newNode(path,NULL,0);
    int ins=__insert(&Root,node);
    if(!ins) return -EEXIST;
    return 0;
}

static int FFL_mknod(const char *path, mode_t mode, dev_t dev) 
{
    struct FFL_node* node=newNode(path,NULL,1);
    int ins=__insert(&Root,node);
    if(!ins) return -EEXIST;
    return 0;
}

static const struct fuse_operations FFL_op = 
{
	.init       = FFL_init,
	.getattr	= FFL_getattr,
	.readdir	= FFL_readdir,
	.open		= FFL_open,
	.read		= FFL_read,
    .mknod      = FFL_mknod,
    .write      = FFL_write,
    .release    = FFL_release,
    .mkdir      = FFL_mkdir,
};

static void show_help(const char *progname)
{
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
		assert(fuse_opt_add_arg(&args, "--help") == 0); //This line is the help represent api with libfuse.
		args.argv[0][0] = '\0';
	}

    __insert(&Root,newNode("/",NULL,0));
	ret = fuse_main(args.argc, args.argv, &FFL_op, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
