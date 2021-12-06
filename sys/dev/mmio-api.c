#include <dev/mmio.h>
#include <dev/mmio-api.h>

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
