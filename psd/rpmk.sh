#echo 'gcc -o rp_read rp_read.c fpga_osc.c -g -Wall -Werror -I/opt/redpitaya/include -I../../api/include -L/opt/redpitaya/lib -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread -l:librp.so'
#gcc -o rp_read rp_read.c fpga_osc.c -g -Wall -Werror -I/opt/redpitaya/include -I../../api/include -L/opt/redpitaya/lib -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread -l:librp.so
echo 'gcc -o rp_read rp_read.c -g -Wall -Werror -I/opt/redpitaya/include -I../../api/include -L/opt/redpitaya/lib -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread -l:librp.so'
gcc -o rp_read rp_read.c -g -Wall -Werror -I/opt/redpitaya/include -I../../api/include -L/opt/redpitaya/lib -DVERSION=0.00-0000 -DREVISION=devbuild -lm -lpthread -l:librp.so


