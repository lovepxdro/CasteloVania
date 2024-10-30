TARGET = vania

CC = gcc

CFLAGS = -Wall -g

INCLUDES = -I/usr/local/include

LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11

SRCS = main.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
