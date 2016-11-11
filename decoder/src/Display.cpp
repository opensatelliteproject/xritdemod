/*
 * Display.cpp
 *
 *  Created on: 07/11/2016
 *      Author: Lucas Teske
 */

#include "Display.h"
#include <iomanip>

Display::Display() {
    this->scid = 0;
    this->vcid = 0;
    this->packetNumber = 0;
    this->vitErrors = 0;
    this->frameBits = 0;

    for (int i = 0; i < 4; i++) {
        this->rsErrors[i] = 0;
    }

    this->signalQuality = 0;
    this->syncCorrelation = 0;
    this->phaseCorrection = 0;
    this->lostPackets = 0;
    this->averageVitCorrections = 0;
    this->averageRSCorrections = 0;
    this->droppedPackets = 0;
    for (int i = 0; i < 256; i++) {
        this->receivedPacketsPerChannel[i] = 0;
        this->lostPacketsPerChannel[i] = 0;
    }
    this->totalPackets = 0;
    this->startTime = SatHelper::Tools::getTimestamp();
}

Display::~Display() {
    // TODO Auto-generated destructor stub
}

void Display::show() {
    int runningTime = SatHelper::Tools::getTimestamp() - startTime;
    screenManager.GotoXY(0, 0);
    std::cout << "┌─────────────────────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "|                         LRIT DECODER - Lucas Teske                          |" << std::endl;
    std::cout << "|─────────────────────────────────────────────────────────────────────────────|" << std::endl;
    std::cout << "|         Current Frame Data           |               Statistics             |" << std::endl;
    std::cout << "|──────────────────────────────────────|──────────────────────────────────────|" << std::endl;
    std::cout << "|                                      |                                      |" << std::endl;
    std::cout << "| SC ID:                           " << std::setw(3) << (uint32_t)scid << " |  Total Lost Packets:    " << std::setw(10) << lostPackets << "   |"
            << std::endl;
    std::cout << "| VC ID:                           " << std::setw(3) << (uint32_t)vcid << " |  Average Viterbi Correction:  " << std::setw(4) << averageVitCorrections
            << "   |" << std::endl;
    std::cout << "| Packet Number:            " << std::setw(10) << packetNumber << " |  Average RS Correction:         " << std::setw(2)
            << (uint32_t) averageRSCorrections << "   |" << std::endl;
    std::cout << "| Viterbi Errors:       " << std::setw(4) << vitErrors << "/" << std::setw(4) << frameBits << " bits |  Total Dropped Packets: "
            << std::setw(10) << droppedPackets << "   |" << std::endl;
    std::cout << "| Signal Quality:                 " << std::setw(3) << (uint32_t)signalQuality << "% |  Total Packets:         " << std::setw(10) << totalPackets
            << "   |" << std::endl;
    std::cout << "| RS Errors:               " << std::setw(2) << rsErrors[0] << " " << std::setw(2) << rsErrors[1] << " " << std::setw(2) << rsErrors[2] << " "
            << std::setw(2) << rsErrors[3] << " |──────────────────────────────────────|" << std::endl;
    std::cout << "| Sync Correlation:                 " << std::setw(2) << (uint32_t)syncCorrelation << " |             Channel Data             |" << std::endl;
    std::cout << "| Phase Correction:                " << std::setw(3) << (uint32_t)phaseCorrection << " |──────────────────────────────────────|" << std::endl;
    std::cout << "| Running Time:             " << std::setw(10) << runningTime << " |  Chan  |   Received   |     Lost     |" << std::endl;

    int maxChannels = 8;
    int printedChannels = 0;

    for (int i = 0; i < 255; i++) {
        if (receivedPacketsPerChannel[i] != -1) {
            std::cout << "|                                      |   " << std::setw(2) << i << "   |  " << std::setw(10) << receivedPacketsPerChannel[i]
                    << "  |  " << std::setw(10) << lostPacketsPerChannel[i] << "  |" << std::endl;
            printedChannels++;
            if (printedChannels == maxChannels) {
                break;
            }
        }
    }

    for (int i = 0; i < maxChannels - printedChannels; i++) {
        std::cout << "|                                      |        |              |              |" << std::endl;
    }

    std::cout << "└─────────────────────────────────────────────────────────────────────────────┘";
}

void Display::update(uint8_t scid, uint8_t vcid, uint64_t packetNumber, uint16_t vitErrors, uint16_t frameBits, int32_t *rsErrors, uint8_t signalQuality,
        uint8_t syncCorrelation, uint8_t phaseCorrection, uint64_t lostPackets, uint16_t averageVitCorrections, uint8_t averageRSCorrections,
        uint64_t droppedPackets, int64_t *receivedPacketsPerChannel, int64_t *lostPacketsPerChannel, uint64_t totalPackets) {

    this->scid = scid;
    this->vcid = vcid;
    this->packetNumber = packetNumber;
    this->vitErrors = vitErrors;
    this->frameBits = frameBits;

    for (int i = 0; i < 4; i++) {
        this->rsErrors[i] = rsErrors[i];
    }

    this->signalQuality = signalQuality;
    this->syncCorrelation = syncCorrelation;
    this->phaseCorrection = phaseCorrection;
    this->lostPackets = lostPackets;
    this->averageVitCorrections = averageVitCorrections;
    this->averageRSCorrections = averageRSCorrections;
    this->droppedPackets = droppedPackets;
    for (int i = 0; i < 256; i++) {
        this->receivedPacketsPerChannel[i] = receivedPacketsPerChannel[i];
        this->lostPacketsPerChannel[i] = lostPacketsPerChannel[i];
    }
    this->totalPackets = totalPackets;
}
