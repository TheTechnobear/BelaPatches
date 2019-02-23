# splite-bela
bela project interfacing to splite

#building and running

##pre-requisites
the mec project

    cd ~
    mkdir dev
    git clone http://github.com/TheTechnobear/SoundplaneLite.git
    git submodule update --init --recursive
    mkdir build
    cd build
    cmake ..
    make
    
#status
work in progress!
to do

#notes


makefile flags for this project

       AT=;CPPFLAGS=-I/root/projects/SoundplaneLite/splite;LDFLAGS=-L/root/projects/SoundplaneLite/build/release/lib;LDLIBS=-lsplite -Wl,-rpath,/root/projects/SoundplaneLite/build/release/lib"

