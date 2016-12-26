

all: demodulator-debug demodulator-release demodulator

clean:
	@echo -e '\033[0;32mCleaning target Debug\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C Debug clean
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished cleaning Debug\033[0m'
	@echo -e '\033[0;32mCleaning target Release\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C Release clean
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished cleaning Release\033[0m'
	@echo -e '\033[0;32mCleaning target Release (Static)\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C "Release Static" clean
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished cleaning target Release (Static)\033[0m'

demodulator-debug:
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C Debug
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

demodulator-release:
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C Release
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '


demodulator:
	@echo -e '\033[0;32mBuilding target: $@\033[0m'
	@echo -e '\033[0;34m'
	$(MAKE) -C "Release Static"
	@echo -e '\033[0m'
	@echo -e '\033[0;32mFinished building target: $@\033[0m'
	@echo ' '

test:
	@echo 'No tests configured'