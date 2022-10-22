#ifndef __WIFI_H__
#define __WIFI_H__

#define ILP_WIFI_STAT_NONE                      0
#define ILP_WIFi_STAT_IDLE                      1
#define ILP_WIFI_STAT_COMPLETED                 2
#define ILP_WIFI_STAT_CONNECTING                3
#define ILP_WIFI_STAT_CONNECTED                 4
#define ILP_WIFI_STAT_CONNECTION_LOST           5
#define ILP_WIFI_STAT_DISCONNECTED              6
#define ILP_WIFI_STAT_FAILED                    98
#define ILP_WIFI_STAT_ERROR                     99

#define ILP_WIFI_MODE_CLIENT                    0
#define ILP_WIFI_MODE_AP                        1

#define ILP_WIFI_MAX_RETRY                      5

//same as turnon wifi
int ilp_wifi_enable(void);
//same as turnoff wifi
int ilp_wifi_disable(void);
//this should be called first after enabling
void ilp_wifi_init(void);
//this should be called if you want to stop wifi from system
void ilp_wifi_deinit(void);
int ilp_wifi_config_client(char* ssid, char* pw);
int ilp_wifi_config_ap(char* ap_ssid, char* ap_pw);
int ilp_get_wifi_status(void);

int ilp_wifi_connect(void);
int ilp_wifi_disconnect(void);

int ilp_get_ipchar(char* ipstr);
int ilp_get_netmaskchar(char* netmaskstr);




#endif //__WIFI_H__
