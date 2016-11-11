################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ChannelWriter.cpp \
../src/Display.cpp \
../src/newdecoder.cpp 

OBJS += \
./src/ChannelWriter.o \
./src/Display.o \
./src/newdecoder.o 

CPP_DEPS += \
./src/ChannelWriter.d \
./src/Display.d \
./src/newdecoder.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -I../../..//SatHelper/includes -I../../../SatHelper/includes/exceptions -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


