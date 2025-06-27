# nFace

nFace is a terminal-based camera application that converts your live camera feed
into ASCII art in real-time using ncurses.

![Video Preview](assets/example.gif)

## Requirements

- Linux (for v4l2)
- A camera (obviously)
- gcc
- gzip
- ncurses library
- tmux (optional but recommended)

## Installation Process

First you'll have to clone the repo from Github

```bash
git clone https://github.com/tomScheers/nFace.git
cd nFace
```

Secondly, you'll have to run make

```bash
make
```

To run the program, you'll need to run

(optional) install it to your system

```bash
sudo make install
```

### For nix users

You can install all necessary dependencies via a development shell

```bash
nix develop
```

## Usage

You can run the project by executing the binary

```bash
bin/nface
```

Or, if you have installed it to your system, just run

```bash
nface
```

### For nix users

You can run the program without any prior installations with nix

```bash
nix run github:tomScheers/nFace#default
```

### N.B.

- You can change the resolution of the camera by increasing/decreasing your
  terminal size using tmux, because this program is responsive
