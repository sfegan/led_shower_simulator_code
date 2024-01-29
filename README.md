Build instructions

1. mkdir build
2. cd build
3. export PICO_SDK_PATH=/Users/sfegan/RaspberryPi/pico/pico-sdk
4. cmake ..
5. cd flasher
6. make -j4
7. Hold button on PICO and connect USB power until device mounted in mass-storage mode
8. cp flasher.uf2 /Volumes/RPI-RP2
9. screen /dev/tty.usbmodem141101
