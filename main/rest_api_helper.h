#ifndef __REST_API_HELPER_H__
#define __REST_API_HELPER_H__


#define MAX_SEND_BUFFER         512
#define MAX_DATA_PER_SEND       300
#define MAX_BOUNDARY_LEN        60
#define MAX_BOUNDARY_RAN_NUMB   24
#define MAX_CONDISPO            200

void ilp_register_event_handler(
    int (*response_callback)(int response_len, char* response)
);

/* NOTE: not sure why port is not working
 *  but including the port in url is ok url = 172.20.10.5:3000 
 */
int ilp_get_request(char* url, int port);
int ilp_post_request(
    char* url, 
    int port, 
    char* post_data, 
    int post_data_len
);
int ilp_multiform_post_request(
    char* url, 
    int port, 
    char* post_data, 
    int post_data_len,
    char* filename
);


#endif //__REST_API_HELPER_H__