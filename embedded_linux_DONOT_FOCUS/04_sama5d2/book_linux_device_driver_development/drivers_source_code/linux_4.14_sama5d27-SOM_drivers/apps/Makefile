all: ioctl_test

app : ioctl_test.c
	$(CC) -o $@ $^
clean :
	rm ioctl_test
deploy : ioctl_test
	scp $^ root@10.0.0.10:/home/root/
