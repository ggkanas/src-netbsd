/*	$NetBSD: ld_at_virtio.c,v 1.3 2016/01/26 23:12:16 pooka Exp $	*/

/*
 * Copyright (c) 2010 Antti Kantee.  All Rights Reserved.
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
__KERNEL_RCSID(0, "$NetBSD: ld_at_virtio.c,v 1.3 2016/01/26 23:12:16 pooka Exp $");

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/bus.h>
#include <sys/stat.h>
#include <sys/disklabel.h>
#include <sys/mount.h>

#include <rump-sys/kern.h>
#include <rump-sys/vfs.h>

// #include "ioconf.c"

#include "ioconf.h"
#define IOCONF virtio_ld




// static const struct cfiattrdata mmiocmdlcf_iattrdata = {
// 	"mmiocmdl", 0, {
// 		{ NULL, NULL, 0 },
// 	}
// };

CFDRIVER_DECL(ld, DV_DISK, NULL);


static struct cfdriver * const cfdriver_ioconf_virtio_ld[] = {
	&ld_cd,
	NULL
};

extern struct cfattach ld_virtio_ca;

static const struct cfparent pspec0 = {
	"mmiocmdl", "mmiocmdl", DVUNIT_ANY
};

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

static struct cfdata cfdata_ioconf_virtio_ld[] = {
    /* driver           attachment    unit state      loc   flags  pspec */
/*  0: ld* at virtio? */
    { "ld",		"ld_virtio",	 0, STAR,    NULL,      0, &pspec0 },
    { NULL,		NULL,		 0,    0,    NULL,      0, NULL }
};

static struct cfattach * const ld_cfattachinit[] = {
	&ld_virtio_ca, NULL
};

static const struct cfattachinit cfattach_ioconf_virtio_ld[] = {
	{ "ld", ld_cfattachinit },
	{ NULL, NULL }
};


RUMP_COMPONENT(RUMP_COMPONENT_DEV)
{

	config_init_component(cfdriver_ioconf_virtio_ld,
	    cfattach_ioconf_virtio_ld, cfdata_ioconf_virtio_ld);
}

/*
 * Pseudo-devfs.  Since creating device nodes is non-free, don't
 * speculatively create hundreds of them (= milliseconds slower
 * bootstrap).  Instead, after the probe is done, see which units
 * were found and create nodes only for them.
 */
RUMP_COMPONENT(RUMP_COMPONENT_POSTINIT)
{
	extern const struct bdevsw ld_bdevsw;
	extern const struct cdevsw ld_cdevsw;
	devmajor_t bmaj = -1, cmaj = -1;
	int error, i;

	if ((error = devsw_attach("ld", &ld_bdevsw, &bmaj,
	    &ld_cdevsw, &cmaj)) != 0)
		panic("cannot attach ld: %d", error);

	for (i = 0; i < 10; i++) {
		char bbase[] = "/dev/ldX";
		char rbase[] = "/dev/rldX";

		if (device_lookup(&ld_cd, i) == NULL)
			break;

		bbase[sizeof(bbase)-2] = '0' + i;
		rbase[sizeof(rbase)-2] = '0' + i;

		if ((error = rump_vfs_makedevnodes(S_IFBLK, bbase, 'a',
		    bmaj, DISKMINOR(i, 0), 5)) != 0)
			panic("cannot create cooked ld dev nodes: %d", error);
		if ((error = rump_vfs_makedevnodes(S_IFCHR, rbase, 'a',
		    cmaj, DISKMINOR(i, 0), 5)) != 0)
			panic("cannot create raw ld dev nodes: %d", error);
	}
}
