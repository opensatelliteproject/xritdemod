/*
 * Display.h
 *
 *  Created on: 07/11/2016
 *      Author: Lucas Teske
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <cstdint>
#include <sathelper.h>

class Display {
private:
    SatHelper::ScreenManager screenManager;
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

public:
    Display();
    virtual ~Display();

    inline void start() { this->startTime = SatHelper::Tools::getTimestamp(); }
    void show();

    void update(uint8_t scid, uint8_t vcid, uint64_t packetNumber, uint16_t vitErrors, uint16_t frameBits, int32_t *rsErrors, uint8_t signalQuality,
            uint8_t syncCorrelation, uint8_t phaseCorrection, uint64_t lostPackets, uint16_t averageVitCorrections, uint8_t averageRSCorrections,
            uint64_t droppedPackets, int64_t *receivedPacketsPerChannel, int64_t *lostPacketsPerChannel, uint64_t totalPackets);
};

#endif /* DISPLAY_H_ */
