#ifndef _UDP_MULTICAST_H
#define _UDP_MULTICAST_H



/*@brief initialize multicasting
*/
void initMulticastReceiver();

/*@brief initialize multicasting
*/
void initMulticastSender();

/*@brief receive udp multicasting messages and send identity response to sender
 * returns 0 if message received successfully
 *         1 if failed 
*/
int receiveMulticastMessage();

/*@brief send the discovery response message over tcp server back to multicast msg sender
 * returns 0 for success
 *         1 if connect to tcp server/msg sent failed  
*/
int sendIdentityResponse(char *);

/*@brief send the udp multicasting messages
 * returns 0 for success
*/
void transmitMulticastMessage();

int sendIdentityResponse_Cloud(char *);

#endif // _UDP_MULTICAST_RECEIVER_H
