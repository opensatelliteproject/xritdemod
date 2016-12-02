/*
 * ChannelPacket.cpp
 *
 *  Created on: 01/12/2016
 *      Author: Lucas Teske
 */

#include "ChannelPacket.h"
#include <cstring>

ChannelPacket::ChannelPacket(char *data, int length) {
    this->data = new char[length];
    this->length = length;
    std::memcpy(this->data, data, length);
}

ChannelPacket::~ChannelPacket() {
    delete this->data;
}

