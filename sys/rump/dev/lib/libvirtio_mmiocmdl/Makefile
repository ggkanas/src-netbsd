RUMPTOP=${TOPRUMP}

.PATH:	${RUMPTOP}/../dev/virtio

LIB=	rumpdev_virtio_mmiocmdl
COMMENT=VirtIO-MMIO through CommandLine

IOCONF=	VIRTIO_MMIOCMDL.ioconf
RUMP_COMPONENT=ioconf

SRCS+=	virtio_mmiocmdl.c virtio_mmio.c

CPPFLAGS+= -I${RUMPTOP}/librump/rumpkern

.include "${RUMPTOP}/Makefile.rump"
.include <bsd.lib.mk>
.include <bsd.klinks.mk>
