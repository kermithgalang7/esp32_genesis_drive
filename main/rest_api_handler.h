#ifndef __REST_API_HANDLER_H__
#define __REST_API_HANDLER_H__

// this is a server handler, 
// received request are handled here
// not the one in rest_api_helper.c(client)

#define MAX_URI_HANDLER         10

int register_get_handler(char* uri, void (*get_req_cb)(char*));
int init_http_server();

struct callbacklist_t {
    char* uri;
    void (*get_req_callback)(char*);
};

#endif //__REST_API_HANDLER_H__
