#ifndef _TCP_CONTROL_SERVER_H
#define _TCP_CONTROL_SERVER_H

void initTcpServer();         //initialize the tcpServer
void reInitTcpServer();       //reinit tcp server
void sendTcpResponseMessages(char *msg, uint32_t len); //send the control messages to tcp client
int handleTcpControlMessages();  //receive the control messages from tcp client


#endif // _TCP_CONTROL_SERVER_H
