//============================================================================
// Name        : GOES LRIT Decoder
// Author      : Lucas Teske
// Version     : 1.0
// Copyright   : Copyright 2016
// Description : GOES LRIT Decoder
//============================================================================

#include <iostream>
#include <memory.h>
#include <cstdint>
#include <sathelper.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Display.h"
#include "ChannelWriter.h"
#include "ChannelDispatcher.h"
#include "StatisticsDispatcher.h"

using namespace std;

//#define DUMP_CORRUPTED_PACKETS
#define USE_LAST_FRAME_DATA
//#define DEBUG_MODE

#define FRAMESIZE 1024
#define FRAMEBITS (FRAMESIZE * 8)
#define CODEDFRAMESIZE (FRAMEBITS * 2)
#define MINCORRELATIONBITS 46
#define RSBLOCKS 4
#define RSPARITYSIZE 32
#define RSPARITYBLOCK (RSPARITYSIZE * RSBLOCKS)
#define SYNCWORDSIZE 32
#define LASTFRAMEDATABITS 64
#define LASTFRAMEDATA (LASTFRAMEDATABITS / 8)
#define TIMEOUT 2

const uint64_t UW0 = 0xfca2b63db00d9794;
const uint64_t UW2 = 0x035d49c24ff2686b;

int main(int argc, char **argv) {
#ifdef USE_LAST_FRAME_DATA
    uint8_t viterbiData[CODEDFRAMESIZE + LASTFRAMEDATABITS];
    uint8_t vitdecData[FRAMESIZE + LASTFRAMEDATA];
    uint8_t decodedData[FRAMESIZE + LASTFRAMEDATA];
    uint8_t lastFrameEnd[LASTFRAMEDATABITS];
#else
    uint8_t vitdecData[FRAMESIZE];
    uint8_t decodedData[FRAMESIZE];
    //uint8_t viterbiData[CODEDFRAMESIZE];
#endif
    uint8_t codedData[CODEDFRAMESIZE];
    uint8_t rsCorrectedData[FRAMESIZE];
    uint8_t rsWorkBuffer[255];

    uint8_t  syncWord[4];
    uint64_t droppedPackets = 0;
    uint64_t averageRSCorrections = 0;
    uint64_t averageVitCorrections = 0;
    uint64_t frameCount = 0;
    uint64_t lostPackets = 0;
    int64_t lostPacketsPerFrame[256];
    int64_t lastPacketCount[256];
    int64_t receivedPacketsPerFrame[256];
    uint32_t checkTime = 0;
    bool runUi = false;
    bool dump = false;
    Statistics statistics;

    SatHelper::Correlator correlator;
    SatHelper::PacketFixer packetFixer;
#ifdef USE_LAST_FRAME_DATA
    SatHelper::Viterbi27 viterbi(FRAMEBITS+LASTFRAMEDATABITS);
#else
    SatHelper::Viterbi27 viterbi(FRAMEBITS);
#endif
    SatHelper::ReedSolomon reedSolomon;
    SatHelper::DeRandomizer deRandomizer;
    ChannelWriter channelWriter("channels");

    Display display;

    reedSolomon.SetCopyParityToOutput(true);

    if (argc > 1) {
        std::string ui(argv[1]);
        if (ui == "display") {
            runUi = true;
        }
    }

    if (argc > 2) {
        std::string dumppacket(argv[2]);
        if (dumppacket == "dump") {
            dump = true;
        }
    }

    for (int i=0; i<256;i++) {
      lostPacketsPerFrame[i] = 0;
      lastPacketCount[i] = -1;
      receivedPacketsPerFrame[i] = -1;
    }

#ifdef USE_LAST_FRAME_DATA
    for (int i=0; i<LASTFRAMEDATABITS; i++) {
        lastFrameEnd[i] = 128;
    }
#endif

    correlator.addWord(UW0);
    correlator.addWord(UW2);

    // Dispatchers
    ChannelDispatcher channelDispatcher;
    StatisticsDispatcher statisticsDispatcher;

    // Socket Init
    SatHelper::TcpServer tcpServer;
    cout << "Starting Demod Receiver at port 5000\n";
    tcpServer.Listen(5000);

    while (true) {
        cout << "Waiting for a client connection" << endl;

        SatHelper::TcpSocket client = tcpServer.Accept();
        cout << "Client connected!" << endl;


        if (runUi) {
            SatHelper::ScreenManager::Clear();
        }

        while (true) {
            uint32_t chunkSize = CODEDFRAMESIZE;
            try {
                checkTime = SatHelper::Tools::getTimestamp();
                while (client.AvailableData() < CODEDFRAMESIZE) {
                    if (SatHelper::Tools::getTimestamp() - checkTime > TIMEOUT) {
                        throw SatHelper::ClientDisconnectedException();
                    }
                }
                client.Receive((char *) codedData, chunkSize);
                correlator.correlate(codedData, chunkSize);

                uint32_t word = correlator.getCorrelationWordNumber();
                uint32_t pos = correlator.getHighestCorrelationPosition();
                uint32_t corr = correlator.getHighestCorrelation();
                SatHelper::PhaseShift phaseShift = word == 0 ? SatHelper::PhaseShift::DEG_0 : SatHelper::PhaseShift::DEG_180;

                if (corr < MINCORRELATIONBITS) {
                    cerr << "Correlation didn't match criteria of " << MINCORRELATIONBITS << " bits." << endl;
                    continue;
                }

                // Sync Frame
                if (pos != 0) {
                    // Shift position
                    char *shiftedPosition = (char *) codedData + pos;
                    memcpy(codedData, shiftedPosition, CODEDFRAMESIZE - pos); // Copy from p to chunk size to start of codedData
                    chunkSize = pos; // Read needed bytes to fill a frame.
                    checkTime = SatHelper::Tools::getTimestamp();
                    while (client.AvailableData() < chunkSize) {
                        if (SatHelper::Tools::getTimestamp() - checkTime > TIMEOUT) {
                            throw SatHelper::ClientDisconnectedException();
                        }
                    }

                    client.Receive((char *) (codedData + CODEDFRAMESIZE - pos), chunkSize);
                }

                // Fix Phase Shift
                packetFixer.fixPacket(codedData, CODEDFRAMESIZE, phaseShift, false);
#ifdef USE_LAST_FRAME_DATA
                // Shift data and add previous values.
                memcpy(viterbiData, lastFrameEnd, LASTFRAMEDATABITS);
                memcpy(viterbiData+LASTFRAMEDATABITS, codedData, CODEDFRAMESIZE);
#endif

                // Viterbi Decode
                //
#ifdef USE_LAST_FRAME_DATA
                viterbi.decode(viterbiData, decodedData);
#else
                viterbi.decode(codedData, decodedData);
#endif
                float signalErrors = viterbi.GetPercentBER();
                signalErrors = 100 - (signalErrors * 10);
                uint8_t signalQuality = signalErrors < 0 ? 0 : (uint8_t)signalErrors;

#ifdef USE_LAST_FRAME_DATA
                // Shift Back
                memmove(decodedData, decodedData+LASTFRAMEDATA/2, FRAMESIZE);

                // Save last data
                memcpy(lastFrameEnd, viterbiData+CODEDFRAMESIZE, LASTFRAMEDATABITS);
#endif
                memcpy(syncWord, decodedData, 4);
                // DeRandomize Stream
                uint8_t skipsize = (SYNCWORDSIZE/8);
                memcpy(vitdecData, decodedData, FRAMESIZE);
                memmove(decodedData, decodedData + skipsize, FRAMESIZE-skipsize);
                deRandomizer.DeRandomize(decodedData, FRAMESIZE-skipsize);

                averageVitCorrections += viterbi.GetBER();
                frameCount++;

                // Reed Solomon Error Correction
                int32_t derrors[4] = { 0, 0, 0, 0 };
                for (int i=0; i<RSBLOCKS; i++) {
                  reedSolomon.deinterleave(decodedData, rsWorkBuffer, i, RSBLOCKS);
                  derrors[i] = reedSolomon.decode_ccsds(rsWorkBuffer);
                  reedSolomon.interleave(rsWorkBuffer, rsCorrectedData, i, RSBLOCKS);
                }

                if (derrors[0] == -1 && derrors[1] == -1 && derrors[2] == -1 && derrors[3] == -1) {
                  droppedPackets++;
                  #ifdef DUMP_CORRUPTED_PACKETS
                  channelWriter.dumpCorruptedPacket(codedData, CODEDFRAMESIZE, 0);
                  channelWriter.dumpCorruptedPacket(vitdecData, FRAMESIZE, 1);
                  channelWriter.dumpCorruptedPacket(rsCorrectedData, FRAMESIZE, 2);
                  channelWriter.dumpCorruptedPacketStatistics(viterbi.GetBER(), corr, derrors);
                  #endif
                  continue;
                } else {
                  averageRSCorrections += derrors[0] != -1 ? derrors[0] : 0;
                  averageRSCorrections += derrors[1] != -1 ? derrors[1] : 0;
                  averageRSCorrections += derrors[2] != -1 ? derrors[2] : 0;
                  averageRSCorrections += derrors[3] != -1 ? derrors[3] : 0;
                }

                // Packet Header Filtering
                //uint8_t versionNumber = (*rsCorrectedData) & 0xC0 >> 6;
                uint8_t scid = ((*rsCorrectedData) & 0x3F) << 2 | (*(rsCorrectedData+1) & 0xC0) >> 6;
                uint8_t vcid = (*(rsCorrectedData+1)) & 0x3F;

                // Packet Counter from Packet
                uint32_t counter = *((uint32_t *) (rsCorrectedData+2));
                counter = SatHelper::Tools::swapEndianess(counter);
                counter &= 0xFFFFFF00;
                counter = counter >> 8;
                //writeChannel(rsCorrectedData, FRAMESIZE - RSPARITYBLOCK - (SYNCWORDSIZE/8), vcid);
                if (dump) {
                    channelWriter.writeChannel(rsCorrectedData, FRAMESIZE - RSPARITYBLOCK - (SYNCWORDSIZE/8), vcid);
                }
                channelDispatcher.add((char *)rsCorrectedData, FRAMESIZE - RSPARITYBLOCK - (SYNCWORDSIZE/8));

                if (lastPacketCount[vcid]+1 != counter && lastPacketCount[vcid] > -1) {
                  int lostCount = counter - lastPacketCount[vcid] - 1;
                  lostPackets += lostCount;
                  lostPacketsPerFrame[vcid] += lostCount;
                }

                lastPacketCount[vcid] = counter;
                receivedPacketsPerFrame[vcid] = receivedPacketsPerFrame[vcid] == -1 ? 1 : receivedPacketsPerFrame[vcid] + 1;
                uint8_t phaseCorr = phaseShift == SatHelper::PhaseShift::DEG_180 ? 180 : 0;
                uint16_t partialVitCorrections = (uint16_t) (averageVitCorrections / frameCount);
                uint8_t partialRSCorrections = (uint8_t) (averageRSCorrections / frameCount);

                statistics.update(scid, vcid, (uint64_t) counter, (int16_t) viterbi.GetBER(), FRAMEBITS, derrors,
                        signalQuality, corr, phaseCorr,
                        lostPackets, partialVitCorrections, partialRSCorrections,
                        droppedPackets, receivedPacketsPerFrame, lostPacketsPerFrame, frameCount, syncWord);

                statisticsDispatcher.Update(statistics);

                if (runUi) {
                    display.update(scid, vcid, (uint64_t) counter, (int16_t) viterbi.GetBER(), FRAMEBITS, derrors,
                            signalQuality, corr, phaseCorr,
                            lostPackets, partialVitCorrections, partialRSCorrections,
                            droppedPackets, receivedPacketsPerFrame, lostPacketsPerFrame, frameCount);

                    display.show();
                }

            } catch (SatHelper::SocketException &e) {
                cerr << endl;
                cerr << "Client disconnected" << endl;
                cerr << "   " << e.what() << endl;
                break;
            }
        }

        client.Close();
    }
    return 0;
}
