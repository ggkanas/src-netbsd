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

#include <sys/device.h>

#include <dev/mmio.h>
#include <dev/mmiocmdl/mmiocmdlvar.h>

#define VIRTIO_PRIVATE
#include <dev/virtio/virtio_mmiovar.h>

static int	mmiocmdl_match(device_t, cfdata_t, void *);
static void	mmiocmdl_attach(device_t, device_t, void *);

struct mmiocmdl_softc {
	struct virtio_mmio_softc	sc_msc;
	int				sc_phandle;
};


CFATTACH_DECL3_NEW(virtio_mmiocmdl, sizeof(struct mmiocmdl_softc),
    mmiocmdl_match, mmiocmdl_attach, NULL, NULL,
    NULL, NULL, DVF_DETACH_SHUTDOWN);

static int
mmiocmdl_match(device_t parent, cfdata_t match, void *aux)
{
	return 1;
}

static void
mmiocmdl_attach(device_t parent, device_t self, void *aux)
{
    while (mmio_device_info_entry_index)
        config_found(self, aux, NULL);
}
