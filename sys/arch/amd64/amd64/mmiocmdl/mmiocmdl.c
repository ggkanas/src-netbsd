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
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/kauth.h>
#include <sys/kmem.h>
#include <sys/malloc.h>
#include <machine/intr.h>
#include <machine/i8259.h>
#include <machine/pic.h>

#include <dev/mmio.h>
#include <machine/mmiocmdl/mmiocmdlvar.h>
// #include <dev/acpi/acpivar.h>
// #include <dev/acpi/acpi_intr.h>

#define VIRTIO_PRIVATE
#include <dev/virtio/virtio_mmiovar.h>

#include "ioconf.h"


// extern struct cfattach mmiocmdl_ca;
// extern struct cfdriver mmiocmdl_cd;
// extern struct cfdata* cfdata_ioconf_mmiocmdl;
extern paddr_t vtophys(vaddr_t);

// static int	mmiocmdl_match(device_t, cfdata_t, void *);
static void	mmiocmdl_attach(device_t, device_t, void *);
static int mmiocmdl_rescan(device_t, const char*, const int*);
static int mmiocmdl_alloc_interrupts(struct virtio_mmio_softc *);
static void mmiocmdl_free_interrupts(struct virtio_mmio_softc *);

struct mmiocmdl_softc {
	struct virtio_mmio_softc	sc_msc;
	int				sc_phandle;
    int				sc_irq;
};


CFATTACH_DECL3_NEW(mmiocmdl, sizeof(struct mmiocmdl_softc),
    NULL, mmiocmdl_attach, NULL, NULL,
    NULL, NULL, DVF_DETACH_SHUTDOWN);

// CFDRIVER_DECL(mmiocmdl, DV_VIRTUAL, NULL);

void
mmiocmdl_attach(device_t parent, device_t self, void * aux)
{
    struct mmiocmdl_softc * const sc = device_private(self);
    struct virtio_mmio_softc * const msc = &sc->sc_msc;
    struct virtio_softc * const vsc = &msc->sc_sc;
    // struct mmiocmdl_attach_args * const maa = aux;
    bus_addr_t addr;
    bus_size_t size;
    int error;

    addr = (bus_addr_t)mmio_device_info_entries[0].address;
    size = (bus_size_t)mmio_device_info_entries[0].size;

    sc->sc_phandle = addr;
    msc->sc_iot = (bus_space_tag_t)1; //x86_bus_space_mem
    vsc->sc_dev = self;
    // vsc->sc_dmat = &pci_bus_dma_tag;

    error = bus_space_map(msc->sc_iot, addr, size, 0, &msc->sc_ioh);
    if(error) {
        aprint_error_dev(self, "couldn't map %#" PRIx64 ": %d", (uint64_t)addr, error);
        return;
    }
    msc->sc_iosize = size;

    msc->sc_alloc_interrupts = mmiocmdl_alloc_interrupts;
    msc->sc_free_interrupts = mmiocmdl_free_interrupts;
    aprint_normal("1\n");


    virtio_mmio_common_attach(msc);
    aprint_normal("2\n");

    mmiocmdl_rescan(self, "virtio", NULL);

    aprint_normal("mmiocmdl attach\n");

    aprint_normal("virtual %p\n", &self);
    aprint_normal("physical %p\n", (void*)vtophys((vaddr_t)&self));

    for (int i = 0; i < mmio_device_info_entry_index; ++i)
        //config_found(self, aux, NULL);
        aprint_normal("found mmio\n");
}


extern struct cfattach vioif_ca;

int match_child(device_t, cfdata_t, const int*, void*);
int
match_child(device_t parent, cfdata_t cf, const int *locs, void* aux) {
    // struct mmiocmdl_softc * const sc = device_private(parent);
    // struct virtio_mmio_softc * const msc = &sc->sc_msc;
    // struct virtio_softc * const vsc = &msc->sc_sc;
    if (vioif_ca.ca_match(parent, cf, aux)) return 1;
    else return 0;
}

static int
mmiocmdl_rescan(device_t self, const char *ifattr, const int *locs) {
    struct mmiocmdl_softc * const sc = device_private(self);
    struct virtio_mmio_softc * const msc = &sc->sc_msc;
    struct virtio_softc * const vsc = &msc->sc_sc;
    struct virtio_attach_args va;

    aprint_normal("3\n");
    if (vsc->sc_child) return 0;

    aprint_normal("4\n");
    memset(&va, 0, sizeof(va));
    va.sc_childdevid = vsc->sc_childdevid;
    aprint_normal("5: %d\n", vsc->sc_childdevid);
    // cfdata_t cf = config_search_ia(NULL, self, "virtio", &va);
    // config_attach(self, cf, &va, NULL);
     
    
    struct cfiattrdata * second = malloc(sizeof(const struct cfiattrdata), M_DEVBUF, M_WAITOK);
    second->ci_name = "mmiocmdl";

    const struct cfiattrdata ** first = malloc(3*sizeof(const struct cfiattrdata * const), M_DEVBUF, M_WAITOK);
    *first = second;
    second = malloc(sizeof(const struct cfiattrdata), M_DEVBUF, M_WAITOK);
    second->ci_name = "virtio";
    (first[1]) = second;
    first[2] = NULL;
    (self->dv_cfdriver->cd_attrs) = first;
    // memcpy(first, &second, sizeof(struct cfiattrdata));
    // memcpy((const struct cfiattrdata **)self->dv_cfdriver->cd_attrs, &first, sizeof(struct cfiattrdata*));
    // (*(self->dv_cfdriver->cd_attrs)) = malloc(sizeof(const struct cfiattrdata *), M_DEVBUF, M_WAITOK);
    // *(*(self->dv_cfdriver->cd_attrs[0])) = 
    // (*self->dv_cfdriver->cd_attrs[0])->ci_name = "mmiocmdl";
    // config_found_sm_loc(self, NULL, NULL, &va, NULL, match_child);
    config_found(self, &va, NULL);
    // if (vioif_ca.ca_match(self, NULL, vsc)) vioif_ca.ca_attach(self)
    aprint_normal("6\n");
    if(virtio_attach_failed(vsc)) return 0;
    aprint_normal("7\n");
    return 0;
}

void
mmiocmdlattach(int num)
{
    int error;

    // error = config_cfdriver_attach(&mmiocmdl_cd);
    // if (error) {
    //     aprint_error("%s: unable to register cfdriver, error = %d\n",
    //         mmiocmdl_cd.cd_name, error);
    // }
	error = config_cfattach_attach(mmiocmdl_cd.cd_name, &mmiocmdl_ca);
	if (error) {
		aprint_error("%s: unable to register cfattach, error = %d\n",
		    mmiocmdl_cd.cd_name, error);
    }
    // struct mmiocmdl_attach_args* aux = kmem_zalloc(sizeof(struct mmiocmdl_attach_args), KM_SLEEP);
    // cfdata_t cf = config_search_ia(NULL, NULL, "mmiocmdl", aux);
    cfdata_t cf = malloc(sizeof(*cf), M_DEVBUF, M_WAITOK);
		cf->cf_name = mmiocmdl_cd.cd_name;
		cf->cf_atname = mmiocmdl_cd.cd_name;
		cf->cf_unit = 0;
		cf->cf_fstate = FSTATE_STAR;
    config_attach_pseudo(cf);
    // device_t mmiocmdl = device_lookup(&mmiocmdl_cd, 0);
    // mmiocmdl_attach(NULL, mmiocmdl , (void*)&aux);
    
}

static int
mmiocmdl_alloc_interrupts(struct virtio_mmio_softc *msc)
{
	// struct mmiocmdl_softc * const sc = (struct mmiocmdl_softc *)msc;
	// struct virtio_softc * const vsc = &msc->sc_sc;

    // // struct acpi_irq *irq = kmem_zalloc(sizeof(struct acpi_irq), KM_SLEEP);
    // // acpi_irq->ar_irq = 5;
    // //(uint64_t)(uintptr_t)sc->sc_handle
	// msc->sc_ih = intr_establish_xname(5,  &i8259_pic, 0, IST_EDGE,
    //     IPL_VM, virtio_mmio_intr,
    //         msc, false, device_xname(vsc->sc_dev));
	// if (msc->sc_ih == NULL) {
	// 	aprint_error_dev(vsc->sc_dev, "couldn't install interrupt handler\n");
	// 	return -1;
	// }

	// aprint_normal_dev(vsc->sc_dev, "interrupting on irq %d\n", sc->sc_irq);

	return 0;
}

static void
mmiocmdl_free_interrupts(struct virtio_mmio_softc *msc)
{
	// if (msc->sc_ih != NULL) {
	// 	acpi_intr_disestablish(msc->sc_ih);
	// 	msc->sc_ih = NULL;
	// }
}


