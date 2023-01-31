#include <malloc.h>
#include <string.h>
#include "linkedlist.h"


int linked_list_add_node(struct linked_list_node* head, void* val, size_t val_length){
    if(head->value == NULL && head->next == NULL){
        head->value = malloc(val_length);
        if (head->value == NULL){
            return -1;
        }
        memcpy(head->value, val, val_length);
        head->value_len = val_length;
    } else if (head->next == NULL){
        head->next = (struct linked_list_node*)malloc(sizeof(struct linked_list_node));
        if (head->next == NULL) {
            return -1;
        }
        memset(head->next, 0, sizeof(struct linked_list_node));
        head->next->value = malloc(val_length);
        memcpy(head->next->value, val, val_length);
        head->next->value_len = val_length;
    } else {
        return linked_list_add_node(head->next, val, val_length);
    }
    return 0;
}

int linked_list_free(struct linked_list_node* head){
    struct linked_list_node* next = head->next;
    free(head->value);
    free(head);
    if (next != NULL){
        linked_list_free(head->next);
    }
    return 0;
}

const void* linked_list_get(struct linked_list_node* head, int index, size_t* out_value_size){
    struct linked_list_node* cur = head;
    for (int i = 0; i <= index; i++){
        if (head->next == NULL){
            return NULL;
        }
        cur = head->next;
    }
    memcpy(out_value_size, &(cur->value_len), sizeof(size_t));
    return head->value;
}

int linked_list_length(const struct linked_list_node* head){
    int count = 0;
    for (const struct linked_list_node* cur = head; cur != NULL; cur = cur->next){
        count += 1;
    }
    if (count == 1 && head->value_len == 0) // if the head is uninitialised
        return 0;
    return count;
}