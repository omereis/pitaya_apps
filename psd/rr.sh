echo 'gcc -o fast_ai fpga_osc.o main_osc.o worker.o fast_ai.o -g -std=gnu99 -Wall -Werror -I../../api/include -DVERSION=0.0 -0000 -DREVISION=devbuild -lm -lpthread'

gcc -o read_signal -g -std=gnu99 -Wall -Werror -I../../api/include -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread

