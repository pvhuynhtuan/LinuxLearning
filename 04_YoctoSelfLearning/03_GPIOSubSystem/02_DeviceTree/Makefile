# Device Tree Overlay targets
DTS_FILE = gpiosubsystem.dts
DTBO_FILE = gpiosubsystem.dtbo

# Default target
all:
	dtc -@ -I dts -O dtb -o $(DTBO_FILE) $(DTS_FILE)

# Clean target
clean:
	rm -rf $(DTBO_FILE)
