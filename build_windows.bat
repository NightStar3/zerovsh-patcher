cd user
make clean
make
../tools/_bin2c zerovsh_upatcher.prx zerovsh_upatcher.h zerovsh_user_module
copy zerovsh_upatcher.h ../kernel
del -Rf *.prx *.elf zerovsh_upatcher.h
cd ../kernel
make clean
make
copy zerovsh_patcher.prx ../bin
del -Rf *.prx *.elf zerovsh_upatcher.h
cd ../