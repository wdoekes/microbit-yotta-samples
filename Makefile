NAME=microbit-yotta-samples
COMBINED_HEX=microbit-yotta-samples-combined.hex
PLATFORM=bbc-microbit-classic-gcc
SOURCES=main.cpp
TARGET=build/$(PLATFORM)/$(NAME)/$(NAME)-combined.hex

.PHONY: all
all: select-target build
	@echo "Now do 'make upload' to upload the binary"

.PHONY: select-target
select-target: .yotta.json

.yotta.json:
	@if ! grep -xF '"$(PLATFORM)@https://github.com/lancaster-university/yotta-target-$(PLATFORM)"' .yotta.json; then \
	  yotta target $(PLATFORM)@https://github.com/lancaster-university/yotta-target-$(PLATFORM); \
	fi

.PHONY: build
build: $(TARGET)
	@echo "$(TARGET) is built"

$(TARGET): $(SOURCES)
	yotta build

.PHONY: clean
clean:
	yotta clean

.PHONY: fullclean
fullclean:
	$(RM) -r build yotta_modules yotta_targets .yotta.json

.PHONY: upload
upload:
	DEST=$$(sed -ne 's/^[^ ]* \(.*MICROBIT\) vfat .*/\1/p' < /proc/mounts) && \
	cp -v "$(TARGET)" "$$DEST/"
