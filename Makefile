obj-m += add.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o prod producer.c
	gcc -o con consumer.c
      

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
