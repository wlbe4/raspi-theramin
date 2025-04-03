.PHONY: all clean
default: all

CFLAGS += -g -O3 -Wall -Werror
LDFLAGS += -lpthread -lrt
raspi_uart: raspi_uart.c Makefile
	@echo Compiling $@
	@$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

all: raspi_uart

clean:
	@rm -f raspi_uart
