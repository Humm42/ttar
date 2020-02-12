.POSIX:

include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

TARG=ttar
OFILES=ttar.o util.o

all: ${TARG} config.mk

.c.o:
	${CC} -c ${CFLAGS} ${CPPFLAGS} $<

${TARG}: ${OFILES}
	${LD} -o${TARG} ${LDFLAGS} ${OFILES} ${LIBS}

clean:
	rm -f ${TARG} *.o
nuke: clean

install:
	cp ${TARG} ${DESTDIR}${BINDIR}
	chmod 555 ${DESTDIR}${BINDIR}/${TARG}
	cp ttar.1 ${DESTDIR}${MANDIR}/man1/ttar.1
	chmod 444 ${DESTDIR}${MANDIR}/man1/ttar.1

uninstall:
	rm -f ${DESTDIR}${BINDIR}/${TARG}
	rm -f ${DESTDIR}${MANDIR}/man1/ttar.1

.PHONY: all clean nuke install uninstall
