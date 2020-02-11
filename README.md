# Bela Patch 
a collection of patches for the various variations of BELA by bela.io - https://bela.io

Bela is a 'addon' board for the BeagleBone Black/Green,  providing low latency audio, analog and digial i/o

BELA   - just the plain Bela board
SALT   - a flexible Eurorack module made in collaboration with Rebel Technology
PEPPER - a flexible DIY Eurorack module this is passive so much cheaper than SALT but with limitations



These are patches Ive made for my own use, or may have adapted from elsewhere
note: they may or may not be up to date, or working ;) 


# initial setup 
clone the repository into a directory
```
mkdir ~/projects
cd ~/projects
git clone https:https:/github.com/thetechnobear/BelaPatches
cd BelaPatches
```

Some patches (particulary my C++ ones), will need to have external libraries built, so we need to pull the external submodules

```
git submodule update --init --recursive 
```

now install some dev libraries we will need to build
```
apt-get install libusb-1.0-0-dev
apt-get install libasound2-dev
apt-get install libcairo2-dev 
```

now we can build requisite libraries etc

```
mkdir build
cd build
cmake -DBELA=1 ..
make
```

all these projects are configured to expect a symbolic link to the external projects in the projects directory
to achieve this
```
cd ~/Bela/projects
ln -s ~/projects/BelaPatches/external .
ln -s ~/projects/BelaPatches/build/release/lib .
```


now bela projects are able to use these libraries

note: if you want to distribute as standalone, you will need to copy libraries and alter the settings.json for the bela project, since they refer to the build area
