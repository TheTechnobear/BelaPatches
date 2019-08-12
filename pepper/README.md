# Pepper Patches
a collection of patches for Pepper by bela.io 


### Credits
the following patches have been based on work from other open source projects, 
I thank them for their work, and for making them available to the wider community.

### 3rd Party Patches
(these may either be straight copies I keep for convience of may have been adapted for my needs ;) ) 
granular-live : by Hyppasus , can be found at https://github.com/Hyppasus/supercollider-eurobela

### My Patches
These patches use open souce code (usually C++) that Ive then integrated into my own projects and patches
splite : uses Madrona Labs code for the soundplane, note: it is depend on my SoundplaneLite project


### Assumption
generally these patches will be for a standard pepper board implementation 
this means the LED segement bar, and setup as 8 analog input/output
any variation to this will be added as a comment in the patch


### How I use these patches



### How I use these patches

I create a link in the bela projects to patches i want to use 
```
cd ~/Bela/Projects
ln -s ~/projects/BelaPatches/pepper/c++/splite
```

if you want you can rename, e.g. to include as a loop 
```
cd ~/Bela/Projects
ln -s ~/projects/BelaPatches/pepper/c++/splite loop_1_splite
```




