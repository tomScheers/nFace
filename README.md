# nFace
nFace is a terminal-based camera application that converts your live camera feed into ASCII art in real-time using ncurses. 

![Video Preview](assets/example.gif)
## Requirements
- Linux (for v4l2)
- A camera (obviously)
- gcc
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

## Usage
You can run the project by executing the binary
```bash
bin/nface
```
Or, if you have installed it to your system, just run
```bash
nface
```
### N.B.
- You can change the resolution of the camera by increasing/decreasing your terminal size using tmux, because this program is responsive
