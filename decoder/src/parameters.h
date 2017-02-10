/*
 * parameters.h
 *
 *  Created on: 10/02/2017
 *      Author: Lucas Teske
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#define Q(x) #x
#define QUOTE(x) Q(x)

#ifndef MAJOR_VERSION
#define MAJOR_VERSION unk
#endif
#ifndef MINOR_VERSION
#define MINOR_VERSION unk
#endif
#ifndef MAINT_VERSION
#define MAINT_VERSION unk
#endif
#ifndef GIT_SHA1
#define GIT_SHA1 unk
#endif

#define USE_LAST_FRAME_DATA
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


#define DEFAULT_FLYWHEEL_RECHECK 4
#define DEFAULT_DEMODULATOR_PORT 5000
#define DEFAULT_VCHANNEL_PORT 5001
#define DEFAULT_STATISTICS_PORT 5002

// Config file keys
#define CFG_MODE "mode"
#define CFG_DISPLAY "display"
#define CFG_DUMP_PACKET "dumpPackets"
#define CFG_FLYWHEEL_RECHECK "flywheelRecheck"
#define CFG_DEBUG_MODE "debug"
#define CFG_DEMOD_PORT "demodulatorPort"
#define CFG_VCHANNEL_PORT "vChannelPort"
#define CFG_STATISTICS_PORT "statisticsPort"

#endif /* PARAMETERS_H_ */
