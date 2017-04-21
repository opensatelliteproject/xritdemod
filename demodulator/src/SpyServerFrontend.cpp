/*
 * SpyServerFrontend.cpp
 *
 *  Created on: 21/04/2017
 *      Author: Lucas Teske
 */

#include "SpyServerFrontend.h"
#include <cstring>

namespace OpenSatelliteProject {

SpyServerFrontend::SpyServerFrontend(std::string &hostname, int port) :
		client(hostname, port), terminated(false), streaming(false), gotDeviceInfo(false),
		receiverThread(NULL), hasError(false), hostname(hostname), port(port), streamingMode(STREAM_MODE_IQ_ONLY),
		bodyBuffer(NULL) {
	headerData = new uint8_t[sizeof(MessageHeader)];
}

SpyServerFrontend::~SpyServerFrontend() {
	Disconnect();
	delete[] headerData;
}

void SpyServerFrontend::Connect() {
	if (receiverThread != NULL) {
		return;
	}

	client.Connect();
	isConnected = true;

	SayHello();
	Cleanup();

	terminated = false;
	hasError = false;
	gotSyncInfo = false;
	gotDeviceInfo = false;

	receiverThread  = new std::thread(&SpyServerFrontend::threadLoop, this);

	for (int i=0; i<1000, !hasError; i++) {
		if (gotDeviceInfo) {
			if (gotDeviceInfo) {
				if (deviceInfo.DeviceType == DEVICE_INVALID) {
					error = std::exception("Server is up but no device is available");
					hasError = true;
					break;
				}

				if (gotSyncInfo) {
					OnConnect();
					return;
				}
			}
		}
	}

	Disconnect();
	if (hasError) {
		hasError = false;
		throw error;
	}

	throw SatHelperException("Server didn't send the device capability and synchronization info.");
}

void SpyServerFrontend::Disconnect() {
	terminated = true;
	if (isConnected) {
		client.Close();
	}

	if (receiverThread != NULL) {
		receiverThread->join();
		receiverThread = NULL;
	}

	Cleanup();
}

void SpyServerFrontend::OnConnect() {
	SetSetting(SETTING_STREAMING_MODE, { streamingMode });
	SetSetting(SETTING_IQ_FORMAT, { STREAM_FORMAT_INT16 });
	SetSetting(SETTING_FFT_FORMAT, { STREAM_FORMAT_UINT8 });
	//SetSetting(SETTING_FFT_DISPLAY_PIXELS, { displayPixels });
	//SetSetting(SETTING_FFT_DB_OFFSET, { fftOffset });
	//SetSetting(SETTING_FFT_DB_RANGE, { fftRange });
}

bool SpyServerFrontend::SetSetting(uint32_t settingType, std::vector<uint32_t> params) {
	uint8_t *argBytes;
	if (params.size() > 0) {
		argBytes = new uint8_t[sizeof(SettingType) + params.size() * sizeof(uint32_t)];
		uint8_t *settingBytes = (uint8_t *) &settingType;
		for (int i=0; i< sizeof(uint32_t); i++) {
			argBytes[i] = settingBytes[i];
		}

		std::memcpy(argBytes+sizeof(uint32_t), &params[0], sizeof(uint32_t) * params.size());
	} else {
		argBytes = NULL;
	}

	bool result = SendCommand(CMD_SET_SETTING, argBytes);
	if (argBytes != NULL) {
		delete[] argBytes;
	}
	return result;
}

bool SpyServerFrontend::SayHello() {
	const uint8_t *protocolVersionBytes = (const uint8_t *) &ProtocolVersion;
	const uint8_t *softwareVersionBytes = (const uint8_t *) SoftwareID.c_str();
	uint8_t *args = new uint8_t[sizeof(ProtocolVersion) + SoftwareID.size()];

	std::memcpy(args, protocolVersionBytes, sizeof(ProtocolVersion));
	std::memcpy(args + sizeof(ProtocolVersion), softwareVersionBytes, SoftwareID.size());

	bool result = SendCommand(CMD_HELLO, args);
	delete[] args;
	return result;
}

void SpyServerFrontend::Cleanup() {
    deviceInfo.DeviceType = 0;
    deviceInfo.DeviceSerial = 0;
    deviceInfo.DecimationStageCount = 0;
    deviceInfo.GainStageCount = 0;
    deviceInfo.MaximumSampleRate = 0;
    deviceInfo.MaximumBandwidth = 0;
    deviceInfo.MaximumGainIndex = 0;
    deviceInfo.MinimumFrequency = 0;
    deviceInfo.MaximumFrequency = 0;

    gain = 0;
    //displayCenterFrequency = 0;
    //deviceCenterFrequency = 0;
    //displayDecimationStageCount = 0;
    //channelDecimationStageCount = 0;
    //minimumTunableFrequency = 0;
    //maximumTunableFrequency = 0;
    canControl = false;
    gotDeviceInfo = false;
    gotSyncInfo = false;

    lastSequenceNumber = ((uint32_t)-1);
    droppedBuffers = 0;
    down_stream_bytes = 0;

    parserPhase = AcquiringHeader;
    parserPosition = 0;

    streaming = false;
    terminated = true;
}

void SpyServerFrontend::threadLoop() {
	parserPhase = AcquiringHeader;
	parserPosition = 0;

	char buffer[BufferSize];
	try {
		if (terminated) {
			break;
		}
		uint32_t availableData = client.AvailableData();
		if (availableData > 0) {
			availableData = availableData > BufferSize ? BufferSize : availableData;
			client.Receive(buffer, availableData);
			ParseMessage(buffer, availableData);
		}
	} catch (SatHelperException &e) {
		error = e;
	}

	if (bodyBuffer != NULL) {
		delete[] bodyBuffer;
		bodyBuffer = NULL;
	}

	Cleanup();
}

void SpyServerFrontend::ParseMessage(char *buffer, int len) {
	down_stream_bytes++;

	int consumed;
	while (len > 0 && !terminated) {
		if (parserPhase == AcquiringHeader) {
			while (parserPhase == AcquiringHeader && len > 0) {
				consumed = ParseHeader(buffer, len);
				buffer += consumed;
				len -= consumed;
			}

			if (parserPhase == ReadingData) {
				// TODO
			}
		}
	}
}

} /* namespace OpenSatelliteProject */
