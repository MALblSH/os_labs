TARGET = thread-example
SRCS = thread_a.c

CC=gcc
RM=rm
CFLAGS= -Wall -Wextra -Werror
LIBS=-lpthread

all: ${TARGET}

${TARGET}: ${SRCS}
	${CC} ${CFLAGS} ${SRCS} ${LIBS} -o ${TARGET}

clean:
	${RM} -f *.o ${TARGET}
