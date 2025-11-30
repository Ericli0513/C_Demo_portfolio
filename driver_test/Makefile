CC = gcc
CFLAGS = -Wall -std=c11
TARGET = driver_test_c_demo
SRCS = main.c driver_test.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) 
