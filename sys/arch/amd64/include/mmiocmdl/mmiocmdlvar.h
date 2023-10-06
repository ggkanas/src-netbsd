#ifndef _DEV_MMIOCMDLVAR_H_
#define	_DEV_MMIOCMDLVAR_H_

struct mmiocmdl_attach_args {
	const char *maa_name;
	bus_space_tag_t maa_bst;
	bus_dma_tag_t maa_dmat;
	int maa_phandle;
	int maa_mmio_index;
    uint64_t maa_addr;
    uint64_t maa_size;
    unsigned int maa_irq_no;
};

void mmiocmdlattach(int);

#endif /*_DEV_MMIOCMDLVAR_H_*/
