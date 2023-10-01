#include <sys/types.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/kmem.h>

#include <rump-sys/kern.h>
#include <rump-sys/dev.h>
#include <rump-sys/vfs.h>

#include <dev/pci/pcivar.h>
#include <machine/mmiocmdl/mmiocmdlvar.h>

#include "ioconf.h"
#include "ioconf.c"

#include <rump-sys/vfs.h>

// struct cfdriver mmiocmdl_cd;
// struct cfattach mmiocmdl_ca;
extern struct cfdata cfdata[];

int mmiocmdlprint(void *, const char*);

#define S_IFCHR 0020000

// const struct cdevsw mmiocmdl_cdevsw = {
// 	.d_open = noopen,
// 	.d_close = nullclose,
// 	.d_read = noread,
// 	.d_write = nowrite,
// 	.d_ioctl = noioctl,
// 	.d_stop = nostop,
// 	.d_tty = notty,
// 	.d_poll = nopoll,
// 	.d_mmap = nommap,
// 	.d_kqfilter = nokqfilter,
// 	.d_discard = nodiscard,
// 	.d_flag = D_OTHER
// };

// extern struct cfattach mmiocmdl_ca;
// extern struct cfattach vioif_ca;
// static struct cfattach * cas[] = {&vioif_ca, NULL};
// static struct cfattachinit cai[] = {{"mmiocmdl", cas}, { NULL, NULL }};

RUMP_COMPONENT(RUMP_COMPONENT_DEV)
{
    // int error;

    config_init_component(cfdriver_ioconf_mmiocmdl,
        cfattach_ioconf_mmiocmdl, cfdata_ioconf_mmiocmdl);


    memcpy(&cfdata[1], &cfdata_ioconf_mmiocmdl[0], sizeof(cfdata[1]));


	rump_pdev_add(mmiocmdlattach, 4);
    



    // cfdata_t cf = cfdata_ioconf_mmiocmdl;
    // struct cfdriver ** cds = kmem_zalloc(sizeof(struct cfdriver*), KM_SLEEP);
    // struct cfattachinit *casi;
    // struct cfattach ** cas = kmem_zalloc(sizeof(struct cfattach*), KM_SLEEP);
    // //struct cfattachinit mmiocmdl_cas = {.cfai_name = "mmiocmdl", .cfai_list = cas};
    // casi = kmem_alloc(sizeof(struct cfattachinit), KM_SLEEP);
    // *casi = cfattach_ioconf_mmiocmdl[0];
    // //mmiocmdl_cas;
    //
    // cds[0] = &mmiocmdl_cd;
    // cas[0] = &mmiocmdl_ca;
    // cf->cf_name = "mmiocmdl";
    // cf->cf_atname = "mmmiocmdl";
    // cf->cf_fstate = FSTATE_NOTFOUND;
    // //cfdriver_ioconf_mmiocmdl[0]->cd_name = mmiocmdl_cd.cd_name;
    //
    // //config_init_component(cds,
    // //    casi, cfdata_ioconf_mmiocmdl);
    // config_cfdata_attach(cfdata_ioconf_mmiocmdl, 0);
	// config_cfdriver_attach(cds[0]);
	// config_cfattach_attach(casi->cfai_name, casi->cfai_list[0]);
    // aprint_normal("cf name %s\n", cfdata_ioconf_mmiocmdl->cf_name);
    // if(cds[0] != NULL) aprint_normal("cd not null\n");//, cfdriver_ioconf_mmiocmdl[0].cd_name);
    // else aprint_normal("cd null\n");
    // aprint_normal("cf atname %s\n", cfdata_ioconf_mmiocmdl->cf_atname);
    // aprint_normal("cf unit %d\n", cf->cf_unit);
    // aprint_normal("cf fstate %d\n", cf->cf_fstate);
    // aprint_normal("cf flags %d\n", cf->cf_flags);
    // if(cf->cf_pspec){
    //     aprint_normal("cf parent iattr %s\n", cf->cf_pspec->cfp_iattr);
    //     aprint_normal("cf parent cfp_parent %s\n", cf->cf_pspec->cfp_parent);
    // }
    // return;
}

int
mmiocmdlprint(void *vpa, const char *pnp)
{
	struct mmiocmdl_attach_args *maa = vpa;

	if (pnp)
		aprint_normal("pci at %s", pnp);
	aprint_normal(" phandle %d", maa->maa_phandle);
	return (UNCONF);
}

RUMP_COMPONENT(RUMP_COMPONENT_DEV_AFTERMAINBUS) {

    // cfdata_t mmiocmdl_cd = kmem_zalloc(sizeof(struct cfdata), KM_SLEEP);

    // mmiocmdl_cd->cf_name = "mmiocmdl";


    // device_t mmiocmdl = config_attach_pseudo(mmiocmdl_cd);
    // aprint_normal("%s\n", mmiocmdl->dv_xname);



    // //dgreg
    // struct mmiocmdl_attach_args maa;
    // // device_t mainbus;
    // device_t self;

    // /* XXX: attach args should come from elsewhere */
    // memset(&maa, 0, sizeof(maa));

    // // maa.maa_bus = 0;
    // // maa.maa_iot = (bus_space_tag_t)0;
    // // maa.maa_memt = (bus_space_tag_t)1;
    // maa.maa_dmat = (void *)0x20;
    // // #ifdef _LP64
    // // maa.maa_dmat64 = (void *)0x40;
    // // #endif
    // //maa.maa_dmat = &pci_bus_dma_tag;
    // maa.maa_bst = (bus_space_tag_t)1; //x86_bus_space_mem;

    // // const struct cfdriver mmiocmdl_cd = *cfdriver_ioconf_mmiocmdl;
    // aprint_normal("%s, %d\n", cfdriver_ioconf_mmiocmdl[0]->cd_name, cfdriver_ioconf_mmiocmdl[0]->cd_ndevs);
    // const struct cfiattrdata* attrs = *(cfdriver_ioconf_mmiocmdl[0]->cd_attrs);
    // if(attrs[0].ci_name) aprint_normal("%s\n", attrs[0].ci_name);

    // // self = device_find_by_driver_unit("mmiocmdl", 0);
    // self = device_lookup(cfdriver_ioconf_mmiocmdl[0], 0);
    // if (!self)
    //     panic("no mmiocmdl.");
    // config_found_ia(self, "mmiobus", &maa, mmiocmdlprint);
    // //CFARGS(.iattr = "mmiobus")



    // if (!mainbus)
	// 	panic("no mainbus.  use maintaxi instead?");
    // int mmio_present = mmiocmdl_match(mainbus, cfdata_ioconf_mmiocmdl, &maa);
    // if(mmio_present) {
    //     aprint_normal("hello?\n");
    //     //config_found_ia(mainbus, "mmiobus", &maa, NULL);
    //     config_attach(mainbus, cfdata_ioconf_mmiocmdl, &maa, NULL, NULL);
    // }
}
