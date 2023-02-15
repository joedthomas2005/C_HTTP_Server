#pragma once
#include <stddef.h>
#include "linkedlist.h"

struct searchtree_node {
    char key[129]; // 128 chars + \0
    void* value;
    size_t value_len;
    struct searchtree_node* left;
    struct searchtree_node* right;
};

struct searchtree_key_value_pair {
    char key[129];
    void* value;
    size_t value_len;
};

int searchtree_add_entry(struct searchtree_node** root, const char* key, void* value, size_t value_len);
void* searchtree_lookup(struct searchtree_node* root, const char* key, size_t* out_value_len);
void searchtree_free(struct searchtree_node* root);
void searchtree_for_each(const struct searchtree_node* root, void (*callback)(const char*, const void*, size_t));
int searchtree_get_keys(const struct searchtree_node* root, struct linked_list_node** out);
int searchtree_get_entries(const struct searchtree_node* root, struct linked_list_node** out);
int searchtree_count(const struct searchtree_node* root);
unsigned char searchtree_contains(const struct searchtree_node* root, const char* key);
