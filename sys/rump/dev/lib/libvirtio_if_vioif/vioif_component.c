/*	$NetBSD: rnd_component.c,v 1.5 2016/05/30 14:52:06 pooka Exp $	*/

/*
 * Copyright (c) 2009 Antti Kantee.  All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: rnd_component.c,v 1.5 2016/05/30 14:52:06 pooka Exp $");

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/stat.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#include <dev/pci/virtiovar.h>

#include <rump-sys/kern.h>
#include <rump-sys/dev.h>
#include <rump-sys/vfs.h>

#include "ioconf.c"

// void virtioattach(int num) {
// 	int error;
//     aprint_normal("virtio rump component\n");

// 	// /* go, mydevfs */
// 	// bmaj = cmaj = -1;

// 	// if ((error = devsw_attach("virtio", NULL, &bmaj,
// 	//     &virtio_cdevsw, &cmaj)) != 0)
// 	// 	panic("cannot attach virtio: %d", error);

// 	cfdata_ioconf_virtio->cf_name = "virtio";
// 	cfdata_ioconf_virtio->cf_atname = "mmiocmdl";
// 	struct cfparent * parent = malloc(sizeof(struct cfparent), M_DEVBUF, M_WAITOK);
// 	parent->cfp_parent = "mmiocmdl";
// 	parent->cfp_iattr = "virtio";
// 	cfdata_ioconf_virtio->cf_pspec = parent;

// 	aprint_normal("%s\n", cfdata_ioconf_virtio->cf_name);
// 	aprint_normal("%s\n", cfdata_ioconf_virtio->cf_atname);
// 	aprint_normal("%s\n", cfdata_ioconf_virtio->cf_pspec->cfp_parent);

//     error = config_init_component(cfdriver_ioconf_virtio,
//     cfattach_ioconf_virtio, cfdata_ioconf_virtio);

//     if(error) aprint_normal("virtio: Error init component: %d\n", error);
// }

CFDRIVER_DECL(vioif, DV_DULL, NULL);
extern struct cfattach vioif_ca;

extern struct cfattach mmiocmdl_ca;
static struct cfattach * cas[] = {&vioif_ca, NULL};
static struct cfattachinit cai[] = {{"vioif", cas}, { NULL, NULL }};

RUMP_COMPONENT(RUMP_COMPONENT_DEV)
{
	// extern const struct cdevsw virtio_cdevsw;
	// devmajor_t bmaj, cmaj;
	int error;
    aprint_normal("vioif rump component\n");

    cfdata_ioconf_virtio_if_vioif->cf_name = "vioif";
	cfdata_ioconf_virtio_if_vioif->cf_atname = "mmiocmdl";
	struct cfparent * parent = malloc(sizeof(struct cfparent), M_DEVBUF, M_WAITOK);
	parent->cfp_parent = "mmiocmdl";
	parent->cfp_iattr = "mmiocmdl";
    parent->cfp_unit = DVUNIT_ANY;

	cfdata_ioconf_virtio_if_vioif->cf_pspec = parent;
    struct cfdriver * const a = malloc(sizeof(struct cfdriver), M_DEVBUF, M_WAITOK);
    a->cd_name = "mmiocmdl";
    struct cfdriver ** cds = malloc(sizeof (struct cfdriver*), M_DEVBUF, M_WAITOK);
    cds[0] = &vioif_cd;
    // *(cfdriver_ioconf_virtio_if_vioif[0]) = *a;


	// struct cfattach **ca = malloc(sizeof (struct cfattach*), M_DEVBUF, M_WAITOK);
	// *ca = &mmiocmdl_ca;
	// struct cfattachinit cai = {.cfai_name="vioif", .cfai_list=ca };
	// // cai.cfai_list[0] = &mmiocmdl_cd;
	// *cfattach_ioconf_virtio_if_vioif = cai;
	// // memcpy(cfattach_ioconf_virtio_if_vioif, &cai, sizeof(cai));
	// // cfattach_ioconf_virtio_if_vioif->cfai_name = "vioif";
	// cfattach_ioconf_virtio_if_vioif->cfai_list[0] = &mmiocmdl_ca;



	aprint_normal("%s\n", vioif_ca.ca_name);
	// aprint_normal("%s\n", a.cd_a);
	aprint_normal("%s\n", cfdata_ioconf_virtio_if_vioif->cf_pspec->cfp_parent);

	// /* go, mydevfs */
	// bmaj = cmaj = -1;

	// if ((error = devsw_attach("virtio", NULL, &bmaj,
	//     &virtio_cdevsw, &cmaj)) != 0)
	// 	panic("cannot attach virtio: %d", error);

    error = config_init_component(cds,
    cai, cfdata_ioconf_virtio_if_vioif);
    if(error) {
        aprint_error("vioif: Error init component: %d\n", error);
	}

    // error = config_cfattach_attach(vioif_cd.cd_name, &vioif_ca);
	// if (error) {
	// 	aprint_error("%s: unable to register cfattach, error = %d\n",
	// 	    vioif_cd.cd_name, error);
    // }

    

}