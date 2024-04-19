# Universe

The universe is filled with multiple globally accessed variables.

This isn't a problem for single-threaded applications, because multiple process instances can be used to run the operation in parallel. However, for HPC programming, this is not an option. We ideally want to have a single host (CPU) process utilizing the hundreds of computation units in the device (GPU).

The quickest solution is to dump all of the global written-to variables into a Universe structure, to hold the state of the universe locally. Variables that are dynamically initilized once with the same values should NOT be in the universe and should use the `__global` OpenCL keyword. Constant variables should also NOT be in the universe and should be transformed into defines or enums, or use the `__constant` OpenCL keyword.
