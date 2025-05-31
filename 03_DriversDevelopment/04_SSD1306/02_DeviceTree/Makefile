#sudo apt-get install gcc-aarch64-linux-gnu
# to build: dtc -@ -I dts -O dtb -o <name>.dtbo <name>.dts
# Copy to boot: sudo cp <name>.dtbo /boot/overlays/
# Add a line to /boot/firmware/config.txt: dtoverlay=<name>
# Reboot the Raspberry Pi: sudo reboot
# Device Tree Overlay targets
DTS_FILE = ssd1306.dts
DTBO_FILE = ssd1306.dtbo

# Default target
all:
	dtc -@ -I dts -O dtb -o $(DTBO_FILE) $(DTS_FILE)

# Clean target
clean:
	rm -rf $(DTBO_FILE)
