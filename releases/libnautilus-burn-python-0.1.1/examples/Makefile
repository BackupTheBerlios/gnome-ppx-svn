PACKAGES = gtk+-2.0 libnautilus-burn
CFLAGS = `pkg-config --cflags $(PACKAGES)`
LDFLAGS = `pkg-config --libs $(PACKAGES)` 

all: bacon_test cd_drive_test

bacon_test: bacon_test.o
	$(CC) $(LDFLAGS) $^ -o $@

cd_drive_test: cd_drive_test.o
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f bacon_test cd_drive_test *.o
