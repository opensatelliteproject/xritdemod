
all: decoder demodulator

clean:
	@echo -e '\033[0;32mCleaning target Debug\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C decoder/build clean
	$(MAKE) -C demodulator/build clean
	rm -fr demodulator/build
	rm -fr decoder/build
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished cleaning Debug\033[0m'

decoder: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@mkdir decoder/build -p
	@cd decoder/build && cmake ..
	$(MAKE) -C decoder/build
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

demodulator: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@mkdir demodulator/build -p
	@cd demodulator/build && cmake ..
	$(MAKE) -C demodulator/build
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

libcorrect: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@git clone https://github.com/quiet/libcorrect/
	@mkdir libcorrect/build -p
	@cd libcorrect/build && cmake ..
	$(MAKE) -C libcorrect/build
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

libcorrect-install: FORCE
	@echo -e '\033[0;32mInstalling target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C libcorrect/build install
	@ldconfig
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished installing target: $@\033[0m'
	@echo ' '

libSatHelper: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@git clone https://github.com/opensatelliteproject/libsathelper/
	$(MAKE) -C libsathelper
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

librtlsdr: FORCE
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	@git clone https://github.com/librtlsdr/librtlsdr
	@mkdir librtlsdr/build -p
	@cd librtlsdr/build && cmake ..
	$(MAKE) -C librtlsdr/build
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

libSatHelper-install: FORCE
	@echo -e '\033[0;32mInstalling target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C libsathelper install
	@ldconfig
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished installing target: $@\033[0m'
	@echo ' '

librtlsdr-install: FORCE
	@echo -e '\033[0;32mInstalling target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C librtlsdr/build install
	@echo -e 'Running ldconfig'
	@ldconfig
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished installing target: $@\033[0m'
	@echo ' '

test:
	@echo -e '\033[0;32mNothing to test\033[0m'

FORCE: