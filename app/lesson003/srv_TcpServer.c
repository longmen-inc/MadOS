#include <string.h>
#include <unistd.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Network.h"
#include "dat_Status.h"
#include "lwip/sockets.h"

#define CLIENT_Q_NUM 3

static void tcp_server(MadVptr exData);

void srvTcpServer_Init(void)
{
    madThreadCreate(tcp_server, 0, 1024 * 4, THREAD_PRIO_SRV_TCPSERVER);
}

static void tcp_server(MadVptr exData)
{
    int i, rc, len, idx;
    int s_srv, s_tcp[CLIENT_Q_NUM], s_max;
    fd_set socks;
    struct sockaddr_in addr;
    struct timeval tv;

    (void)exData;

    while(1) {
        madTimeDly(3000);

        if(!EthIf || !netif_is_link_up(EthIf)) {
            continue;
        }

        s_srv = socket(AF_INET, SOCK_STREAM, 0);
        if(s_srv < 0) continue;

        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port        = htons(2222);
        if(0 > bind(s_srv, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
            closesocket(s_srv);
            continue;
        }

        if(0 > listen(s_srv, CLIENT_Q_NUM)) {
            closesocket(s_srv);
            continue;
        }

        idx = 0;
        for(i=0; i<CLIENT_Q_NUM; i++) {
            s_tcp[i] = -1;
        }

        while(1) {
            FD_ZERO(&socks);
            FD_SET(s_srv, &socks);
            s_max = s_srv;
            for(i=0; i<CLIENT_Q_NUM; i++) {
                if(0 > s_tcp[i]) continue;
                FD_SET(s_tcp[i], &socks);
                if(s_tcp[i] > s_max) s_max = s_tcp[i];
            }
            s_max++;

            tv.tv_sec  = 360;
            tv.tv_usec = 0;

            rc = select(s_max, &socks, NULL, NULL, &tv);

            if(rc < 0) {
                break;
            } else if(rc == 0) {
                MAD_LOG("[Tcp Server]Timeout\n");
                continue;
            }

            for(i=0; i<CLIENT_Q_NUM; i++) {
                if(0 > s_tcp[i]) continue;
                if(FD_ISSET(s_tcp[i], &socks)) {
                    char tmp[8] = { 0 };
                    if(0 > read(s_tcp[i], tmp, 8)) { 
                        len = sizeof(struct sockaddr_in);
                        getpeername(s_tcp[i], (struct sockaddr*)&addr, (socklen_t *)&len);
                        closesocket(s_tcp[i]);
                        MAD_LOG("[Tcp Server]Connection[%s:%d] closed\n",
                                inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                        s_tcp[i] = -1;
                    } else {
                        char *out = datStatus_RxJson();
                        write(s_tcp[i], out, strlen(out));
                        free(out);
                    }
                }
            }

            if(FD_ISSET(s_srv, &socks)) {
                int s;
                len = sizeof(struct sockaddr_in);
                s = accept(s_srv, (struct sockaddr*)&addr, (socklen_t *)&len);
                if(0 > s) break;
                if(idx == CLIENT_Q_NUM) idx = 0;
                if(s_tcp[idx] >= 0) closesocket(s_tcp[idx]);
                s_tcp[idx++] = s;
                MAD_LOG("[Tcp Server]New connection[%s:%d]\n",
                        inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            }
        }

        closesocket(s_srv);
        for(i=0; i<CLIENT_Q_NUM; i++) {
            if(0 > s_tcp[i]) continue;
            closesocket(s_tcp[i]);
        }
    }
}
