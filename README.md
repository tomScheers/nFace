# nFace
nFace is a terminal-based camera application that converts your live camera feed into ASCII art in real-time using ncurses. 

This is one of my favorite projects I've ever created, so try it out and have some fun with it!

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
```bash
bin/nface
```

(optional) install it to your system
```bash
sudo make install
```
And run it from anywhere
```bash
nface
```
