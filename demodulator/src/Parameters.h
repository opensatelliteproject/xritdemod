/*
 * Parameters.h
 *
 *  Created on: 31/01/2017
 *      Author: Lucas Teske
 */

#ifndef SRC_PARAMETERS_H_
#define SRC_PARAMETERS_H_

#define Q(x) #x
#define QUOTE(x) Q(x)

// These are the parameters used by the demodulator. Change with care.

// GOES HRIT Settings
#define HRIT_CENTER_FREQUENCY 1694100000
#define HRIT_SYMBOL_RATE      927000
#define HRIT_RRC_ALPHA        0.3f

// GOES LRIT Settings
#define LRIT_CENTER_FREQUENCY 1691000000
#define LRIT_SYMBOL_RATE      293883
#define LRIT_RRC_ALPHA        0.5f

// Loop Settings
#define LOOP_ORDER 2
#define RRC_TAPS 63
#define PLL_ALPHA 0.025
#define CLOCK_ALPHA 0.0037
#define CLOCK_MU 0.5
#define CLOCK_OMEGA_LIMIT 0.005
#define CLOCK_GAIN_OMEGA (CLOCK_ALPHA * CLOCK_ALPHA) / 4.0
#define AGC_RATE 0.01
#define AGC_REFERENCE 0.5
#define AGC_GAIN 1
#define AGC_MAX_GAIN 4000

#define AIRSPY_MINI_DEFAULT_SAMPLERATE 3000000
#define AIRSPY_R2_DEFAULT_SAMPLERATE   2500000
#define DEFAULT_SAMPLE_RATE AIRSPY_MINI_DEFAULT_SAMPLERATE
#define DEFAULT_DECIMATION 1

#define DEFAULT_LNA_GAIN 5
#define DEFAULT_VGA_GAIN 5
#define DEFAULT_MIX_GAIN 5

// FIFO Size in Samples
// 1024 * 1024 samples is about 4Mb of ram.
// This should be more than enough
#define FIFO_SIZE (1024 * 1024)

// Config parameters
#define CFG_SYMBOL_RATE "symbolRate"
#define CFG_FREQUENCY "frequency"
#define CFG_RRC_ALPHA "rrcAlpha"
#define CFG_MODE "mode"
#define CFG_SAMPLE_RATE "sampleRate"
#define CFG_DECIMATION "decimation"
#define CFG_AGC "agcEnabled"
#define CFG_MIXER_GAIN "mixerGain"
#define CFG_LNA_GAIN "lnaGain"
#define CFG_VGA_GAIN "vgaGain"
#define CFG_DEVICE_TYPE "deviceType"
#define CFG_FILENAME "filename"

#endif /* SRC_PARAMETERS_H_ */
