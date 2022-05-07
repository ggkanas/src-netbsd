RUMPTOP=${TOPRUMP}

.PATH:	${RUMPTOP}/../dev/virtio

LIB=	rumpdev_mmiocmdl
COMMENT=MMIO through CommandLine

IOCONF=	MMIOCMDL.ioconf
#RUMP_COMPONENT=ioconf

SRCS+=	mmiocmdl.c mmiocmdl_cfdata.c

CPPFLAGS+= -I${RUMPTOP}/librump/rumpkern

.include "${RUMPTOP}/Makefile.rump"
.include <bsd.lib.mk>
.include <bsd.klinks.mk>
