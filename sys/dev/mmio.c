#include <sys/types.h>
#include <sys/malloc.h>

#include <dev/mmio.h>
#include <dev/pci/virtiovar.h>


#include <uvm/uvm.h>
#include <uvm/uvm_prot.h>

void mmio_setb(mmioaddr_t addr, uint8_t val)
{
    *(uint8_t*)(addr) = val;
}

void mmio_setw(mmioaddr_t addr, uint16_t val)
{
    *(uint16_t*)(addr) = val;
}

void mmio_setl(mmioaddr_t addr, uint32_t val)
{
    *(uint32_t*)(addr) = val;
}

void mmio_setq(mmioaddr_t addr, uint64_t val)
{
    *(uint64_t*)(addr) = val;
}

uint8_t mmio_getb(mmioaddr_t addr)
{
    return (*(uint8_t*)(addr));
}

uint16_t mmio_getw(mmioaddr_t addr)
{
    return (*(uint16_t*)(addr));
}

uint32_t mmio_getl(mmioaddr_t addr)
{
    return (*(uint32_t*)(addr));
}

uint64_t mmio_getq(mmioaddr_t addr)
{
    return (*(uint64_t*)(addr));
}

extern struct vm_map *kernel_map;

//NOTE: IS THIS CORRECT? !!!!
mmioaddr_t mmio_map(uint64_t paddr, size_t size_bytes)
{
    vaddr_t* map_to = NULL;
    uvm_map_findspace(kernel_map, (vaddr_t)paddr, (vsize_t)size_bytes, map_to, NULL, 0, 0, 0);
    pmap_enter(kernel_map->pmap, *map_to, paddr, VM_PROT_WRITE | VM_PROT_READ, VM_PROT_WRITE | VM_PROT_READ);
    return (mmioaddr_t) *map_to;
}

void mmio_unmap(mmioaddr_t addr, size_t size_bytes)
{
    pmap_remove(kernel_map->pmap, (vaddr_t)addr, (vaddr_t)addr+size_bytes);
}



uint8_t get_status(mmio_dev* dev) {
    return mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_STATUS);
}

void set_status(mmio_dev* dev, uint8_t status) {
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_STATUS, status);
}

uint64_t get_available_features(mmio_dev* dev) {
    uint64_t features;

    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_DEVICE_FEATURES_SEL, 1);
    features = mmio_getl((uint8_t*)dev->addr_mmio +  MMIO_REGISTER_DEVICE_FEATURES);
    features <<= 32;

    mmio_setl((uint8_t*)dev->addr_mmio +  MMIO_REGISTER_DEVICE_FEATURES_SEL, 0);
    features |= mmio_getl((uint8_t*)dev->addr_mmio +  MMIO_REGISTER_DEVICE_FEATURES);

    return features;
}

void set_enabled_features(mmio_dev* dev, uint64_t features)
{
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_DRIVER_FEATURES_SEL, 1);
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_DRIVER_FEATURES, (uint32_t)(features >> 32));

    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_DRIVER_FEATURES_SEL, 0);
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_DRIVER_FEATURES, (uint32_t)(features & 0xFFFFFFFF));
}

void kick_queue(mmio_dev* dev, int queue_num)
{
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_NOTIFY, queue_num);
}

void select_queue(mmio_dev* dev, int queue_num)
{
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_SEL, queue_num);
    assert(!mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_READY));
}

uint16_t get_queue_size(mmio_dev* dev)
{
    return mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_NUM_MAX) & 0xffff;
}


// NOTE! Make sure conversion is correct !!!!!!!!!!
void setup_queue(mmio_dev* dev, struct virtqueue* queue)
{
    // Set size
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_NUM, queue->vq_num);

    // Pass addresses
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_DESC_LOW, (uint32_t)vtophys((vaddr_t)queue->vq_desc));
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_DESC_HIGH, (uint32_t)(vtophys((vaddr_t)queue->vq_desc) >> 32));

    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_DRIVER_LOW, (uint32_t)vtophys((vaddr_t)queue->vq_avail));
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_DRIVER_HIGH, (uint32_t)(vtophys((vaddr_t)queue->vq_avail) >> 32));

    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_DEVICE_LOW, (uint32_t)vtophys((vaddr_t)queue->vq_used));
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_DEVICE_HIGH, (uint32_t)(vtophys((vaddr_t)queue->vq_used) >> 32));
}

void activate_queue(mmio_dev* dev, int queue)
{
    // Make it ready
    select_queue(dev, queue);
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_QUEUE_READY, 1 );
}


uint8_t read_and_ack_isr(mmio_dev* dev)
{
    unsigned long status = mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_INTERRUPT_STATUS);
    mmio_setl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_INTERRUPT_ACK, status);
    return (status & MMIO_INT_VRING);
}

uint8_t read_config(mmio_dev* dev, uint32_t offset)
{
    return mmio_getb((uint8_t*)dev->addr_mmio + MMIO_REGISTER_CONFIG + offset);
}


//FIXME: Convert to Rumprun !!!!!!! SERIOUS
/*void register_interrupt(mmio_dev* dev, interrupt_factory irq_factory)
{
    _irq.reset(irq_factory.create_gsi_edge_interrupt());
}*/

bool parse_config(mmio_dev* dev)
{
    dev->addr_mmio = mmio_map(dev->dev_info.address, dev->dev_info.size);

    uint32_t magic = mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_MAGIC_VALUE);
    if (magic != MMIO_VIRT_MAGIC_VALUE) {
        return false;
    }

    // Check device version
    uint32_t dev_version = mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_VERSION);
    if (dev_version != 2) {
        //debugf( "Version %ld not supported!\n", version);
        return false;
    }

    dev->device_id = mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_DEVICE_ID);
    if (dev->device_id == 0) {
        //
        // virtio-mmio device with an ID 0 is a (dummy) placeholder
        // with no function. End probing now with no error reported.
        //debug( "Dummy virtio-mmio device detected!\n");
        return false;
    }
    dev->vendor_id = mmio_getl((uint8_t*)dev->addr_mmio + MMIO_REGISTER_VENDOR_ID);

    //debugf("Detected virtio-mmio device: (%ld,%ld)\n", dev->device_id, dev->vendor_id);
    return true;
}

#define VIRTIO_MMIO_DEVICE_CMDLINE_PREFIX "virtio_mmio.device="
struct dev_info_t* parse_mmio_device_info(char *cmdline)
{   //
    // Parse virtio mmio device information from command line
    // appended/prepended by VMMs like firecracker. After successfully
    // parsing any found mmio device info, remove it from the commandline.
    //
    // [virtio_mmio.]device=<size>@<baseaddr>:<irq>[:<id>]
    char *prefix_pos = strstr(cmdline,VIRTIO_MMIO_DEVICE_CMDLINE_PREFIX);
    if (!prefix_pos)
        return NULL;

    uint64_t size = 0;
    char *size_pos = prefix_pos + strlen(VIRTIO_MMIO_DEVICE_CMDLINE_PREFIX);
    //if (sscanf(size_pos,"%ld", &size) != 1)
    //    return NULL;
    size = 1;

    char *at_pos = strstr(size_pos,"@");
    if (!at_pos)
        return NULL;

    switch(*(at_pos - 1)) {
        case 'k':
        case 'K':
            size *= 1024;
            break;
        case 'm':
        case 'M':
            size *= (1024 * 1024);
            break;
        default:
            break;
    }

    uint64_t irq = 0, address = 0;
    //if (sscanf(at_pos, "@%lli:%u", &address, &irq) != 2)
    //    return NULL;
    address = 0xd0000000;
    irq = 5;

    // Find first white-character or null as an end of device description
    char * desc_end_pos = at_pos;
    while (*desc_end_pos != 0 && !isspace(*desc_end_pos))
        desc_end_pos++;

    // Remove conf info part from cmdline by copying over remaining part
    do {
       *prefix_pos = *desc_end_pos++;
    } while (*prefix_pos++);

    struct dev_info_t* result = (struct dev_info_t*) kern_malloc(sizeof(struct dev_info_t), 0);
    result->address = address;
    result->size = size;
    result->irq_no = irq;

    return result;
}

struct dev_info_t mmio_device_info_entries[128];
int mmio_device_info_entry_index;

void parse_mmio_device_configuration(char *cmdline)
{   //
    // We are assuming the mmio devices information is appended to the
    // command line (at least it is the case with the firecracker) so
    // once we parse those we strip it away so only plain OSv command line is left
    mmio_device_info_entry_index = 0;
    for(struct dev_info_t* device_info = parse_mmio_device_info(cmdline); device_info != NULL; device_info = parse_mmio_device_info(cmdline)) {
        mmio_device_info_entries[mmio_device_info_entry_index]= *device_info;
        mmio_device_info_entry_index++;
    }
}

/*void register_mmio_devices(hw::device_manager *dev_manager)
{
    for (auto info : *mmio_device_info_entries) {
        auto device = new mmio_device(info);
        if (device->parse_config()) {
            dev_manager->register_device(device);
        }
        else {
            delete device;
        }
    }
}*/
