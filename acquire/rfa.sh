echo $'gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild fast_ai.c -o fast_ai.o'
gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild fast_ai.c -o fast_ai.o

echo $'gcc -o fast_ai fpga_osc.o main_osc.o worker.o fast_ai.o -g -std=gnu99 -Wall -Werror -I../../api/include -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread'
gcc -o fast_ai fpga_osc.o main_osc.o worker.o fast_ai.o -g -std=gnu99 -Wall -Werror -I../../api/include -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread
