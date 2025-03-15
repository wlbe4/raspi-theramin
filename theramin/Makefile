.PHONY: all clean
default: all

CFLAGS += -g -O3 -Wall -Werror
LDFLAGS += -lpthread -lrt
raspi_theramin: raspi_theramin.c Makefile
	@echo Compiling $@
	@$(CC) $< $(CFLAGS) $(LDFLAGS) -lasound -o $@

all: raspi_theramin

clean:
	@rm -f raspi_theramin
