# Shared Memory Utility
This is the only program which creates and destroys shared memory regions. It uses the global
definitions contained in tco_shmem to accomplish this.

## Training Memory
To avoid allocating shared memory for the region which holds simulator state while training, the
```build.sh``` script must be run with ```-DTRAINING``` to enable the sim memory to be created and
destroyed (and its associated semaphore), otherwise it will not be created.
