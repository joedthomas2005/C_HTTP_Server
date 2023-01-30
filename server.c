#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<string.h>
#include<malloc.h>
#include "searchtree.h"

#define PORT 8081
#define BUFSIZE 8192 
#define TRUE 1
#define FALSE 0
#define bool unsigned char

#define ERROR(err) {\
    fprintf(stderr, "%s\n", err); \
    return -1; \
    }

struct http_request {
    char method[8];
    char path[2049];
    struct treenode headers;
    char* body;
    size_t body_len;
};

struct http_response {
    int response_code;
    char response_message[256];
    struct treenode* headers;
};

int listen_connections(int port);
int parse_http_request(char* request, size_t request_len, struct http_request* req_struct);
int split_headers(struct treenode* header_search_tree, char* header_block, int header_length);
int accept_connection(int conn_sock, struct sockaddr_in conn_addr, socklen_t conn_addr_size);

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
        ERROR("Could not set socket option");
    }
    struct sockaddr_in bind_info = { 0 };
    bind_info.sin_addr.s_addr = 0;
    bind_info.sin_family = AF_INET;
    bind_info.sin_port = htons(PORT);
    memset(bind_info.sin_zero, 0, 8);
    if (bind(sock_fd, (struct sockaddr*)&bind_info, sizeof(bind_info))){
        ERROR("Could not bind socket");
    }
    if (listen(sock_fd, 30)){
        ERROR("Could not listen on socket");
    }
    while (1){
        int conn_sock;
        struct sockaddr_in conn_addr = { 0 };
        socklen_t conn_addr_size = sizeof(conn_addr);
        if ((conn_sock = accept(sock_fd, (struct sockaddr*) &conn_addr, &conn_addr_size)) == -1){
            shutdown(sock_fd, SHUT_RDWR);
            ERROR("Failure in accepting connection");
        }
        accept_connection(conn_sock, conn_addr, conn_addr_size);
    }
    close(sock_fd);
    return 0;
}

int accept_connection(int conn_sock, struct sockaddr_in conn_addr, socklen_t conn_addr_size){
    printf("Connection from %s\n", inet_ntoa(conn_addr.sin_addr));
    char* buf = (char*)malloc((BUFSIZE) * sizeof(char));
    
    ssize_t buf_size = recv(conn_sock, buf, BUFSIZE * sizeof(char), 0);
    if (buf_size == -1){
        close(conn_sock);
        ERROR("Failure in receiving data");    
    }
    printf("Received %ld bytes from client\n", buf_size);
    
    struct http_request req_struct = { 0 };
    if (parse_http_request(buf, buf_size, &req_struct)){
        close(conn_sock);
        free(buf);
        ERROR("Failure parsing http request");
    }
    printf("Method: %s\n", req_struct.method);
    printf("Path: %s\n", req_struct.path);
    printf("Body: %s\n", req_struct.body);
    searchtree_for_each(&(req_struct.headers), print_callback_func);
    free(req_struct.body);
    searchtree_free(&(req_struct.headers));
    return 0;
}

int parse_http_request(char* requestbuf, size_t buflen, struct http_request* out_struct){
    int cur_read_pos = 0;
    int method_len = 0;
    while (requestbuf[method_len] != ' ' && method_len < buflen) method_len++;
    if (method_len > 7){
        return -1;
    }
    strncpy(out_struct->method, requestbuf, method_len);
    cur_read_pos += method_len + 1;
    int path_len = 0;
    while ((requestbuf + cur_read_pos)[path_len] != ' ' && path_len + cur_read_pos < buflen) path_len++;
    if (path_len > 2048){
        return -1;
    }
    strncpy(out_struct->path, requestbuf + cur_read_pos, path_len);
    cur_read_pos += path_len + 1;
    
    if(requestbuf[cur_read_pos + 9] != '\n'){
        return -1;
    }
    cur_read_pos += 9;
    int header_portion_length = 0;
    while (cur_read_pos + header_portion_length < buflen && strncmp("\r\n\r\n", requestbuf + header_portion_length + cur_read_pos - 3, 4) != 0){
        header_portion_length += 1;
    }
    char* header_portion = (char*)malloc((header_portion_length - 1) * sizeof(char));
    memset(header_portion, 0, (header_portion_length - 1) * sizeof(char));
    strncpy(header_portion, requestbuf + cur_read_pos + 1, header_portion_length - 2);
    cur_read_pos += header_portion_length + 1;
    out_struct->body = (char*)malloc(buflen - cur_read_pos + 1);
    memset(out_struct->body, 0, buflen - cur_read_pos + 1);
    strncpy(out_struct->body, requestbuf + cur_read_pos, buflen - cur_read_pos);
    if(split_headers(&(out_struct->headers), header_portion, header_portion_length - 2)){
        return -1;
    }
    free(header_portion);
    return 0;
}

int split_headers(struct treenode* search_tree_root, char* header_block, int header_block_length){
    int cur_pos = 0;
    int line_pos = 0;
    int header_name_length = 0;
    int header_value_length = 0;
    bool split_found = FALSE;
    for (; cur_pos < header_block_length; cur_pos++){
        if (header_block[cur_pos] == '\n'){
            if(!split_found){
                return -1;
            }
            header_value_length = line_pos - header_name_length - 1;
            char* header_value = (char*)malloc(sizeof(char) * header_value_length + 1);
            strncpy(header_value, header_block + cur_pos - header_value_length + 1, header_value_length - 1);
            char* header_name = (char*)malloc(sizeof(char) * header_name_length + 1);
            strncpy(header_name, header_block + cur_pos - line_pos, header_name_length);
            if(searchtree_add_entry(search_tree_root, header_name, header_value, sizeof(char) * (header_value_length + 1))){
                return -1;            
            }
            free(header_value);
            free(header_name);
            line_pos = -1;
            header_name_length = 0;
            header_value_length = 0;
            split_found = FALSE;
        }
        if (header_block[cur_pos] == ':' && !split_found){
            split_found = TRUE;
            header_name_length = line_pos;
        }
        line_pos += 1;
    }
    return 0;
}
