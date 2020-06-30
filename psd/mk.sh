echo 'gcc -o read_signal read_signal.c -g -std=gnu99 -Wall -Werror -I../../api/include -L/opt/redpitaya/lib -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread -l:librp.so'
gcc -o read_signal read_signal.c -g -std=gnu99 -Wall -Werror -I/opt/redpitaya/include -I../../api/include -L/opt/redpitaya/lib -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread -l:librp.so


