# Bee hive monitoring

This project monitors bee hive weight and internal environment, posting results to cloud.

# Set up development environment

## Install ESP-IDF

Install the latest stable ESP IDF as described in https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html. Quick instructions for Debian 11 Bullseye:

Install requisite packages and set Python 3 as default:

```
$ sudo apt install git wget flex bison gperf python python-pip python-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util python3 python3-pip python3-setuptools
$ sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
```

Download ESP IDF source and install toolchain under the project directory:

```
$ export PROJ_DIR="$HOME/projects/hobby/beegram" && export IDF_TOOLS_PATH="$PROJ_DIR/lib/bin/espressif" IDF_PATH="$PROJ_DIR/lib/src/esp-idf"
$ mkdir -p "$PROJ_DIR/lib/src" && cd "$PROJ_DIR/lib/src"
$ git clone -j8 -b release/v4.2 --recursive https://github.com/espressif/esp-idf.git
$ cd esp-idf
$ ./install.sh
$ . export.sh
```

Next time you are starting work on the project, run following commands:

```
$ export PROJ_DIR="$HOME/projects/hobby/beegram" && export IDF_TOOLS_PATH="$PROJ_DIR/lib/bin/espressif" IDF_PATH="$PROJ_DIR/lib/src/esp-idf"
$ . "$IDF_PATH/export.sh"
```

## Get the source

```
$ mkdir -p "$PROJ_DIR/src" && cd "$PROJ_DIR/src"
$ git clone git@github.com:DaStoned/beegram.git
```

## Configuration file for tmuxinator

Propping up the development environment automatically is quite simple with tmuxinator.

```
$ sudo apt install tmux tmuxinator
$ mkdir "$HOME/.tmuxinator"
$ vim "$HOME/.tmuxinator/beegram.yml"
```

Content of `$HOME/.tmuxinator/beegram.yml` should be something like this:

```
name: beegram
root: ~/projects/hobby/beegram

pre_window: export PROJ_DIR="$HOME/projects/hobby/beegram" && export IDF_TOOLS_PATH="$PROJ_DIR/lib/bin/espressif" IDF_PATH="$PROJ_DIR/lib/src/esp-idf" ESPBAUD=921600 && . "$IDF_PATH/export.sh"

windows:
  - code:
      layout: main-vertical
      panes:
        - cd src/beegram && git status
        - cd src/beegram
  - sdk:
      layout: main-vertical
      panes:
        - cd "$IDF_PATH" && git status
```
