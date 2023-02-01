/* $NetBSD: virtio_mmiocmdl.c,v 1.10 2021/10/22 02:57:23 yamaguchi Exp $ */

/*
 * Copyright (c) 2018 Jonathan A. Kollasch
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: virtio_mmiocmdl.c,v 1.10 2021/10/22 02:57:23 yamaguchi Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/intr.h>

#include <sys/device.h>

#include <dev/mmio.h>
#include <dev/mmiocmdl/mmiocmdlvar.h>

#define VIRTIO_PRIVATE
#include <dev/virtio/virtio_mmiovar.h>

static int	virtio_mmiocmdl_match(device_t, cfdata_t, void *);
static void	virtio_mmiocmdl_attach(device_t, device_t, void *);
//static int	virtio_mmiocmdl_rescan(device_t, const char *, const int *);
static int	virtio_mmiocmdl_detach(device_t, int);

static int	virtio_mmiocmdl_alloc_interrupts(struct virtio_mmio_softc *);
static void	virtio_mmiocmdl_free_interrupts(struct virtio_mmio_softc *);

struct virtio_mmiocmdl_softc {
	struct virtio_mmio_softc	sc_msc;
	int				sc_phandle;
};

CFATTACH_DECL3_NEW(virtio_mmiocmdl, sizeof(struct virtio_mmiocmdl_softc),
    virtio_mmiocmdl_match, virtio_mmiocmdl_attach, virtio_mmiocmdl_detach, NULL,
    /*virtio_mmiocmdl_rescan*/ NULL, NULL, DVF_DETACH_SHUTDOWN);

static int
virtio_mmiocmdl_match(device_t parent, cfdata_t match, void *aux)
{
	struct mmiocmdl_attach_args * const maa = aux;
    if(mmio_device_info_entry_index > 0) {
        struct dev_info_t dev = mmio_device_info_entries[--mmio_device_info_entry_index];
        maa->maa_addr = dev.address;
        maa->maa_size = dev.size;
        maa->maa_irq_no = dev.irq_no;
        return 1;
    }
    return 0;
}

extern struct x86_bus_dma_tag isa_bus_dma_tag;

static void
virtio_mmiocmdl_attach(device_t parent, device_t self, void *aux)
{
	struct virtio_mmiocmdl_softc * const mmsc = device_private(self);
	struct virtio_mmio_softc * const msc = &mmsc->sc_msc;
	struct virtio_softc * const vsc = &msc->sc_sc;
	struct mmiocmdl_attach_args * const maa = aux;
	bus_addr_t addr;
	bus_size_t size;
	int error;

	aprint_normal("\n");
	aprint_naive("\n");

	addr = maa->maa_addr;
    size = maa->maa_size;

	mmsc->sc_phandle = maa->maa_phandle;
	msc->sc_iot = 1;
	vsc->sc_dev = self;
	vsc->sc_dmat = &isa_bus_dma_tag; //is this okay?

	error = bus_space_map(msc->sc_iot, addr, size, 0, &msc->sc_ioh);
	if (error) {
		aprint_error_dev(self, "couldn't map %#" PRIx64 ": %d",
		    (uint64_t)addr, error);
		return;
	}
	msc->sc_iosize = size;

	msc->sc_alloc_interrupts = virtio_mmiocmdl_alloc_interrupts;
	msc->sc_free_interrupts = virtio_mmiocmdl_free_interrupts;

	virtio_mmio_common_attach(msc);
	//virtio_mmiocmdl_rescan(self, NULL, NULL);
}

/* ARGSUSED */
/*static int
virtio_mmiocmdl_rescan(device_t self, const char *attr, const int *scan_flags)
{
	struct virtio_mmiocmdl_softc * const fsc = device_private(self);
	struct virtio_mmio_softc * const msc = &fsc->sc_msc;
	struct virtio_softc * const vsc = &msc->sc_sc;
	struct virtio_attach_args va;

	if (vsc->sc_child)*/	/* Child already attached? */
/*		return 0;

	memset(&va, 0, sizeof(va));
	va.sc_childdevid = vsc->sc_childdevid;

	config_found(self, &va, NULL, CFARGS_NONE);

	if (virtio_attach_failed(vsc))
		return 0;

	return 0;
}*/

static int
virtio_mmiocmdl_detach(device_t self, int flags)
{
	struct virtio_mmiocmdl_softc * const mmsc = device_private(self);
	struct virtio_mmio_softc * const msc = &mmsc->sc_msc;

	return virtio_mmio_common_detach(msc, flags);
}

static int
virtio_mmiocmdl_alloc_interrupts(struct virtio_mmio_softc *msc)
{
	//struct virtio_mmiocmdl_softc * const mmsc = (void *)msc;
	struct virtio_softc * const vsc = &msc->sc_sc;
	char intrstr[128];
	int flags = 0;

	/*if (!fdtbus_intr_str(mmsc->sc_phandle, 0, intrstr, sizeof(intrstr))) {
		aprint_error_dev(vsc->sc_dev, "failed to decode interrupt\n");
		return -1;
	}*/

	if (vsc->sc_flags & VIRTIO_F_INTR_MPSAFE)
		flags |= VIRTIO_F_INTR_MPSAFE;
    flags |= vsc->sc_ipl;

	msc->sc_ih = softint_establish(flags, (void (*)(void *))virtio_mmio_intr, msc);
	if (msc->sc_ih == NULL) {
		aprint_error_dev(vsc->sc_dev,
		    "failed to establish interrupt on %s\n", intrstr);
		return -1;
	}
	aprint_normal_dev(vsc->sc_dev, "interrupting on %s\n", intrstr);

	return 0;
}

static void
virtio_mmiocmdl_free_interrupts(struct virtio_mmio_softc *msc)
{
	//struct virtio_mmiocmdl_softc * const mmsc = (void *)msc;

	if (msc->sc_ih != NULL) {
		softint_disestablish(msc->sc_ih);
		msc->sc_ih = NULL;
	}
}
