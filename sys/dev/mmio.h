#ifndef VIRTIO_MMIO_DEVICE_HH
#define VIRTIO_MMIO_DEVICE_HH

#include <sys/types.h>

#include <dev/pci/virtiovar.h>

/* MMIO Device Register Layout */
#define MMIO_REGISTER_MAGIC_VALUE 0x000
#define   MMIO_VIRT_MAGIC_VALUE 0x74726976
#define MMIO_REGISTER_VERSION 0x004
#define   MMIO_MODERN_VERSION 0x2
#define MMIO_REGISTER_DEVICE_ID 0x008
#define   VIRTIO_DEVICE_TYPE_NET 1
#define   VIRTIO_DEVICE_TYPE_BLK 2
#define   VIRTIO_DEVICE_TYPE_CONS 3
#define   VIRTIO_DEVICE_TYPE_ENTROPY 4
#define   VIRTIO_DEVICE_TYPE_MEMORY_BALLOON_TRAD 5
#define   VIRTIO_DEVICE_TYPE_IO_MEM 6
#define   VIRTIO_DEVICE_TYPE_RPMSG 7
#define   VIRTIO_DEVICE_TYPE_SCSI 8
#define   VIRTIO_DEVICE_TYPE_9P_TRANSPORT 9
#define   VIRTIO_DEVICE_TYPE_MAC_80211_WLAN 10
#define   VIRTIO_DEVICE_TYPE_RPROC_SERIAL 11
#define   VIRTIO_DEVICE_TYPE_VIRTIO_CAIF 12
#define   VIRTIO_DEVICE_TYPE_MEMORY_BALLOON 13
#define   VIRTIO_DEVICE_TYPE_GPU 16
#define   VIRTIO_DEVICE_TYPE_TIMER_CLOCK 17
#define   VIRTIO_DEVICE_TYPE_INPUT 18
#define   VIRTIO_DEVICE_TYPE_SOCKET 19
#define   VIRTIO_DEVICE_TYPE_CRYPTO 20
#define   VIRTIO_DEVICE_TYPE_SIGNAL_DISTRIBUTION_MODULE 21
#define   VIRTIO_DEVICE_TYPE_PSTORE 22
#define   VIRTIO_DEVICE_TYPE_IOMMU 23
#define   VIRTIO_DEVICE_TYPE_MEMORY 24
#define MMIO_REGISTER_VENDOR_ID 0x00c
#define MMIO_REGISTER_DEVICE_FEATURES 0x010
#define MMIO_REGISTER_DEVICE_FEATURES_SEL 0x014
#define MMIO_REGISTER_DRIVER_FEATURES 0x020
#define MMIO_REGISTER_DRIVER_FEATURES_SEL 0x024
#define MMIO_REGISTER_QUEUE_SEL 0x030
#define MMIO_REGISTER_QUEUE_NUM_MAX 0x034
#define MMIO_REGISTER_QUEUE_NUM 0x038
#define MMIO_REGISTER_QUEUE_READY 0x044
#define MMIO_REGISTER_QUEUE_NOTIFY 0x050
#define MMIO_REGISTER_INTERRUPT_STATUS 0x060
#define MMIO_REGISTER_INTERRUPT_ACK 0x064
#define MMIO_REGISTER_STATUS 0x070
#define MMIO_REGISTER_QUEUE_DESC_LOW 0x080
#define MMIO_REGISTER_QUEUE_DESC_HIGH 0x084
#define MMIO_REGISTER_QUEUE_DRIVER_LOW 0x090
#define MMIO_REGISTER_QUEUE_DRIVER_HIGH 0x094
#define MMIO_REGISTER_QUEUE_DEVICE_LOW 0x0a0
#define MMIO_REGISTER_QUEUE_DEVICE_HIGH 0x0a4
#define MMIO_REGISTER_CONFIG_GENERATION 0x0fc
#define MMIO_REGISTER_CONFIG 0x100

#define MMIO_INT_VRING		(1 << 0)
#define MMIO_INT_CONFIG		(1 << 1)

typedef void* mmioaddr_t;

struct dev_info_t {
    uint64_t address;
    uint64_t size;
    unsigned int irq_no;
};

struct mmio_device {
    struct dev_info_t dev_info;
    uint16_t vendor_id;
    uint16_t device_id;
    uint64_t size;
    mmioaddr_t addr_mmio;
    //???*  irq;
};

typedef struct mmio_device mmio_dev;

void mmio_setb(mmioaddr_t addr, uint8_t val);
void mmio_setw(mmioaddr_t addr, uint16_t val);
void mmio_setl(mmioaddr_t addr, uint32_t val);
void mmio_setq(mmioaddr_t addr, uint64_t val);
uint8_t mmio_getb(mmioaddr_t addr);
uint16_t mmio_getw(mmioaddr_t addr);
uint32_t mmio_getl(mmioaddr_t addr);
uint64_t mmio_getq(mmioaddr_t addr);

mmioaddr_t mmio_map(uint64_t paddr, size_t size_bytes);
void mmio_unmap(mmioaddr_t addr, size_t size_bytes);

uint8_t get_status(mmio_dev* dev);
void set_status(mmio_dev* dev, uint8_t status);
uint64_t get_available_features(mmio_dev* dev);
void set_enabled_features(mmio_dev* dev, uint64_t features);
void kick_queue(mmio_dev* dev, int queue_num);
void select_queue(mmio_dev* dev, int queue_num);
uint16_t get_queue_size(mmio_dev* dev);
void setup_queue(mmio_dev* dev, struct virtqueue* queue);
void activate_queue(mmio_dev* dev, int queue);
uint8_t read_and_ack_isr(mmio_dev* dev);
uint8_t read_config(mmio_dev* dev, uint32_t offset);
//void register_interrupt(mmio_dev* dev, interrupt_factory irq_factory);
bool parse_config(mmio_dev* dev);

struct dev_info_t* parse_mmio_device_info(char *cmdline);
//void parse_mmio_device_configuration(char *cmdline);



#endif //VIRTIO_MMIO_DEVICE_HH
