## How to build

The project could be build with [PlatformIO](https://platformio.org/). Before building make a hardware connection to SPI modules.

Steps to run:
 - Configuring and building firmware
 - Making a LittleFS image file with web resources
 - Uploading firmware
 - Uploading LittleFS image
 
### External Libs used to build/use the firmware
- [EmbUI](https://github.com/vortigont/EmbUI) framework is used to construct Web interface, manage WiFi, store configuration, etc
- [FTPClientServer](https://github.com/charno/FTPClientServer) to access ESP's file system (optional)


### Configuring and Building
First, copy default config file `infoclock/config.h.default` to `infoclock/config.h` and change default options. There are not so many things to change there.

To build fw for esp8266 run:
```sh
platformio run
```
To build fw with debug output, run:
```sh
platformio run -e debug
```
it will build fw with lot's of debug info being printer to the Hardware `Serial` port.

### Making a LittleFS image file with web resources
To handle WebUI it is required to build a LittleFS image and upload it to the controller. The image contains files from the [EmbUI](https://github.com/vortigont/EmbUI) framework and js/css files for the ESPEM project. The is no prebuild image since it is hard to maintain it, instead there is a shell script that downloads required files from github, repacks it and places under `/data` directory. That directory is used to create and upload LittleFS image to the controller.
Script is a linux shell, Windows users can use git-bash installed with [Git for Windows](https://git-scm.com/downloads)
 Run
```sh
cd resources
./respack.sh
```
It should populate `/data` dir with `js`, `css`, `index.html.gz`, etc...

Now the FS image and firmware could be uploaded to the controller
To upload LitlleFS image for ESP32 (until PIO esp32 core v2 release) it is required to use an uploader binary *mklittlefs*. Pls, download version for your OS fro
m [here](https://github.com/earlephilhower/mklittlefs/releases) and put the binary to the root dir of the project.

```sh
platformio run -t uploadfs
platformio run -t upload
```

That's it. Controller should reboot and enable WiFi.
