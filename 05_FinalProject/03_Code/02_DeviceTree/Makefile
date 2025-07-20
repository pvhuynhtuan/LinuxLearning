#sudo apt-get install gcc-aarch64-linux-gnu
#sudo apt-get install device-tree-compiler
# to build: dtc -@ -I dts -O dtb -o <name>.dtbo <name>.dts
# Copy to boot: sudo cp <name>.dtbo /boot/overlays/
# Add a line to /boot/firmware/config.txt: dtoverlay=<name>
# Reboot the Raspberry Pi: sudo reboot
# Device Tree Overlay targets
DTS_FILE1 = ili9341.dts
DTBO_FILE1 = ili9341.dtbo
DTS_FILE2 = fb_ili9341.dts
DTBO_FILE2 = fb_ili9341.dtbo

# Default target
all:
	dtc -@ -I dts -O dtb -o $(DTBO_FILE1) $(DTS_FILE1)
	dtc -@ -I dts -O dtb -o $(DTBO_FILE2) $(DTS_FILE2)

install1:
	sudo cp ./$(DTBO_FILE1) /boot/overlays/

install2:
	sudo cp ./$(DTBO_FILE2) /boot/overlays/

# Clean target
clean:
	rm -rf $(DTBO_FILE1)
	rm -rf $(DTBO_FILE2)
