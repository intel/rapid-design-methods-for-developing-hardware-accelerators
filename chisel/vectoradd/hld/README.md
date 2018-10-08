# Preliminary instruction to run ASE using the OPAE platform

Build the ASE simulation for this block
````code
afu_sim_setup --source rtl/sources.txt build_sim --platform intg_xeon --tool VCS
````
Make sure the the OPAE install bin directory is in your path.
For me this is:
````
modpath /nfs/pdx/home/smburns/ppt_work_area/OPAE_INSTALL/bin
````
(You might need to change a few paths in `rtl/sources.txt`. I'll work on fixing this eventually.)

Make the simulator
````code
cd build_sim
make
````

Create a new window to run the software
````code
xterm &
````

Start the simulator
````code
make sim
````

In the new window, set work directory through an environment variable. The command that you need to issue is printed in the console.
For me, this is:
````code
setenv ASE_WORKDIR /nfs/site/disks/scl.work.37/ppt/users/smburns/hld/chisel/vectoradd/hld/build_sim/work
````

Build the software and run
````code
cd ../vectoradd_standalone
make && ./vectoradd_ase
````
Make sure that the OPAE install's lib directory (and other dependent directories are in your library path):
For me, this is:
````code
setenv LD_LIBRARY_PATH "/nfs/site/disks/scl.work.37/ppt/users/smburns/OPAE_INSTALL/lib64:/usr/intel/pkgs/boost/1.67.0/lib"
````
