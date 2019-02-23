# Salt Patches
a collection of patches for SALT by bela.io 
a flexible Eurorack module based on the BeagleBone Black and Bela board. made in collaboration with Rebel Technology
https://bela.io

These are patches Ive made for my own use, or may have adapted from elsewhere (see credits below)
note: they may or may not be up to date, or working ;) 


### How I use these patches

I clone the repository into a directory e.g.
```
mkdir ~/projects
cd ~/projects
git clone https:https:/github.com/thetechnobear/salt_patches
```
then I create a link in the bela projects to patches i want to use 
```
cd ~/Bela/Projects
ln -s ~/projects/salt_patches/sc/splite
```

if you want you can rename, e.g. to include as a loop 
```
cd ~/Bela/Projects
ln -s ~/projects/salt_patches/sc/splite loop_1_splite
```


### Credits
the following patches have been based on work from other open source projects, 
I thank them for their work, and for making them available to the wider community.

### 3rd Party Patches
(these may either be straight copies I keep for convience of may have been adapted for my needs ;) ) 
granular-live : by Hyppasus , can be found at https://github.com/Hyppasus/supercollider-eurobela

### My Patches
These patches use open souce code (usually C++) that Ive then integrated into my own projects and patches
splite : uses Madrona Labs code for the soundplane, note: it is depend on my SoundplaneLite project

