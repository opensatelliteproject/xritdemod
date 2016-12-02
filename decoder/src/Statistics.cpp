/*
 * Statistics.cpp
 *
 *  Created on: 01/12/2016
 *      Author: lucas
 */

#include "Statistics.h"
#include <sathelper.h>
#include <cstring>
#include <chrono>

Statistics::Statistics() {
    this->data.scid = 0;
    this->data.vcid = 0;
    this->data.packetNumber = 0;
    this->data.vitErrors = 0;
    this->data.frameBits = 0;

    for (int i = 0; i < 4; i++) {
        this->data.rsErrors[i] = 0;
    }

    this->data.signalQuality = 0;
    this->data.syncCorrelation = 0;
    this->data.phaseCorrection = 0;
    this->data.lostPackets = 0;
    this->data.averageVitCorrections = 0;
    this->data.averageRSCorrections = 0;
    this->data.droppedPackets = 0;
    for (int i = 0; i < 256; i++) {
        this->data.receivedPacketsPerChannel[i] = 0;
        this->data.lostPacketsPerChannel[i] = 0;
    }

    this->data.totalPackets = 0;
    this->data.startTime = SatHelper::Tools::getTimestamp();
}

Statistics::~Statistics() {

}

void Statistics::update(const Statistics &statistics) {
    std::memcpy(&this->data, &statistics.data, sizeof(Statistics_st));
}

void Statistics::update(uint8_t scid, uint8_t vcid, uint64_t packetNumber, uint16_t vitErrors, uint16_t frameBits, int32_t *rsErrors, uint8_t signalQuality,
        uint8_t syncCorrelation, uint8_t phaseCorrection, uint64_t lostPackets, uint16_t averageVitCorrections, uint8_t averageRSCorrections,
        uint64_t droppedPackets, int64_t *receivedPacketsPerChannel, int64_t *lostPacketsPerChannel, uint64_t totalPackets) {

    this->data.scid = scid;
    this->data.vcid = vcid;
    this->data.packetNumber = packetNumber;
    this->data.vitErrors = vitErrors;
    this->data.frameBits = frameBits;

    for (int i = 0; i < 4; i++) {
        this->data.rsErrors[i] = rsErrors[i];
    }

    this->data.signalQuality = signalQuality;
    this->data.syncCorrelation = syncCorrelation;
    this->data.phaseCorrection = phaseCorrection;
    this->data.lostPackets = lostPackets;
    this->data.averageVitCorrections = averageVitCorrections;
    this->data.averageRSCorrections = averageRSCorrections;
    this->data.droppedPackets = droppedPackets;
    for (int i = 0; i < 256; i++) {
        this->data.receivedPacketsPerChannel[i] = receivedPacketsPerChannel[i];
        this->data.lostPacketsPerChannel[i] = lostPacketsPerChannel[i];
    }
    this->data.totalPackets = totalPackets;
}
