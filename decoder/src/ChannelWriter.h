/*
 * ChannelWritter.h
 *
 *  Created on: 07/11/2016
 *      Author: Lucas Teske
 */

#ifndef CHANNELWRITTER_H_
#define CHANNELWRITTER_H_

#include <string>
#include <cstdint>

class ChannelWriter {
private:
    std::string baseFolder;
public:
    ChannelWriter(std::string baseFolder);
    void writeChannel(uint8_t *data, int size, uint16_t vcid);
    void dumpCorruptedPacket(uint8_t *data, int size, int type);
    void dumpCorruptedPacketStatistics(uint16_t viterbiErrors, uint8_t syncCorrelation, int32_t *rsErrors);
    void dumpDebugData(uint8_t *data, int size, int type);
};

#endif /* CHANNELWRITTER_H_ */
