# Soundnator

## Install

- download and burn armhf version of Raspberry OS
- download armv6 OpenFrameworks from https://openframeworks.cc/download/
- extract to Home and rename to "openFrameworks"

```
# on sdcard /bootfs

touch ssh

echo 'pi:$6$fQwSiHFktRr.TuI5$puVCTNrtU9ecaRbbKJthEvxtOKX9aJKv3oALDFpTouv8FMqNNEuA.Pw/nScb1xJYCEqIpyz4y3r3a6Q2.OIgR0' > userconf.txt

echo 'ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=BR

network={
    scan_ssid=1
    ssid="Toca da Raposa 2.4GHz"
    psk="coelhoraposa"
    proto=RSN
    key_mgmt=WPA-PSK
    pairwise=CCMP
    auth_alg=OPEN
}
' > wpa_supplicant.conf
```

```

sudo apt update
sudo apt upgrade

cd openFrameworks/addons
git clone https://github.com/cleissom/ofxTableGestures
git clone https://github.com/cleissom/ofx2DFigures
git clone https://github.com/chaosct/ofxGlobalConfig
git clone https://github.com/danomatika/ofxMidi
git clone https://github.com/npisanti/ofxPDSP
git clone https://github.com/npisanti/ofxAudioFile

cd -
cd openFrameworks/scripts/linux/debian
sudo ./install_dependencies.sh
sudo ./install_codecs.sh

cd -
cd openFrameworks/scripts/linux
./compileOF.sh -j1

cd -
cd openFrameworks/apps/Soundnator
make -j1
```

- fix error libopenmaxil.so: [link](https://forum.openframeworks.cc/t/of-not-working-with-raspberry-os-bullseye/38779/4)

### Cross-compiling

```

```

### ReacTIVision

```
sudo apt install libsdl2-dev libturbojpeg0-dev libdc1394-dev
git clone https://github.com/mkalten/reacTIVision.git
cd reacTIVision/linux/
make -j4
```

### Post install

- place .service files and enable
- disable blanking
