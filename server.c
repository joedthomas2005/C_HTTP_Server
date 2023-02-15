#include <stddef.h>
#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<string.h>
#include<malloc.h>

#include "zeroedmalloc.h"
#include "searchtree.h"
#include "linkedlist.h"

#define TRUE 1
#define FALSE 0
#define ERR -1
#define PORT 8081
#define BUFSIZE 8192
#define bool unsigned char

struct http_request {
    char method[8];
    char path[2049];
    struct searchtree_node* headers;
    char* body;
    size_t body_len;
};

struct http_response {
    int response_code;
    char response_message[256];
    struct searchtree_node* headers;
};

int listen_connections(int port);
int parse_http_request(char* request, size_t request_len, struct http_request* req_struct);
int split_headers(struct searchtree_node** header_search_tree, char* header_block, size_t header_length);
int accept_connection(int conn_sock, struct sockaddr_in conn_addr, socklen_t conn_addr_size);
int create_http_response(int status_code, const char* response_message, const struct searchtree_node* custom_headers, const char* mime_type, const char* body, size_t body_len);
int split_header_line(struct searchtree_node** header_search_tree, char* header_line, size_t header_line_length);

int main(int argc, char** argv){
    printf("Beginning HTTP Server on port %d\n", PORT);
    listen_connections(PORT);
    return 0;
}

void print_callback_func(const char* key, const void* value, size_t value_len){
    printf("Header Name: %s\n", key);
    printf("Header Value: %s\n", (char*)value);
}

int listen_connections(int port){
    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    int true_val = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &true_val, sizeof(true_val))){
        return ERR;
    }
    struct sockaddr_in bind_info = { 0 };
    bind_info.sin_addr.s_addr = 0;
    bind_info.sin_family = AF_INET;
    bind_info.sin_port = htons(PORT);
    memset(bind_info.sin_zero, 0, 8);
    if (bind(sock_fd, (struct sockaddr*)&bind_info, sizeof(bind_info))){
        goto error;
    }
    if (listen(sock_fd, 30)){
        goto error;
    }
    while (1){
        int conn_sock;
        struct sockaddr_in conn_addr = { 0 };
        socklen_t conn_addr_size = sizeof(conn_addr);
        if ((conn_sock = accept(sock_fd, (struct sockaddr*) &conn_addr, &conn_addr_size)) == ERR){
            goto error;
        }
        accept_connection(conn_sock, conn_addr, conn_addr_size);
    }
    close(sock_fd);
    return 0;
    error:
    close(sock_fd);
    return ERR;
}

int accept_connection(int conn_sock, struct sockaddr_in conn_addr, socklen_t conn_addr_size){
    int RET = 0;
    struct http_request req_struct = { 0 };
    char* buf = (char*)zeroedmalloc((BUFSIZE) * sizeof(char));
    char* file_data = NULL;
    char* filename = NULL;
    size_t file_data_size = 0;
    char mime_type[256];
    struct http_response response = { 0 };
    struct searchtree_node* response_custom_headers = NULL;
    
    printf("Connection from %s\n", inet_ntoa(conn_addr.sin_addr));
    ssize_t buf_size = recv(conn_sock, buf, BUFSIZE * sizeof(char), 0);
    if (buf_size == ERR){
        RET = ERR;
        goto cleanup;
    }
    printf("Received %ld bytes from client\n", buf_size);

    if (parse_http_request(buf, buf_size, &req_struct)){
        RET = ERR;
        goto cleanup;
    }
    if ((/*filename = searchtree_lookup(WEB_PAGE_ROUTES, req_struct.path, NULL)*/&RET) != NULL){
        file_data = NULL;
        /*file_data_size = get_file_contents(filename, &file_data);*/
        if(file_data_size == -1){
            RET = ERR;
            goto cleanup;
        }
        /*if (get_mime_type(filename, mime_type) == -1){*/
            /*RET = ERR;*/
            /*goto cleanup;*/
        /*}*/
        searchtree_add_entry(&response_custom_headers, "TestHeader", "This is a test", sizeof(char) * (strlen("This is a test") + 1));
        searchtree_add_entry(&response_custom_headers, "TestHeader2", "This is also a test", sizeof(char) * (strlen("This is also a test") + 1));
        create_http_response(200, "OK", response_custom_headers, mime_type, file_data, file_data_size);
        searchtree_free(response_custom_headers);
        response_custom_headers = NULL;
    }
    cleanup:
    close(conn_sock);
    free(buf);
    free(filename);
    free(file_data);
    free(req_struct.body);
    searchtree_free(req_struct.headers);
    return RET;
}

int parse_http_request(char* requestbuf, size_t buflen, struct http_request* out_struct){
    int cur_read_pos = 0;
    int method_len = 0;
    while (requestbuf[method_len] != ' ' && method_len < buflen) method_len++;
    if (method_len > 7){
        return ERR;
    }
    strncpy(out_struct->method, requestbuf, method_len);
    cur_read_pos += method_len + 1;
    int path_len = 0;
    while ((requestbuf + cur_read_pos)[path_len] != ' ' && path_len + cur_read_pos < buflen) path_len++;
    if (path_len > 2048){
        return ERR;
    }
    strncpy(out_struct->path, requestbuf + cur_read_pos, path_len);
    cur_read_pos += path_len + 1;

    if(requestbuf[cur_read_pos + 9] != '\n'){
        return ERR;
    }
    cur_read_pos += 9;
    int header_portion_length = 0;
    while (cur_read_pos + header_portion_length < buflen && strncmp("\r\n\r\n", requestbuf + header_portion_length + cur_read_pos - 3, 4) != 0){
        header_portion_length += 1;
    }
    if(header_portion_length + cur_read_pos >= buflen){
        return ERR;
    }
    char* header_portion = (char*)zeroedmalloc((header_portion_length - 1) * sizeof(char));
    if(header_portion == NULL){
        return ERR;
    }
    strncpy(header_portion, requestbuf + cur_read_pos + 1, header_portion_length - 2); // -2 removes \r\n
    cur_read_pos += header_portion_length + 1;
    out_struct->body = (char*)zeroedmalloc(buflen - cur_read_pos + 1);
    if(out_struct->body == NULL){
        return ERR;
    }
    int RET = 0;
    strncpy(out_struct->body, requestbuf + cur_read_pos, buflen - cur_read_pos);
    if(split_headers(&(out_struct->headers), header_portion, header_portion_length - 2) == ERR){
        RET = ERR;
        goto end;
    }
    struct linked_list_node* keys_list = NULL;
    if(searchtree_get_keys(out_struct->headers, &keys_list) == ERR){
        RET = ERR;
        goto end;
    }

    end:
    free(header_portion);
    linked_list_free(keys_list);
    return RET;
}

int split_headers(struct searchtree_node** search_tree_root, char* header_block, size_t header_block_length){
    int last_line = 0;
    for (size_t pos = 0; pos < header_block_length; pos++){
        if(header_block[pos] == '\n'){
            char* line = (char*)zeroedmalloc(pos - last_line - 1);
            if(line == NULL) return ERR;
            memcpy(line, header_block + last_line, pos - last_line - 1);
            if(split_header_line(search_tree_root, line, pos - last_line - 1) == ERR) return ERR;
            free(line);
            last_line = pos + 1;
        }
    }
    return 0;
}

int split_header_line(struct searchtree_node** search_tree_root, char* header_line, size_t header_line_length){
    int colon_pos = -1;
    for (size_t i = 0; i < header_line_length; i++){
        if(header_line[i] == ':'){
            colon_pos = i;
            break;
        }
    }
    if (colon_pos <= 0 || colon_pos >= header_line_length - 1){
        return ERR;
    }
    char* header_name = (char*)zeroedmalloc(sizeof(char) * (colon_pos + 1));
    if(header_name == NULL) return ERR;
    char* header_value = (char*)zeroedmalloc(sizeof(char) * header_line_length - colon_pos - 1);
    if(header_value == NULL) return ERR;
    strncpy(header_name, header_line, colon_pos);
    strncpy(header_value, header_line + colon_pos + 2, header_line_length - colon_pos - 2);
    if (searchtree_add_entry(search_tree_root, header_name, header_value, (strlen(header_value) + 1) * sizeof(char)) == ERR) {
        free(header_name);
        free(header_value);
        return ERR;
    }
    free(header_name);
    free(header_value);
    return 0;
}

int create_http_response(int status_code, const char* response_message, const struct searchtree_node* custom_headers, const char* mime_type, const char* body, size_t body_len){
    char* status_line = (char*)malloc(16 + strlen(response_message)); // HTTP/1.1 xxx MESSAGE\r\n\0
    snprintf(status_line, strlen(response_message) + 16, "HTTP/1.1 %3.3i %s\r\n", status_code, response_message);
    printf("%s\n", status_line);
    free(status_line);
    if (custom_headers != NULL){
        struct linked_list_node* custom_header_list = NULL;
        if (searchtree_get_entries(custom_headers, &custom_header_list) == -1){
            return -1;
        }
        for (const struct linked_list_node* cur = custom_header_list; cur != NULL; cur = cur->next){
            struct searchtree_key_value_pair* header = (struct searchtree_key_value_pair*)cur->value;
            printf("%s: %s\n", header->key, (char*)header->value);
        }
        linked_list_free(custom_header_list);
    }
    return 0;
}
