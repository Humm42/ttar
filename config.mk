PREFIX=/usr/local
BINDIR=${PREFIX}/bin
MANDIR=${PREFIX}/share/man

LIBS=

#CPPFLAGS=-DNDEBUG -D_POSIX_C_SOURCE=200809L
#CFLAGS=-std=c99 -Wall -pedantic -Os -flto
#LDFLAGS=-O3 -flto -s
CPPFLAGS=-D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c99 -Wall -Wextra -pedantic -O0 -g
LDFLAGS=-g

CC=c99
LD=${CC}
