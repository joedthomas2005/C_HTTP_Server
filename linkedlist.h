#pragma once
#include <stddef.h>

struct linked_list_node{
    struct linked_list_node* next;
    void* value;
    size_t value_len;
};

int linked_list_add_node(struct linked_list_node** head, void* val, size_t val_length);
int linked_list_add_node_int(struct linked_list_node** head, int val);
int linked_list_add_node_str(struct linked_list_node** head, char* str);
const void* linked_list_get(const struct linked_list_node* head, int index, size_t* out_val_length);
int linked_list_free(struct linked_list_node* head);
int linked_list_length(const struct linked_list_node* head);
int linked_list_find(const struct linked_list_node* head, void* target_val, size_t val_length);
