################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AirspyDevice.cpp \
../src/SampleFIFO.cpp \
../src/SymbolManager.cpp \
../src/demodulator.cpp 

OBJS += \
./src/AirspyDevice.o \
./src/SampleFIFO.o \
./src/SymbolManager.o \
./src/demodulator.o 

CPP_DEPS += \
./src/AirspyDevice.d \
./src/SampleFIFO.d \
./src/SymbolManager.d \
./src/demodulator.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I../../../SatHelper/includes -I../../../SatHelper/includes/exceptions -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


