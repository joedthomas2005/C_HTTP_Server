#pragma once
#include <stddef.h>

struct treenode {
    struct treenode* left;
    struct treenode* right;
    char key[129]; // 128 chars + \0
    void* value;
    size_t value_len;
};

int searchtree_add_entry(struct treenode* root, const char* key, void* value, size_t value_len);
void* searchtree_lookup(struct treenode* root, const char* key, size_t* out_value_len);
void searchtree_free(struct treenode* root);
void searchtree_for_each(struct treenode* root, void (*callback)(const char*, const void*, size_t));


