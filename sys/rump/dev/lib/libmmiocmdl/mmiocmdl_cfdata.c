#include <sys/types.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/kmem.h>

#include <rump-sys/kern.h>

#include <dev/pci/pcivar.h>
#include <dev/virtio/mmiocmdlvar.h>

#include "ioconf.c"

extern struct cfdriver mmiocmdl_cd;
extern struct cfattach mmiocmdl_ca;



RUMP_COMPONENT(RUMP_COMPONENT_DEV)
{
    cfdata_t cf = cfdata_ioconf_mmiocmdl;
    struct cfdriver ** cds = kmem_zalloc(sizeof(struct cfdriver*), KM_SLEEP);
    struct cfattachinit *casi;
    struct cfattach ** cas = kmem_zalloc(sizeof(struct cfattach*), KM_SLEEP);
    //struct cfattachinit mmiocmdl_cas = {.cfai_name = "mmiocmdl", .cfai_list = cas};
    casi = kmem_alloc(sizeof(struct cfattachinit), KM_SLEEP);
    *casi = cfattach_ioconf_mmiocmdl[0];
    //mmiocmdl_cas;

    cds[0] = &mmiocmdl_cd;
    cas[0] = &mmiocmdl_ca;
    cf->cf_name = "mmiocmdl";
    cf->cf_atname = "mmmiocmdl";
    cf->cf_fstate = FSTATE_NOTFOUND;
    //cfdriver_ioconf_mmiocmdl[0]->cd_name = mmiocmdl_cd.cd_name;

    //config_init_component(cds,
    //    casi, cfdata_ioconf_mmiocmdl);
    config_cfdata_attach(cfdata_ioconf_mmiocmdl, 0);
	config_cfdriver_attach(cds[0]);
	config_cfattach_attach(casi->cfai_name, casi->cfai_list[0]);
    aprint_normal("cf name %s\n", cfdata_ioconf_mmiocmdl->cf_name);
    if(cds[0] != NULL) aprint_normal("cd not null\n");//, cfdriver_ioconf_mmiocmdl[0].cd_name);
    else aprint_normal("cd null\n");
    aprint_normal("cf atname %s\n", cfdata_ioconf_mmiocmdl->cf_atname);
    aprint_normal("cf unit %d\n", cf->cf_unit);
    aprint_normal("cf fstate %d\n", cf->cf_fstate);
    aprint_normal("cf flags %d\n", cf->cf_flags);
    if(cf->cf_pspec){
        aprint_normal("cf parent iattr %s\n", cf->cf_pspec->cfp_iattr);
        aprint_normal("cf parent cfp_parent %s\n", cf->cf_pspec->cfp_parent);
    }
    return;
}

RUMP_COMPONENT(RUMP_COMPONENT_DEV_AFTERMAINBUS) {
    int mmio_present = mmiocmdl_probe();
    //dgreg
    struct mmiocmdl_attach_args maa;
    device_t mainbus;
    //maa.maa_dmat = &pci_bus_dma_tag;
    maa.maa_bst = 1; //x86_bus_space_mem;
    mainbus = device_find_by_driver_unit("mainbus", 0);
	if (!mainbus)
		panic("no mainbus.  use maintaxi instead?");
    if(mmio_present) {
        aprint_normal("hello?\n");
        //config_found_ia(mainbus, "mmiobus", &maa, NULL);
        config_attach(mainbus, cfdata_ioconf_mmiocmdl, &maa, NULL);
    }
}
