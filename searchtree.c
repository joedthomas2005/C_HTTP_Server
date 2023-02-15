#include "searchtree.h"
#include "linkedlist.h"
#include "zeroedmalloc.h"
#include <malloc.h>
#include <string.h>

int searchtree_add_entry(struct searchtree_node** root_ptr, const char* key, void* value, size_t value_len){
    if(*root_ptr == NULL){
        *root_ptr = (struct searchtree_node*)zeroedmalloc(sizeof(struct searchtree_node));
        if(*root_ptr == NULL){
            return -1;
        }

        strncpy((*root_ptr)->key, key, 128);
        (*root_ptr)->value = zeroedmalloc(value_len);
        memcpy((*root_ptr)->value, value, value_len);
        (*root_ptr)->value_len = value_len;
        return 0;
    }
    struct searchtree_node* root = *root_ptr;
    int comparison = strncmp(key, root->key, 128);
    if(comparison > 0) {
        return searchtree_add_entry(&(root->right), key, value, value_len);
    }
    else if (comparison < 0){
        return searchtree_add_entry(&(root->left), key, value, value_len);
    }
    else {
        return -1; // key is already in tree
    }
    return 0;
}

void* searchtree_lookup(struct searchtree_node* root, const char* key, size_t* out_value_len){
    int comparison = strncmp(key, root->key, 128);
    if (comparison == 0){
        void* out_value = zeroedmalloc(root->value_len);
        memcpy(out_value, root->value, root->value_len);
        if (out_value_len != NULL){
            *out_value_len = root->value_len;
        }
        return out_value;
    }
    else if (comparison > 0){
        if(root->right == NULL){
            return NULL;
        }
        return searchtree_lookup(root->right, key, out_value_len);
    } else{
        if(root->left == NULL){
            return NULL;
        }
        return searchtree_lookup(root->left, key, out_value_len);
    }
}

unsigned char searchtree_contains(const struct searchtree_node* root, const char* key){
    int comparison = strncmp(root->key, key, 128);
    if (comparison == 0){
        return 1;
    } else if (comparison > 0){
        if (root->right != NULL){
            return searchtree_contains(root->right, key);
        }
    } else {
        if (root->left != NULL){
            return searchtree_contains(root->left, key);
        }
    }
    return 0;
}

void searchtree_free(struct searchtree_node* root){
    if(root->left != NULL){
        searchtree_free(root->left);
    }
    if(root->right != NULL){
        searchtree_free(root->right);
    }
    free(root->value);
    free(root);
}

/**
 * Run callback for each key value pair in the searchtree. Callback should
 * accept a const char* for the key and a void* for the value
 **/
void searchtree_for_each(const struct searchtree_node* root, void (*callback)(const char*, const void*, size_t)){
    callback(root->key, root->value, root->value_len);
    if (root->left != NULL){
        searchtree_for_each(root->left, callback);
    }
    if (root->right != NULL){
        searchtree_for_each(root->right, callback);
    }
}

/**
 * Place all the keys of the search tree into the linked list who's head is pointed to in out
 **/
int searchtree_get_keys(const struct searchtree_node* root, struct linked_list_node** out){
    if(root != NULL){
        if(linked_list_add_node_str(out, (void*)root->key) == -1){
            return -1;
        }
        if (root->left){
            if(searchtree_get_keys(root->left, out) == -1){
                return -1;
            }
        }
        if (root->right){
            if(searchtree_get_keys(root->right, out) == -1){
                return -1;
            }
        }
    }
    return 0;
}

int searchtree_get_entries(const struct searchtree_node* root, struct linked_list_node** out){
    if (root != NULL){
        if(linked_list_add_node(out, (void*)root, sizeof(struct searchtree_key_value_pair)) == -1){
            return -1;
        }
        if(searchtree_get_entries(root->left, out) == -1){
            return -1;
        }
        if(searchtree_get_entries(root->right, out) == -1){
            return -1;
        }
    }
    return 0;
}

int searchtree_count(const struct searchtree_node* root){
    if (root != NULL){
        return 1 + searchtree_count(root->left) + searchtree_count(root->right);
    }
    return 0;
}
