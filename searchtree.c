#include "searchtree.h"
#include <malloc.h>
#include <string.h>

int searchtree_add_entry(struct treenode* root, const char* key, void* value, size_t value_len){
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
            root->right = (struct treenode*)malloc(sizeof(struct treenode));
            memset(root->right, 0, sizeof(struct treenode));
        }
        return searchtree_add_entry(root->right, key, value, value_len);
    }
    else if (comparison < 0){
        if (root->left == NULL){
            root->left = (struct treenode*)malloc(sizeof(struct treenode));
            memset(root->left, 0, sizeof(struct treenode));
        }
        return searchtree_add_entry(root->left, key, value, value_len);
    }
    else {
        return -1; // key is already in tree
    }
    return 0;
}

void* searchtree_lookup(struct treenode* root, const char* key, size_t* out_value_len){
    int comparison = strncmp(key, root->key, 128);
    if (comparison == 0){
        void* out_value = malloc(root->value_len);
        memset(out_value, 0, root->value_len);
        memcpy(out_value, root->value, root->value_len);
        out_value_len = malloc(sizeof(size_t));
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

void searchtree_free(struct treenode* root){
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

void searchtree_for_each(struct treenode* root, void (*callback)(const char*, const void*, size_t)){
    if (root->value != NULL){
        callback(root->key, root->value, root->value_len);
    }
    if (root->left != NULL){
        searchtree_for_each(root->left, callback);
    }
    if (root->right != NULL){
        searchtree_for_each(root->right, callback);
    }
}
