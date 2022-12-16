# RNBO template


this worked is based on the work done by @giuliomoro


more details here: 

https://github.com/giuliomoro/rnbo.example.bela

https://learn.bela.io/using-bela/languages/rnbo/



# How to use

there are two steps to use these templates.



## Initial Setup 
intially generate an RNBO C++ export, it can be completely empty rnbo, 
we just want the rnbo template code... we are not interested in the rnbo_source.cpp


so once generated copy the rnbo subfolder of this export to your bela to `~/Bela/rnbo`


this is a one-off step, and only needs to be repeated if RNBO is updated.

## Creating new bela projects


use **this** folder contents as a new project for bela.
you can simple zip this folder, and drop onto IDE if you wish.


export rnbo patch
export to rnbo_source.cpp, then simply replace this file in the project.

if you have the project oopen in the IDE, you can simply drag n' drop  to replace.



# Differences with rnbo.example.bela

the only difference in my approach is to use a shared rnbo folder, and then to make use of RNBO.cpp.
rather than include the rnbo subfoler into the bela project, which will then compile it.
the issue I found with the subfolder approach is it cannot use RNBO.cpp to decide what to build, 
which is the approach intended by C74, and this caused compilation issues when upgrading RNBO.

my approach avoids this issue, not only for current release, but also future releases (hopefully ;)) 


the rest is the same...

in particular the main work, which was the creation of render.cpp to interace rnbo->bela is completely unchanged.
SO if this is improved and updated in the rnbo.example.bela repo, you can simply drop this into the project.


 # Templates

basic - a vanilla template from rnbo.example.bela
salt - a template specialised for Bela Salt , including 'starter' rnbo patch
