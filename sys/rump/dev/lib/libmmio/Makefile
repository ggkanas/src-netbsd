RUMPTOP=${TOPRUMP}

.PATH:	${RUMPTOP}/../dev

LIB=	rumpdev_mmio
COMMENT=Memory-Mapped IO

IOCONF=	MMIO.ioconf
RUMP_COMPONENT=ioconf

# SRCS+=	mmio.c mmio-api.c

CPPFLAGS+= -I${RUMPTOP}/librump/rumpkern

.include "${RUMPTOP}/Makefile.rump"
.include <bsd.lib.mk>
.include <bsd.klinks.mk>
