PRG=855resolution

#Uncomment this line if you want to test with a VBIOS image file
#VBIOS_FILE:=-DVBIOS_FILE='"vbios.dmp"'

PLUGINS_SRCS:=${shell ls plugins/*.c}
SRCS=855resolution.c vbios.c ${PLUGINS_SRCS}
OBJS=${SRCS:.c=.o}

PLUGINS_LIST:=${shell cd plugins;ls -x *.c | sed -e 's/.c//g' -e 's/  */,/g' }
PLUGINS_ADDR:=${shell echo ${PLUGINS_LIST} | sed -e 's/^/\&/g' -e 's/,/,\&/g' }

CFLAGS:=-s -DPLUGINS='${PLUGINS_LIST}' -DREF_PLUGINS='${PLUGINS_ADDR}' ${VBIOS_FILE}

${PRG}: ${OBJS}

clean:
	rm -f ${OBJS} ${PRG} *~

install: ${PRG}
	cp ${PRG} /usr/sbin
dump:
	# Create a VBIOS image file
	dd if=/dev/mem of=vbios.dmp bs=16384 skip=48 count=4
