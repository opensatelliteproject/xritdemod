/*
 * ChannelPacket.h
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#ifndef CHANNELPACKET_H_
#define CHANNELPACKET_H_

class ChannelDispatcher;

class ChannelPacket {
    friend ChannelDispatcher;
private:
    char *data;
    int length;
public:
    ChannelPacket(char *data, int length);
    ~ChannelPacket();
};

#endif /* CHANNELPACKET_H_ */
