echo $'gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild read_test.c -o read_test.o'
gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild read_test.c -o read_test.o

echo $'gcc -o read_test fpga_osc.o main_osc.o worker.o read_test.o -g -std=gnu99 -Wall -Werror -I../../api/include -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread'
gcc -o read_test fpga_osc.o main_osc.o worker.o read_test.o -g -std=gnu99 -Wall -Werror -I../../api/include -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread
