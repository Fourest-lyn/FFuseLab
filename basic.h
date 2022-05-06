#include "lib/rbtree.h"
#include <string.h>
#include <stdlib.h>

#define maxN 1000
struct FFL_node //This is a simple structure
{
    const char *path;
    void *data;
    int leaf_flag;

    struct rb_node node;
};

static struct rb_root Root;

static inline struct FFL_node *newNode(const char *path, void *data, int leaf_flag)
{
    struct FFL_node* node=(struct FFL_node*)malloc(sizeof(struct FFL_node));
    node->path=strdup(path);
    node->data=data;
    node->leaf_flag=leaf_flag;
    return node;
}

static inline int FFL_cmp(const struct FFL_node* node1,const struct FFL_node* node2)
{
    int out=strcmp(node1->path,node2->path);
    if(!out) return node1->leaf_flag-node2->leaf_flag;
    else return out;
}

static inline int strCheck(const char *path)
{
    int len=strlen(path),count=0;
    for(int i=len-1;i>=0;--i)
    {
        if (path[i]=='/') count++;
        if (count==2) return 1;
    }
    return 0;
}

static inline char* strChange(const char *path)
{
    int len=strlen(path),count=0,index[2];
    for(int i=len-1;i>=0;--i)
    {
        if (path[i]=='/') index[count++]=i;
        if (count==2) break;
    }
    char *newPath=(char *)malloc(sizeof(char)*len*2);
    strncat(newPath,path,index[1]);
    strncat(newPath,path+index[0],len-index[0]);
    strncat(newPath,path+index[1],index[0]-index[1]);
    return newPath;
}


/** RedBlackTree part for use
 * reference with site https://www.cnblogs.com/haippy/archive/2012/09/02/2668099.html
 *  
 */

struct FFL_node* __search(struct rb_root *root, const char *path)
{
    struct rb_node *node = root->rb_node;

    while (node) 
    {
        struct FFL_node *data = container_of(node, struct FFL_node, node);
        int result=strcmp(path, data->path);

        if (result < 0) node = node->rb_left;
        else if (result > 0) node = node->rb_right;
        else return data;
    }
    return NULL;
}

int __insert(struct rb_root *root, struct FFL_node *data)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) 
    {
        struct FFL_node *this = container_of(*new, struct FFL_node, node);
        int result = strcmp(data->path, this->path);

        parent = *new;
        if (result < 0) new = &((*new)->rb_left);
        else if (result > 0) new = &((*new)->rb_right);
        else return 0;
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, root);

    return 1;
}

/*
void __free(struct FFL_node *node)
{
    if (node != NULL) 
    {
        if (node->path != NULL) 
        {
            free(node->path);
            node->path = NULL;
        }
        free(node);
        node = NULL;
    }
}
*/
