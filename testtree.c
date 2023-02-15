#include "linkedlist.h"
#include "searchtree.h"
#include <stdio.h>
#include <stddef.h>

int main(int argc, char** argv){
    struct searchtree_node* tree_root = NULL;
    int val = 5;
    searchtree_add_entry(&tree_root, "Key1", &val, sizeof(val));
    val = 10;
    searchtree_add_entry(&tree_root, "Key2", &val, sizeof(val));
    searchtree_add_entry(&tree_root, "Another Key", "This is a value", sizeof(char) * 16);
    struct linked_list_node* keys = NULL;
    searchtree_get_keys(tree_root, &keys);
    for(const struct linked_list_node* cur = keys; cur != NULL; cur = cur->next){
        size_t valsize = 0;
        void* val = searchtree_lookup(tree_root, (char*)cur->value, &valsize);
        if(valsize == sizeof(int)){
            printf("%s: %i\n", (char*)cur->value, *(int*)val);
        } else {
            printf("%s: %s\n", (char*)cur->value, (char*)val);
        }
    }
    searchtree_free(tree_root);
    linked_list_free(keys);
    return 0;
}
