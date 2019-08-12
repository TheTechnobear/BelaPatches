# mec-bela
bela project interfacing to eigenlite

#building and running

##pre-requisites
the eigenlite project

    cd ~
    mkdir dev
    git clone http://github.com/TheTechnobear/EigenLite.git
    mkdir build
    cd build
    cmake ..
    make
    
#status
work in progress!



#notes


makefile flags for this project

    AT=;LDFLAGS=-L/root/dev/EigenLite/build/release/lib;LDLIBS=-leigenapi -Wl,-rpath,/root/dev/EigenLite/build/release/lib
