#include <machine/mmio.h>
#include <machine/mmio-api.h>

struct dev_info_t mmio_device_info_entries[128];
int mmio_device_info_entry_index;

void rump_parse_mmio_device_configuration(char *cmdline)
{   //
    // We are assuming the mmio devices information is appended to the
    // command line (at least it is the case with the firecracker) so
    // once we parse those we strip it away so only plain OSv command line is left

    mmioaddr_t addr = mmio_map(0xd0000000);
    //bmk_printf("magic value: %d\n", mmio_getl((uint8_t*)addr + 0));
    uint32_t magic = mmio_getl((uint8_t*)addr + 0);
    bmk_printf("magic value: %ld\n", (uint64_t)addr);
    bmk_printf("magic value: %d\n", magic);

    // mmio_device_info_entry_index = 0;
    // for(struct dev_info_t* device_info = parse_mmio_device_info(cmdline); device_info != NULL; device_info = parse_mmio_device_info(cmdline)) {
    //     mmio_device_info_entries[mmio_device_info_entry_index]= *device_info;
    //     mmio_device_info_entry_index++;
    // }
}