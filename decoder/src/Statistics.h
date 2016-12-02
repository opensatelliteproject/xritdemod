/*
 * Statistics.h
 *
 *  Created on: 01/12/2016
 *      Author: lucas
 */

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <cstdint>

#pragma pack(push, 1)
struct Statistics_st {
    uint8_t scid;
    uint8_t vcid;
    uint64_t packetNumber;
    uint16_t vitErrors;
    uint16_t frameBits;
    uint32_t rsErrors[4];
    uint8_t signalQuality;
    uint8_t syncCorrelation;
    uint8_t phaseCorrection;
    uint64_t lostPackets;
    uint16_t averageVitCorrections;
    uint8_t averageRSCorrections;
    uint64_t droppedPackets;
    int64_t receivedPacketsPerChannel[256];
    int64_t lostPacketsPerChannel[256];
    uint64_t totalPackets;
    uint32_t startTime;
};
#pragma pack(pop)

class Statistics {
private:
    Statistics_st data;

public:
    Statistics();
    ~Statistics();

    void update(uint8_t scid, uint8_t vcid, uint64_t packetNumber, uint16_t vitErrors, uint16_t frameBits, int32_t *rsErrors, uint8_t signalQuality,
            uint8_t syncCorrelation, uint8_t phaseCorrection, uint64_t lostPackets, uint16_t averageVitCorrections, uint8_t averageRSCorrections,
            uint64_t droppedPackets, int64_t *receivedPacketsPerChannel, int64_t *lostPacketsPerChannel, uint64_t totalPackets);

    void update(const Statistics &statistics);

    Statistics_st &GetData() { return data; }
};

#endif /* STATISTICS_H_ */
