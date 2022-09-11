#ifndef __REST_API_HANDLER_H__
#define __REST_API_HANDLER_H__

// this is a server handler, 
// received request are handled here
// not the one in rest_api_helper.c(client)


void register_get_handler(void (*get_req_cb)(int));
int init_http_server();

#endif //__REST_API_HANDLER_H__
