# ASC
Microros on STM32 via Zephyr

## Setup Zephyr

Update your packages
```bash
sudo apt update
sudo apt upgrade
```

### Install dependencies¶
Next, you’ll install some host dependencies using your package manager.

The current minimum required version for the main dependencies are:
| Tool                | Min. Version |
| --------------------| -------------|
| CMake               | 3.20.0       |
| Python              | 3.8          |
| Devicetree compiler | 1.4.6        |


>1. If you are running a Ubuntu version OLDER than 22.04, you need to add extra repos.
>```bash
>wget https://apt.kitware.com/kitware-archive.sh
>sudo bash kitware-archive.sh
>```
>2. Use apt to install the required dependencies:
>```bash
>sudo apt install --no-install-recommends git cmake ninja-build gperf \
>  ccache dfu-util device-tree-compiler wget \
>  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
>  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1
>```
>3. Verify the versions of the main dependencies installed on your system by entering:
>```bash
>cmake --version
>python3 --version
>dtc --version
>```

### Get Zephyr and install Python dependencies¶

>1. Install west, and make sure ~/.local/bin is on your PATH environment variable:
>```bash
>pip3 install --user -U west
>echo 'export PATH=~/.local/bin:"$PATH"' >> ~/.bashrc
>source ~/.bashrc
>```
>2. Get the Zephyr source code:
>```bash
>mkdir ~/zephyrproject
>cd ~/zephyrproject
>git clone https://github.com/Bamelink/ASC.git
>west init .
>cd zephyr
>git checkout 4256cd4
> cd ..
>west update
>```
>3. Export a Zephyr CMake package. This allows CMake to automatically load boilerplate >code required for building Zephyr applications.
>```bash
>west zephyr-export
>```
>4. Zephyr’s scripts/requirements.txt file declares additional Python dependencies. >Install them with pip3.
>```bash
>pip3 install --user -r ~/zephyrproject/zephyr/scripts/requirements.txt
>```

### Install Zephyr SDK

>1. Download and verify the latest Zephyr SDK bundle:
>```bash
>cd ~
>wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.15.0/zephyr-sdk-0.15.0_linux-x86_64.tar.gz
>wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.15.0/sha256.sum | shasum --check --ignore-missing
>```
>2. Extract the Zephyr SDK bundle archive:
>```bash
>tar xvf zephyr-sdk-0.15.0_linux-x86_64.tar.gz
>```
>3. Run the Zephyr SDK bundle setup script:
>```bash
>cd zephyr-sdk-0.15.0
>./setup.sh
>```
>4. Install udev rules, which allow you to flash most Zephyr boards as a regular user:
>```bash
>sudo cp ~/zephyr-sdk-0.15.0/sysroots/x86_64-pokysdk-linux/usr/share/openocd/contrib/>60-openocd.rules /etc/udev/rules.d
>sudo udevadm control --reload
>```

### uROS dependencies
>For uROS, some additional pip packages are requiered
>```bash
>pip3 install catkin_pkg lark-parser empy colcon-common-extensions
>```


## Building
Navigate into the folder
```bash
cd ~/zephyrproject/autonomous-system-controller/Software
```
From here you can build via the following command
```bash
west build -b asc -p always application
```
Devicetreeoverlays or config with the right namingscheme (e.g. asc.overlay or asc.conf) in the applicaiton/boards/ directory will automaticly be compiled into the application

Some config options can also be changed via a terminal tool
```bash
west build -t menuconfig
```
After that, make sure to change -p always to -p auto. Otherwise it will do a complete rebuild and the changes are overwritten, since the changes apply to the build directory. If you want your changes to be permanen, add them to the prj.conf!

Lastly you can flash the applicaiton by just running
```bash
west flash
```
---
## uROS agent
To communicate with the microcontroller, you can run the microros agent via docker
```bash
sudo docker run -it --rm -v /dev:/dev --privileged --net=host microros/micro-ros-agent:foxy serial-usb --dev [YOUR BOARD PORT, e.g. /dev/ttyACM0]
```
You can also put the -v6 flag at the end to get a more detailed output of what is happening. If you want to see a topic, you can run the following commands
```bash
sudo docker run -dit -v /dev:/dev --name urosagent --privileged --net=host microros/micro-ros-agent:foxy serial-usb --dev [YOUR BOARD PORT, e.g. /dev/ttyACM0]

sudo docker exec -ti urosagent /bin/bash
```
With the exec command, you directly attach to the container console and execute ros2 commands. Be sure to delete the container after you are finished!
```bash
sudo docker stop urosagent
sudo docker rm urosagent
```
---
## Errors you can face
### Zephyr repo not found.
If you face the error that the Zephyr repo could not be found, make sure to source it
```bash
source ~/zephyrproject/zephyr/zephyr-env.sh
```
This is especially important, if you do not have cloned the autonomous-system-controller into the zephyrproject or zephyr folder!

### Posix not found
```c
/home/jan/zephyrproject/autonomous-system-controller/Software/application/modules/libmicroros/micro_ros_src/src/rcutils/src/time_unix.c:33:10: fatal error: posix/time.h: No such file or directory
   33 | #include <posix/time.h>  //  Points to Zephyr toolchain posix time implementation
      |          ^~~~~~~~~~~~~~
```
Here a file has to change its include path. Open the  file "modules/libmicroros/micro_ros_src/src/rcutils/src/time_unix.c" and change
```c
#include <posix/time.h>  
```
to
```c
#include <zephyr/posix/time.h>
```

### Undifined Reverence to devicetree_ord_xx
If you face a message like "Undifined Reverence to devicetree_ord_19", you most likely have accessed a device from devicetree which got referenced in your code by something like "DEVICE_DT_GET(DT_ALIAS(scan))".

This will fail if the device is not configured properly in the Devicetree or the coresponding driver is not turned on in the prj.conf. Examples for this would be you want to access a CAN device but the device has status = "disabled"; in the devicetree or CONFIG_CAN=y is not included in the prj.conf.