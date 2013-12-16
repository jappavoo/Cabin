ARDUINO_DIR  = /Users/jonathan/Applications/Arduino.app/Contents/Resources/Java
AVR_TOOLS_DIR = $(ARDUINO_DIR)/hardware/tools/avr
AVRDUDE_CONF = $(ARDUINO_DIR)/hardware/tools/avr/etc/avrdude.conf
ARDMK_DIR    = /Users/jonathan/local/share/Arduino-Makefile
BOARD_TAG    = uno
MONITOR_PORT = /dev/tty.usbmodemfd121

USER_LIB_PATH := $(realpath ../Libraries)
ARDUINO_LIBS = SoftwareSerial OneWire DallasTemperature ReedSwitch


include $(ARDMK_DIR)/arduino-mk/Arduino.mk

