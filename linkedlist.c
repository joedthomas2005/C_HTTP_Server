#include <string.h>
#include <malloc.h>
#include "linkedlist.h"
#include "zeroedmalloc.h"

// NOT RECOMMENDED TO MIX TYPES IN THE LIST
// (a 4 char array and an integer are indistiguishable)

int linked_list_add_node(struct linked_list_node** head, void* val, size_t val_length){
    if(*head == NULL){
        *head = (struct linked_list_node*)zeroedmalloc(sizeof(struct linked_list_node));
        void* val_copy = zeroedmalloc(val_length);
        memcpy(val_copy, val, val_length);
        (*head)->value = val_copy;
        (*head)->value_len = val_length;
    } else {
        struct linked_list_node* new = (struct linked_list_node*)zeroedmalloc(sizeof(struct linked_list_node));
        if (new == NULL){
            return -1;
        }
        new->value = zeroedmalloc(val_length);
        if (new->value == NULL){
            return -1;
        }
        memcpy(new->value, val, val_length);
        new->value_len = val_length;
        new->next = *head;
        *head = new;
    }
    return 0;
}

int linked_list_add_node_int(struct linked_list_node** head, int val){
    return linked_list_add_node(head, &val, sizeof(int));
}

int linked_list_add_node_str(struct linked_list_node** head, char* str){
    return linked_list_add_node(head, str, (strlen(str) + 1) * sizeof(char));
}

int linked_list_free(struct linked_list_node* head){
    if(head != NULL){
        struct linked_list_node* next = head->next;
        free(head->value);
        free(head);
        return linked_list_free(next);
    }
    return 0;
}

const void* linked_list_get(const struct linked_list_node* head, int index, size_t* out_value_size){
    const struct linked_list_node* cur = head;
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
    return count;
}

int linked_list_find(const struct linked_list_node* head, void* target_val, size_t val_length){
    int index = 0;
    for(const struct linked_list_node* cur = head; cur != NULL; cur = cur->next){
        if (cur->value_len == val_length){
            if(memcmp(cur->value, target_val, val_length) == 0){
                return index;
            }
        }
        index++;
    }
    return -1;
}
