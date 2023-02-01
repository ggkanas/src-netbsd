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
#include <dev/pci/virtiovar.h>

#include <rump-sys/kern.h>
#include <rump-sys/dev.h>
#include <rump-sys/vfs.h>

#include "ioconf.c"

void virtioattach(int num) {
	int error;
    aprint_normal("virtio rump component\n");

	// /* go, mydevfs */
	// bmaj = cmaj = -1;

	// if ((error = devsw_attach("virtio", NULL, &bmaj,
	//     &virtio_cdevsw, &cmaj)) != 0)
	// 	panic("cannot attach virtio: %d", error);

	cfdata_ioconf_virtio->cf_name = "virtio";
	cfdata_ioconf_virtio->cf_atname = "mmiocmdl";
	struct cfparent * parent = malloc(sizeof(struct cfparent), M_DEVBUF, M_WAITOK);
	parent->cfp_parent = "mmiocmdl";
	parent->cfp_iattr = "virtio";
	cfdata_ioconf_virtio->cf_pspec = parent;

	aprint_normal("%s\n", cfdata_ioconf_virtio->cf_name);
	aprint_normal("%s\n", cfdata_ioconf_virtio->cf_atname);
	aprint_normal("%s\n", cfdata_ioconf_virtio->cf_pspec->cfp_parent);

    error = config_init_component(cfdriver_ioconf_virtio,
    cfattach_ioconf_virtio, cfdata_ioconf_virtio);

    if(error) aprint_normal("virtio: Error init component: %d\n", error);
}

RUMP_COMPONENT(RUMP_COMPONENT_DEV)
{
	// extern const struct cdevsw virtio_cdevsw;
	// devmajor_t bmaj, cmaj;
	int error;
    aprint_normal("virtio rump component\n");

	// /* go, mydevfs */
	// bmaj = cmaj = -1;

	// if ((error = devsw_attach("virtio", NULL, &bmaj,
	//     &virtio_cdevsw, &cmaj)) != 0)
	// 	panic("cannot attach virtio: %d", error);

    error = config_init_component(cfdriver_ioconf_virtio,
    cfattach_ioconf_virtio, cfdata_ioconf_virtio);

    if(error) aprint_normal("virtio: Error init component: %d\n", error);

}