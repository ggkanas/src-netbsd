/*	$NetBSD: virtio_mmio.c,v 1.7 2021/10/22 02:57:23 yamaguchi Exp $	*/
/*	$OpenBSD: virtio_mmio.c,v 1.2 2017/02/24 17:12:31 patrick Exp $	*/

/*
 * Copyright (c) 2014 Patrick Wildt <patrick@blueri.se>
 * Copyright (c) 2012 Stefan Fritsch.
 * Copyright (c) 2010 Minoura Makoto.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: virtio_mmio.c,v 1.7 2021/10/22 02:57:23 yamaguchi Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/mutex.h>

#define VIRTIO_PRIVATE
#include <dev/virtio/virtio_mmiovar.h>

#define VIRTIO_MMIO_MAGIC		('v' | 'i' << 8 | 'r' << 16 | 't' << 24)


/* Legacy Defines */
#define VIRTIO_MMIO_MAGIC_VALUE		0x000
#define VIRTIO_MMIO_VERSION		0x004
#define VIRTIO_MMIO_DEVICE_ID		0x008
#define VIRTIO_MMIO_VENDOR_ID		0x00c
#define VIRTIO_MMIO_HOST_FEATURES	0x010
#define VIRTIO_MMIO_HOST_FEATURES_SEL	0x014
#define VIRTIO_MMIO_GUEST_FEATURES	0x020
#define VIRTIO_MMIO_GUEST_FEATURES_SEL	0x024
#define VIRTIO_MMIO_GUEST_PAGE_SIZE	0x028
#define VIRTIO_MMIO_QUEUE_SEL		0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX	0x034
#define VIRTIO_MMIO_QUEUE_NUM		0x038
#define VIRTIO_MMIO_QUEUE_ALIGN		0x03c
#define VIRTIO_MMIO_QUEUE_PFN		0x040
#define VIRTIO_MMIO_QUEUE_NOTIFY	0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS	0x060
#define VIRTIO_MMIO_INTERRUPT_ACK	0x064
#define VIRTIO_MMIO_STATUS		0x070
#define VIRTIO_MMIO_CONFIG		0x100

#define VIRTIO_MMIO_INT_VRING		(1 << 0)
#define VIRTIO_MMIO_INT_CONFIG		(1 << 1)

/* 1.1 Defines */

#define VIRTIO_MMIO_1_MAGIC_VALUE			0x000
#define VIRTIO_MMIO_1_VERSION				0x004
#define 	VIRTIO_MMIO_1_VERSION_0		0x01
#define 	VIRTIO_MMIO_1_VERSION_1		0x02
#define VIRTIO_MMIO_1_DEVICE_ID				0x008
#define VIRTIO_MMIO_1_VENDOR_ID				0x00c
#define VIRTIO_MMIO_1_DEVICE_FEATURES		0x010
#define VIRTIO_MMIO_1_DEVICE_FEATURES_SEL	0x014
#define VIRTIO_MMIO_1_DRIVER_FEATURES		0x020
#define VIRTIO_MMIO_1_DRIVER_FEATURES_SEL	0x024
#define VIRTIO_MMIO_1_QUEUE_SEL				0x030
#define VIRTIO_MMIO_1_QUEUE_NUM_MAX			0x034
#define VIRTIO_MMIO_1_QUEUE_NUM				0x038
#define VIRTIO_MMIO_1_QUEUE_READY			0x044
#define VIRTIO_MMIO_1_QUEUE_NOTIFY			0x050
#define VIRTIO_MMIO_1_INTERRUPT_STATUS		0x060
#define 	VIRTIO_MMIO_1_INT_USED_BUFFER 		(1 << 0)
#define 	VIRTIO_MMIO_1_INT_CONFIG	  		(1 << 1)
#define VIRTIO_MMIO_1_INTERRUPT_ACK			0x064
#define VIRTIO_MMIO_1_STATUS				0x070
#define VIRTIO_MMIO_1_QUEUE_DESC_LOW		0x080
#define VIRTIO_MMIO_1_QUEUE_DESC_HIGH		0x084
#define VIRTIO_MMIO_1_QUEUE_DRIVER_LOW		0x090
#define VIRTIO_MMIO_1_QUEUE_DRIVER_HIGH		0x094
#define VIRTIO_MMIO_1_QUEUE_DEVICE_LOW		0x0a0
#define VIRTIO_MMIO_1_QUEUE_DEVICE_HIGH		0x0a4
#define VIRTIO_MMIO_1_CONFIG_GENERATION		0x0fc
#define VIRTIO_MMIO_1_CONFIG				0x100

/*
 * MMIO configuration space for virtio-mmio v1 is in guest byte order.
 *
 * XXX Note that aarch64eb pretends to be little endian. the MMIO registers
 * are in little endian but the device config registers and data structures
 * are in big endian; this is due to a bug in current Qemu.
 */

#if defined(__aarch64__) && BYTE_ORDER == BIG_ENDIAN
#	define READ_ENDIAN	LITTLE_ENDIAN
#	define STRUCT_ENDIAN	BIG_ENDIAN
#elif BYTE_ORDER == BIG_ENDIAN
#	define READ_ENDIAN	BIG_ENDIAN
#	define STRUCT_ENDIAN	BIG_ENDIAN
#else
#	define READ_ENDIAN	LITTLE_ENDIAN
#	define STRUCT_ENDIAN	LITTLE_ENDIAN
#endif


static void	virtio_mmio_kick(struct virtio_softc *, uint16_t);
static uint16_t	virtio_mmio_read_queue_size(struct virtio_softc *, uint16_t);
static void	virtio_mmio_setup_queue(struct virtio_softc *, uint16_t, uint64_t);
static void	virtio_mmio_set_status(struct virtio_softc *, int);
static uint64_t	virtio_mmio_negotiate_features(struct virtio_softc *, uint64_t);
static int	virtio_mmio_alloc_interrupts(struct virtio_softc *);
static void	virtio_mmio_free_interrupts(struct virtio_softc *);
static int	virtio_mmio_setup_interrupts(struct virtio_softc *, int);

static const struct virtio_ops virtio_mmio_ops = {
	.kick = virtio_mmio_kick,
	.read_queue_size = virtio_mmio_read_queue_size,
	.setup_queue = virtio_mmio_setup_queue,
	.set_status = virtio_mmio_set_status,
	.neg_features = virtio_mmio_negotiate_features,
	.alloc_interrupts = virtio_mmio_alloc_interrupts,
	.free_interrupts = virtio_mmio_free_interrupts,
	.setup_interrupts = virtio_mmio_setup_interrupts,
};

static uint16_t
virtio_mmio_read_queue_size(struct virtio_softc *vsc, uint16_t idx)
{
	struct virtio_mmio_softc *sc = (struct virtio_mmio_softc *)vsc;
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_1_QUEUE_SEL, idx);
	return bus_space_read_4(sc->sc_iot, sc->sc_ioh,
	    VIRTIO_MMIO_1_QUEUE_NUM_MAX);
}

static void
virtio_mmio_setup_queue(struct virtio_softc *vsc, uint16_t idx, uint64_t addr)
{
	struct virtio_mmio_softc *sc = (struct virtio_mmio_softc *)vsc;
	struct virtqueue *vq = &vsc->sc_vqs[idx];
	bus_space_tag_t	   iot = sc->sc_iot;
	bus_space_handle_t ioh = sc->sc_ioh;
	KASSERT(vq->vq_index == idx);
	uint32_t ready, num;
	uint64_t desc, avail, used;

	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_SEL, idx);
	ready = bus_space_read_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_READY);
	if (ready) {
		return;
	}
	num = bus_space_read_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_NUM_MAX);

	if (num == 0x0) return;
	if (vq->vq_num < num && vq->vq_num != 0) num = vq->vq_num;

	/* virtqueue4: allocate & zero memory. this should already have been handled by
	 * device-specific code 
	 */

	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_NUM, num);

	desc = (uint64_t) vq->vq_desc;
	avail = (uint64_t) vq->vq_avail;
	used = (uint64_t) vq->vq_used;
	
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_DESC_LOW, 
		desc & 0xffffffff);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_DESC_HIGH, 
		desc >> 32);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_DRIVER_LOW, 
		avail & 0xffffffff);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_DRIVER_HIGH, 
		avail >> 32);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_DEVICE_LOW, 
		used & 0xffffffff);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_DEVICE_HIGH, 
		used >> 32);

	// bus_space_write_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_QUEUE_ALIGN,
	//     VIRTIO_PAGE_SIZE);
	// bus_space_write_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_QUEUE_PFN,
	//     addr / VIRTIO_PAGE_SIZE);

	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_QUEUE_READY, 0x1);
}

static void
virtio_mmio_set_status(struct virtio_softc *vsc, int status)
{
	struct virtio_mmio_softc *sc = (struct virtio_mmio_softc *)vsc;
	int old = 0;

	if (status != 0)
		old = bus_space_read_4(sc->sc_iot, sc->sc_ioh,
				       VIRTIO_MMIO_1_STATUS);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_1_STATUS,
			  status|old);
}

bool
virtio_mmio_common_probe_present(struct virtio_mmio_softc *sc)
{
	uint32_t magic;

	magic = bus_space_read_4(sc->sc_iot, sc->sc_ioh,
	    VIRTIO_MMIO_1_MAGIC_VALUE);
	return (magic == VIRTIO_MMIO_MAGIC);
}

void
virtio_mmio_common_attach(struct virtio_mmio_softc *sc)
{
	struct virtio_softc *vsc = &sc->sc_sc;
	device_t self = vsc->sc_dev;
	uint32_t id, magic, ver;
	magic = bus_space_read_4(sc->sc_iot, sc->sc_ioh,
	    VIRTIO_MMIO_1_MAGIC_VALUE);
	if (magic != VIRTIO_MMIO_MAGIC) {
		aprint_error_dev(vsc->sc_dev,
		    "wrong magic value 0x%08x; giving up\n", magic);
		return;
	}

	//TODO: make transitional driver that accepts both versions
	//accepting only non-legacy version
	ver = bus_space_read_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_1_VERSION);
	if (/*ver != 1 &&*/ ver != VIRTIO_MMIO_1_VERSION_1) {
		aprint_error_dev(vsc->sc_dev,
		    "unknown version 0x%02x; giving up\n", ver);
		return;
	}

	id = bus_space_read_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_1_DEVICE_ID);
	
	/* Legacy */
	/* we could use PAGE_SIZE, but virtio(4) assumes 4KiB for now */
	// bus_space_write_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_GUEST_PAGE_SIZE,
	//     VIRTIO_PAGE_SIZE);

	/* no device connected. */
	if (id == 0)
		return;

	virtio_print_device_type(self, id, ver);
	vsc->sc_ops = &virtio_mmio_ops;

	/* Legacy */	
	// vsc->sc_bus_endian    = READ_ENDIAN;
	// vsc->sc_struct_endian = STRUCT_ENDIAN;

	/* Only Little-Endian in version 1 */
	vsc->sc_bus_endian    = LITTLE_ENDIAN;
	vsc->sc_struct_endian = LITTLE_ENDIAN;

	/* set up our device config tag */
	vsc->sc_devcfg_iosize = sc->sc_iosize - VIRTIO_MMIO_1_CONFIG;
	vsc->sc_devcfg_iot = sc->sc_iot;
	if (bus_space_subregion(sc->sc_iot, sc->sc_ioh,
			VIRTIO_MMIO_1_CONFIG, vsc->sc_devcfg_iosize,
			&vsc->sc_devcfg_ioh)) {
		aprint_error_dev(self, "can't map config i/o space\n");
		return;
	}

	virtio_device_reset(vsc);
	virtio_mmio_set_status(vsc, VIRTIO_CONFIG_DEVICE_STATUS_ACK);
	virtio_mmio_set_status(vsc, VIRTIO_CONFIG_DEVICE_STATUS_DRIVER);

	/* XXX: use softc as aux... */
	vsc->sc_childdevid = id;
	vsc->sc_child = NULL;

	virtio_mmio_alloc_interrupts((struct virtio_softc*) sc);
}

int
virtio_mmio_common_detach(struct virtio_mmio_softc *sc, int flags)
{
	struct virtio_softc *vsc = &sc->sc_sc;
	int r;

	if (vsc->sc_child != NULL && vsc->sc_child != VIRTIO_CHILD_FAILED) {
		r = config_detach(vsc->sc_child, flags);
		if (r)
			return r;
	}
	KASSERT(vsc->sc_child == NULL || vsc->sc_child == VIRTIO_CHILD_FAILED);
	KASSERT(vsc->sc_vqs == NULL);
	KASSERT(sc->sc_ih == NULL);

	if (sc->sc_iosize) {
		bus_space_unmap(sc->sc_iot, sc->sc_ioh, sc->sc_iosize);
		sc->sc_iosize = 0;
	}

	return 0;
}

/*
 * Feature negotiation.
 */
static uint64_t
virtio_mmio_negotiate_features(struct virtio_softc *vsc, uint64_t
    guest_features)
{
	struct virtio_mmio_softc *sc = (struct virtio_mmio_softc *)vsc;
	device_t self          =  vsc->sc_dev;
	bus_space_tag_t	   iot = sc->sc_iot;
	bus_space_handle_t ioh = sc->sc_ioh;
	//uint32_t r;
	uint64_t host, negotiated, status;

	guest_features |= VIRTIO_F_VERSION_1;
	/* notify on empty is 0.9 only */
	guest_features &= ~VIRTIO_F_NOTIFY_ON_EMPTY;
	vsc->sc_active_features = 0;

	//4
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_DEVICE_FEATURES_SEL, 0);
	host = bus_space_read_4(iot, ioh, VIRTIO_MMIO_1_DEVICE_FEATURES);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_DEVICE_FEATURES_SEL, 1);
	host |= (uint64_t)
		bus_space_read_4(iot, ioh, VIRTIO_MMIO_1_DEVICE_FEATURES) << 32;
	
	negotiated = host & guest_features;

	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_DRIVER_FEATURES_SEL, 0);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_DRIVER_FEATURES, 
		negotiated & 0xffffffff);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_DRIVER_FEATURES_SEL, 1);
	bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_DRIVER_FEATURES,
			negotiated >> 32);
	
	//5
	virtio_mmio_set_status(vsc, VIRTIO_CONFIG_DEVICE_STATUS_FEATURES_OK);
	//6
	status = bus_space_read_4(iot, ioh, VIRTIO_MMIO_1_STATUS);
	if((status & VIRTIO_CONFIG_DEVICE_STATUS_FEATURES_OK) == 0) {
		aprint_error_dev(self, "feature negotiation failed\n");
		bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_STATUS,
				VIRTIO_CONFIG_DEVICE_STATUS_FAILED);
		return negotiated;
	}

	if ((negotiated & VIRTIO_F_VERSION_1) == 0) {
		aprint_error_dev(self, "host rejected version 1\n");
		bus_space_write_4(iot, ioh, VIRTIO_MMIO_1_STATUS,
				VIRTIO_CONFIG_DEVICE_STATUS_FAILED);
		return negotiated;
	}

	vsc->sc_active_features = negotiated;
    return negotiated;
}

/*
 * Interrupt handler.
 */
int
virtio_mmio_intr(void *arg)
{
	struct virtio_mmio_softc *sc = arg;
	struct virtio_softc *vsc = &sc->sc_sc;
	int isr, r = 0;
	aprint_normal("INTR INTR INTR\n");

	/* check and ack the interrupt */
	isr = bus_space_read_4(sc->sc_iot, sc->sc_ioh,
			       VIRTIO_MMIO_1_INTERRUPT_STATUS);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh,
			  VIRTIO_MMIO_1_INTERRUPT_ACK, isr);
	if ((isr & VIRTIO_MMIO_1_INT_CONFIG) &&
	    (vsc->sc_config_change != NULL))
		r = (vsc->sc_config_change)(vsc);
	if ((isr & VIRTIO_MMIO_1_INT_USED_BUFFER) &&
	    (vsc->sc_intrhand != NULL)) {
		if (vsc->sc_soft_ih != NULL)
			softint_schedule(vsc->sc_soft_ih);
		else
			r |= (vsc->sc_intrhand)(vsc);
	}

	return r;
}

static void
virtio_mmio_kick(struct virtio_softc *vsc, uint16_t idx)
{
	//TODO: VIRTIO_F_NOTIFICATION_DATA alternative?
	struct virtio_mmio_softc *sc = (struct virtio_mmio_softc *)vsc;
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, VIRTIO_MMIO_1_QUEUE_NOTIFY,
	    idx);
}

static int
virtio_mmio_alloc_interrupts(struct virtio_softc *vsc)
{
	struct virtio_mmio_softc * const sc = (struct virtio_mmio_softc *)vsc;

	return sc->sc_alloc_interrupts(sc);
}

static void
virtio_mmio_free_interrupts(struct virtio_softc *vsc)
{
	struct virtio_mmio_softc * const sc = (struct virtio_mmio_softc *)vsc;

	sc->sc_free_interrupts(sc);
}

static int
virtio_mmio_setup_interrupts(struct virtio_softc *vsc __unused,
    int reinit __unused)
{

	return 0;
}
