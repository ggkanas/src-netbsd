#ifndef _DEV_VIRTIO_MMIOCMDLVAR_H_
#define	_DEV_VIRTIO_MMIOCMDLVAR_H_

int  mmiocmdl_probe(void);
//static void	mmiocmdl_attach(device_t, device_t, void *);

struct mmiocmdl_attach_args {
	const char *maa_name;
	bus_space_tag_t maa_bst;
	bus_dma_tag_t maa_dmat;
	int maa_phandle;
    uint64_t maa_addr;
    uint64_t maa_size;
    unsigned int maa_irq_no;
};

#endif /*_DEV_VIRTIO_MMIOCMDLVAR_H_*/
