RUMPTOP=${TOPRUMP}

.PATH:	${RUMPTOP}/../arch/amd64/amd64/mmiocmdl

LIB=	rumpdev_mmiocmdl
COMMENT=MMIO through CommandLine

IOCONF=	MMIOCMDL.ioconf
#RUMP_COMPONENT=ioconf

SRCS+=	mmiocmdl.c mmiocmdl_cfdata.c

.PATH:	${RUMPTOP}/../arch/x86/x86

.PATH:	${RUMPTOP}/../dev

SRCS+= mmio-api.c mmio.c 

.PATH:	${RUMPTOP}/../dev/virtio

SRCS+= virtio_mmio.c

CPPFLAGS+= -I${RUMPTOP}/librump/rumpkern

.include "${RUMPTOP}/Makefile.rump"
.include <bsd.lib.mk>
.include <bsd.klinks.mk>
