// ======================================================================
//                            AurorOS Driver
// ======================================================================
//
// => The NTFS filesystem driver.
//       This file is basically an implementation of NTFS filesystem
//       and provides API calls to read the files, etc.

#include "lib/ntfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to read a sector from the disk
int ntfs_read_sector(const char *device, uint64_t sector, uint8_t *buffer, size_t size);

// NTFS filesystem structure
typedef struct {
    ntfs_boot_sector_t boot_sector;
    uint64_t mft_start; // Starting sector of MFT
} ntfs_fs_t;

static ntfs_fs_t *ntfs_fs = NULL;

// Function to mount the NTFS filesystem
int ntfs_mount(const char *device) {
    ntfs_fs = (ntfs_fs_t *)malloc(sizeof(ntfs_fs_t));
    if (!ntfs_fs) {
        return -1;
    }

    // Read the boot sector from the device
    if (ntfs_read_sector(device, 0, (uint8_t *)&ntfs_fs->boot_sector, sizeof(ntfs_boot_sector_t)) != 0) {
        free(ntfs_fs);
        ntfs_fs = NULL;
        return -1;
    }

    // Check the validity of the NTFS boot sector
    if (ntfs_fs->boot_sector.end_of_sector_marker != 0xAA55) {
        free(ntfs_fs);
        ntfs_fs = NULL;
        return -1;
    }

    ntfs_fs->mft_start = ntfs_fs->boot_sector.mft_cluster * ntfs_fs->boot_sector.sectors_per_cluster;
    return 0; // Mounting successful
}

// Function to unmount the NTFS filesystem
int ntfs_unmount(void) {
    if (ntfs_fs) {
        free(ntfs_fs);
        ntfs_fs = NULL;
        return 0; // Unmounting successful
    }
    return -1; // NTFS was not mounted
}

// Function to read an MFT entry
int ntfs_read_mft_entry(uint64_t entry_number, ntfs_mft_entry_t *mft_entry) {
    if (!ntfs_fs) {
        return -1; // NTFS is not mounted
    }

    uint64_t sector = ntfs_fs->mft_start + (entry_number * sizeof(ntfs_mft_entry_t)) / 512;

    if (ntfs_read_sector("device", sector, (uint8_t *)mft_entry, sizeof(ntfs_mft_entry_t)) != 0) {
        return -1; // Error reading MFT entry
    }

    if (mft_entry->signature != 0x454C4946) { // Check for "FILE" signature
        return -1; // Invalid MFT entry
    }

    return 0; // MFT entry read successfully
}

// Function to find an attribute within an MFT entry
int ntfs_find_attribute(ntfs_mft_entry_t *mft_entry, uint32_t type, ntfs_attribute_t **attr) {
    uint8_t *ptr = (uint8_t *)mft_entry + mft_entry->offset_to_first_attribute;
    while (ptr < (uint8_t *)mft_entry + mft_entry->used_size) {
        ntfs_attribute_t *attribute = (ntfs_attribute_t *)ptr;
        if (attribute->type == type) {
            *attr = attribute;
            return 0;
        }
        ptr += attribute->length;
    }
    return -1; // Attribute not found
}

// Function to read a file from the NTFS filesystem
int ntfs_read_file(const char *path, void *buffer, size_t size) {
    if (!ntfs_fs) {
        return -1; // NTFS is not mounted
    }

    // Locate the file in the MFT (simplified for root directory entry 0)
    ntfs_mft_entry_t mft_entry;
    if (ntfs_read_mft_entry(0, &mft_entry) != 0) {
        return -1; // Error reading MFT entry
    }

    // Find the $DATA attribute
    ntfs_attribute_t *data_attr;
    if (ntfs_find_attribute(&mft_entry, 0x80, &data_attr) != 0) {
        return -1; // Data attribute not found
    }

    uint8_t *data = (uint8_t *)data_attr + data_attr->name_offset;
    if (data_attr->non_resident) {
        return -1; // Non-resident attributes not handled in this example
    }

    if (data_attr->length > size) {
        return -1; // Buffer size is too small
    }

    memcpy(buffer, data, data_attr->length);
    return data_attr->length; // Return the number of bytes read
}

// Function to read a sector from the disk
int ntfs_read_sector(const char *device, uint64_t sector, uint8_t *buffer, size_t size) {
    FILE *f = fopen(device, "rb");
    if (!f) {
        return -1; // Error opening device
    }

    if (fseek(f, sector * 512, SEEK_SET) != 0) {
        fclose(f);
        return -1; // Error seeking to sector
    }

    if (fread(buffer, 1, size, f) != size) {
        fclose(f);
        return -1; // Error reading sector
    }

    fclose(f);
    return 0; // Sector read successfully
}