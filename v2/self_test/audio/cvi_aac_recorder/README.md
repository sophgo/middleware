1. Build
add libfdkaac_enc.so to aac_lib/64bit or aac_lib/32bit
make clean & make

2. Run
eg.
./cvi_aac_recorder -c 1 -s 8000 -p 1024 -b 64000 -m 1 out.aac
./cvi_aac_recorder -c 1 -s 8000 -p 1024 -b 64000 -m 0 inout_file.raw out.aac
