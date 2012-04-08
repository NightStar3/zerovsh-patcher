cd user
make clean
make
../tools/_bin2c zerovsh_upatcher.prx zerovsh_upatcher.h zerovsh_user_module
cp zerovsh_upatcher.h ../kernel
rm -Rf *.prx *.elf zerovsh_upatcher.h
cd ../kernel
make clean
make
cp zerovsh_patcher.prx ../bin
rm -Rf *.prx *.elf zerovsh_upatcher.h
cd ../