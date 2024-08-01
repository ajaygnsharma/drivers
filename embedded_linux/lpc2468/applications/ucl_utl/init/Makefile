
EXEC1 = parse
OBJS1 = parse.o

BINDIR = /sbin

CFLAGS += -g

all: $(EXEC1)

$(EXEC1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBCRYPT) $(LDLIBS$(LDLIBS_$@))
	cp parse


romfs:
	$(ROMFSINST) -e CONFIG_USER_INIT_EXPAND /bin/$(EXEC2)
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT /bin/$(EXEC1)
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_FACTORY_FACTTEST \
			-a "facttest:unknown:/bin/facttest" /etc/inittab
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_BUSYBOX_SYSLOGD \
			-a "slog:unknown:${BINDIR}/syslogd -n" /etc/inittab
	$(ROMFSINST) -e CONFIG_USER_INIT_INIT -e CONFIG_USER_BUSYBOX_KLOGD \
			-a "klog:unknown:${BINDIR}/klogd -n" /etc/inittab

clean:
	rm -f $(EXEC1) *.elf *.gdb *.o

