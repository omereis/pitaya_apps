echo $'gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild main_osc.c -o main_osc.o'
gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild main_osc.c -o main_osc.o

echo $'gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild worker.c -o worker.o'
gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild worker.c -o worker.o

echo $'gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild fpga_osc.c -o fpga_osc.o'
gcc -c -g -std=gnu99 -Wall -Werror -I../../api/include -I/opt/redpitaya/include -DVERSION=0.00-0000 -DREVISION=devbuild fpga_osc.c -o fpga_osc.o


