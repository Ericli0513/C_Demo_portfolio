CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
TARGET = sys_driver_test
SRCS = main.c sys_test.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) logs.txt
