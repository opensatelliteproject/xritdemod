/*
 * SpyServerFrontend.h
 *
 *  Created on: 21/04/2017
 *      Author: Lucas Teske
 *      Based on Youssef Touil (youssef@live.com) C# implementation.
 */

#ifndef SRC_SPYSERVERFRONTEND_H_
#define SRC_SPYSERVERFRONTEND_H_

#include "FrontendDevice.h"
#include <SatHelper/sathelper.h>
#include <atomic>
#include "SpyServerProtocol.h"
#include "Parameters.h"

namespace OpenSatelliteProject {

enum ParserPhase {
	AcquiringHeader,
	ReadingData
};

#define SAMPLE_BUFFER_SIZE 256 * 1024


class SpyServerFrontend: public FrontendDevice {
private:
	const uint BufferSize = 64 * 1024;
	const int DefaultDisplayPixels = 2000;
	const int DefaultFFTRange = 127;
	const uint32_t ProtocolVersion = SPYSERVER_PROTOCOL_VERSION;
	const std::string SoftwareID = std::string("OpenSatelliteProject " QUOTE(MAJOR_VERSION) "."  QUOTE(MINOR_VERSION) "." QUOTE(MAINT_VERSION));
	const std::string NameNoDevice = std::string("SpyServer - No Device");
	const std::string NameAirspyOne = std::string("SpyServer - Airspy One");
	const std::string NameAirspyHF = std::string("SpyServer - Airspy HF+");
	const std::string NameRTLSDR = std::string("SpyServer - RTLSDR");
	const std::string NameUnknown = std::string("SpyServer - Unknown Device");

    SatHelper::TcpClient client;

    std::atomic_bool terminated;
    std::atomic_bool streaming;
    std::atomic_bool gotDeviceInfo;
    std::atomic_bool gotSyncInfo;
    std::atomic_bool canControl;
    std::atomic_bool isConnected;

    uint8_t *headerData;
    uint8_t *bodyBuffer;
    uint64_t bodyBufferLength;
    uint32_t parserPosition;
    uint32_t lastSequenceNumber;

    std::thread *receiverThread;

    SatHelperException error;
    std::atomic_bool hasError;
    std::string hostname;
    int port;

    DeviceInfo deviceInfo;
    MessageHeader header;

    uint32_t streamingMode;
    uint32_t parserPhase;

    uint32_t droppedBuffers;
    std::atomic<int64_t> down_stream_bytes;


    uint32_t minimumTunableFrequency;
    uint32_t maximumTunableFrequency;
    uint32_t deviceCenterFrequency;
    uint32_t channelCenterFrequency;
    uint32_t channelDecimationStageCount;
    int32_t gain;

    std::vector<uint32_t> availableSampleRates;
    SatHelper::CircularBuffer<uint8_t> dataS8Queue;

    // Not the best way, I know
    float fBuffer[SAMPLE_BUFFER_SIZE];
    int16_t s16Buffer[SAMPLE_BUFFER_SIZE];

	std::function<void(void *data, int length, int type)> cb;

    bool SayHello();
    void Cleanup();
    void OnConnect();
    bool SetSetting(uint32_t settingType, std::vector<uint32_t> params);
    bool SendCommand(uint32_t cmd, std::vector<uint8_t> args);
    void ParseMessage(char *buffer, uint32_t len);
    int ParseHeader(char *buffer, uint32_t len);
    int ParseBody(char *buffer, uint32_t len);
    void ProcessDeviceInfo();
    void ProcessClientSync();
    void ProcessUInt8Samples();
    void ProcessInt16Samples();
    void ProcessFloatSamples();
    void ProcessUInt8FFT();
    void HandleNewMessage();
    void SetStreamState();
    void threadLoop();

public:
	SpyServerFrontend(std::string &hostname, int port);
	virtual ~SpyServerFrontend();

	uint32_t SetSampleRate(uint32_t sampleRate) override;
	uint32_t SetCenterFrequency(uint32_t centerFrequency) override;
	const std::vector<uint32_t>& GetAvailableSampleRates() override;
	void Start() override;
	void Stop() override;
	void SetAGC(bool agc) override;

	void SetLNAGain(uint8_t value) override;
	void SetVGAGain(uint8_t value) override;
	void SetMixerGain(uint8_t value) override;

	uint32_t GetCenterFrequency() override;

	const std::string &GetName() override;

	uint32_t GetSampleRate() override;

	void SetSamplesAvailableCallback(std::function<void(void*data, int length, int type)> cb) override;

    void Connect();
    void Disconnect();

};

} /* namespace OpenSatelliteProject */

#endif /* SRC_SPYSERVERFRONTEND_H_ */
