gcc -O2 -s -march=pentium4 -mtune=pentium4 -finline-functions -fomit-frame-pointer -fno-strength-reduce -pipe -Wall -W -o dbdump -I../../include dbdump.c -L../../lib -lcdb
gcc -O2 -s -march=pentium4 -mtune=pentium4 -finline-functions -fomit-frame-pointer -fno-strength-reduce -pipe -Wall -W -o dfgen -I../../include dfgen.c -L../../lib -lcdb
