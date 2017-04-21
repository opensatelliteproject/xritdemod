/*
 * SpyServerFrontend.h
 *
 *  Created on: 21/04/2017
 *      Author: Lucas Teske
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


class SpyServerFrontend: public FrontendDevice {
private:
	const int BufferSize = 64 * 1024;
	const int DefaultDisplayPixels = 2000;
	const int DefaultFFTRange = 127;
	const uint32_t ProtocolVersion = SPYSERVER_PROTOCOL_VERSION;
	const std::string SoftwareID = std::string("OpenSatelliteProject " QUOTE(MAJOR_VERSION) "."  QUOTE(MINOR_VERSION) "." QUOTE(MAINT_VERSION));

    SatHelper::TcpClient client;
    std::atomic_bool terminated;
    std::atomic_bool streaming;
    std::atomic_bool gotDeviceInfo;
    std::atomic_bool gotSyncInfo;
    std::atomic_bool canControl;
    std::atomic_bool isConnected;

    uint8_t *headerData;
    std::thread *receiverThread;

    SatHelperException error;
    std::atomic_bool hasError;
    std::string hostname;
    int port;

    DeviceInfo deviceInfo;
    uint32_t parserPhase;
    uint32_t streamingMode;
    int32_t gain;
    uint32_t droppedBuffers;
    uint32_t lastSequenceNumber;
    std::atomic<int64_t> down_stream_bytes;
    int32_t parserPosition;
    uint8_t *bodyBuffer;

    void Connect();
    void Disconnect();
    bool SayHello();
    void Cleanup();
    void OnConnect();
    bool SetSetting(uint32_t settingType, std::vector<uint32_t> params);
    bool SendCommand(uint32_t cmd, uint8_t *args);
    void ParseMessage(char *buffer, int len);
    int ParseHeader(char *buffer, int len);

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


};

} /* namespace OpenSatelliteProject */

#endif /* SRC_SPYSERVERFRONTEND_H_ */
