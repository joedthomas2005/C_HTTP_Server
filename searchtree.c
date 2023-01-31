#include "searchtree.h"
#include "linkedlist.h"
#include <malloc.h>
#include <string.h>

int searchtree_add_entry(struct searchtree_node* root, const char* key, void* value, size_t value_len){
    if(root->value == NULL){
        strncpy(root->key, key, 128);
        root->value = malloc(value_len);
        memcpy(root->value, value, value_len);
        root->value_len = value_len;
        return 0;
    }
    int comparison = strncmp(key, root->key, 128);
    if(comparison > 0) {
        if (root->right == NULL){
            root->right = (struct searchtree_node*)malloc(sizeof(struct searchtree_node));
            memset(root->right, 0, sizeof(struct searchtree_node));
        }
        return searchtree_add_entry(root->right, key, value, value_len);
    }
    else if (comparison < 0){
        if (root->left == NULL){
            root->left = (struct searchtree_node*)malloc(sizeof(struct searchtree_node));
            memset(root->left, 0, sizeof(struct searchtree_node));
        }
        return searchtree_add_entry(root->left, key, value, value_len);
    }
    else {
        return -1; // key is already in tree
    }
    return 0;
}

void* searchtree_lookup(struct searchtree_node* root, const char* key, size_t* out_value_len){
    int comparison = strncmp(key, root->key, 128);
    if (comparison == 0){
        void* out_value = malloc(root->value_len);
        memset(out_value, 0, root->value_len);
        memcpy(out_value, root->value, root->value_len);
        memcpy(out_value_len, &(root->value_len), sizeof(size_t));
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

void searchtree_free(struct searchtree_node* root){
    if(root->left != NULL){
        searchtree_free(root->left);
        free(root->left);
    }
    if(root->right != NULL){
        searchtree_free(root->right);
        free(root->right);
    }
    free(root->value);
}

void searchtree_for_each(const struct searchtree_node* root, void (*callback)(const char*, const void*, size_t)){
    callback(root->key, root->value, root->value_len);
    if (root->left != NULL){
        searchtree_for_each(root->left, callback);
    }
    if (root->right != NULL){
        searchtree_for_each(root->right, callback);
    }
}

int searchtree_get_keys(const struct searchtree_node* root, struct linked_list_node* out){
    if(linked_list_add_node(out, (void*)root->key, strnlen(root->key, 129) * sizeof(char)) == -1){
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
    return 0;
}