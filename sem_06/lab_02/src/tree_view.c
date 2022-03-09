#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    char *pathname;
    int inode;
    int par_inode;
    struct node *next;
} node_t;

void free_node(node_t *n)
{
    if (n->pathname)
        free(n->pathname);
    free(n);
}

void free_stack(node_t **stack)
{
    while (*stack)
    {
        node_t *old_h = *stack;
        *stack = (*stack)->next;
        free_node(old_h);
    }
}

char * string_alloc(char *orig)
{
    char * st = malloc(strlen(orig) + 2);
    if (!st)
    {
        printf("Error malloc\n");
    }
    char * tmp = strcpy(st, orig);
    if (tmp)
    {
        st = tmp;
    }
    else
    {
        free(st);
        st = NULL;
    }
    return st;
}

node_t *push(node_t *stack, node_t *elem)
{
    elem->next = stack;
    return elem;
}

node_t *create_node(char *path, int inode, int par_inode)
{
    node_t *elem = malloc(sizeof(node_t));
    if (!elem)
    {
        printf("Error malloc\n");
    }
    else
    {
        elem->pathname = string_alloc(path);
        if (elem->pathname)
        {
            elem->inode = inode;
            elem->par_inode = par_inode;
        }
        else
        {
            free(elem);
            elem = NULL;
        }
    }
    return elem;
}

node_t * pop(node_t **stack)
{
    node_t * data = NULL;
    if (*stack)
    {
        data = (*stack);
        *stack = (*stack)->next;
    }
    return data;
}

int dopath(char *fullpath);
void print_filename(const char *pathname, int is_dir, ino_t inode, int level);

int main(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
    {
        printf("Использование: ftw <начальный каталог>");
        exit(1);
    }
    
    ret = dopath(argv[1]);

    return ret;
}

void analyse_dir(node_t ** stack, node_t *cur, int *cur_dir, int *level)
{
    struct dirent *dirp;
    DIR *dp;
    if (chdir(cur->pathname) < 0)
    {
        printf("Ошибка вызова chdir\n");
        free_node(cur);
        free_stack(stack);
        exit(1);
    }
    if ((dp = opendir(".")) == NULL)
    {
        printf("закрыт доступ к каталогу %s", cur->pathname);
        free_node(cur);
        free_stack(stack);
        exit(1);
    }
    else
    {
        print_filename(cur->pathname, 1, cur->inode, *level);
        (*cur_dir) = cur->inode;
        (*level)++;
    }

    dirp = readdir(dp);

    (*stack) = push(*stack, cur);   
    while (dirp != NULL)
    {

        if (!(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
        {
            node_t *new_n = create_node(dirp->d_name, dirp->d_ino, cur->inode);
            if (!new_n)
            {
                free_stack(stack);
                exit(1);
            }
            (*stack) = push((*stack), new_n);
        }
        dirp = readdir(dp);
    }
    if (closedir(dp) < 0)
    {
        printf("невозможно закрыть каталог %s", cur->pathname);
        free_node(cur);
        free_stack(stack);
        exit(1);
    }
    else
        dp = NULL;
}

int dopath(char *fullpath)
{
    node_t *stack = NULL;
    int ret = 0;
    struct stat statbuf;
    if (lstat(fullpath, &statbuf) < 0)
    {
        printf("ошибка вызова функции stat для %s", fullpath);
        exit(1);
    }
    node_t *elem = create_node(fullpath, statbuf.st_ino, statbuf.st_ino);
    if (!elem)
        exit(1);
    stack = push(stack, elem);
    int cur_dir = 0;
    int level = 0;
    while (stack)
    {
        node_t *cur = pop(&stack); 
        if (cur_dir != cur->inode)
        {
            if (lstat(cur->pathname, &statbuf) < 0)
            {
                printf("ошибка вызова функции stat для %s", cur->pathname);
                free_node(cur);
                free_stack(&stack);
                exit(1);
            }
            else if (S_ISDIR(statbuf.st_mode) == 0)
            {
                print_filename(cur->pathname, 0, statbuf.st_ino, level);
                free_node(cur);
            }
            else
            {
                analyse_dir(&stack, cur, &cur_dir, &level);
            }
        }
        else
        {
            level--;
            if (chdir("..") < 0)
            {
                free_node(cur);
                printf("Ошибка вызова chdir\n");
                free_stack(&stack);
                exit(1);
            }
            cur_dir = cur->par_inode;
            free_node(cur);
        }
    }
    if (stack)
    {
        free_stack(&stack);
    }
    return (ret);
}

void print_filename(const char *pathname, int is_dir, ino_t inode, int level)
{
    int len = strlen(pathname);
    int name_start = 0;
    int slashes_cnt = 0;
    int exists_end_slash = pathname[strlen(pathname) - 1] == '/' ? 1 : 0;
    for (int i = 0; i < level; i++)
    {
        printf("--|");
    }
    printf("-->%s", &pathname[name_start]);
    if (is_dir && !exists_end_slash)
        printf("/");
    printf("    %ld\n", inode);
}