// ======================================================================
//                            AurorOS Driver
// ======================================================================

// => The AurorOS filesystem driver.
//      This file is basically an implementation of the AurorOS filesystem
//      and provides API calls to read files, etc.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ARFS_DEVICES 1024

struct arfs_inode {
    uint32_t inode_number;
    uint32_t size;
    uint64_t created_timestamp;
    uint64_t modified_timestamp;
};

struct arfs_file {
    struct arfs_inode inode;
    uint8_t *data;
    char *filename;
    struct arfs_file *next;
    int in_use;
};

struct arfs_directory {
    struct arfs_inode inode;
    struct arfs_directory *children;
    size_t num_children;
    char *dirname;
    struct arfs_directory *next;
    int in_use;
};

struct arfs_disk {
    struct arfs_file *file_list;
    struct arfs_directory *directory_list;
    uint32_t next_inode_number;
    char *disk_name;
};

static struct arfs_disk disks[MAX_ARFS_DEVICES];
static int num_disks = 0;

void arfs_init() {
    // Initialization code here
    for (int i = 0; i < MAX_ARFS_DEVICES; i++) {
        disks[i].file_list = NULL;
        disks[i].directory_list = NULL;
        disks[i].next_inode_number = 1;
        disks[i].disk_name = NULL;
    }
    num_disks = 0;
}

int arfs_add_disk(const char *disk_name) {
    if (num_disks >= MAX_ARFS_DEVICES) return -1;
    disks[num_disks].disk_name = strdup(disk_name);
    if (!disks[num_disks].disk_name) return -1;
    num_disks++;
    return 0;
}

struct arfs_disk *arfs_find_disk(const char *disk_name) {
    for (int i = 0; i < num_disks; i++) {
        if (strcmp(disks[i].disk_name, disk_name) == 0) {
            return &disks[i];
        }
    }
    return NULL;
}

int arfs_create_file(const char *disk_name, const char *filename) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return -1;

    struct arfs_file *current = disk->file_list;
    while (current) {
        if (strcmp(current->filename, filename) == 0) {
            return -1; // File already exists
        }
        current = current->next;
    }

    struct arfs_file *new_file = (struct arfs_file *)malloc(sizeof(struct arfs_file));
    if (!new_file) return -1;

    new_file->filename = strdup(filename);
    if (!new_file->filename) {
        free(new_file);
        return -1;
    }

    new_file->inode.inode_number = disk->next_inode_number++;
    new_file->inode.size = 0;
    new_file->inode.created_timestamp = time(NULL);
    new_file->inode.modified_timestamp = new_file->inode.created_timestamp;
    new_file->data = NULL;
    new_file->next = disk->file_list;
    new_file->in_use = 0;  // Initially the file is not in use
    disk->file_list = new_file;

    return 0;
}

int arfs_delete_file(const char *disk_name, const char *filename) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return -1;

    struct arfs_file **current = &disk->file_list;
    while (*current) {
        if (strcmp((*current)->filename, filename) == 0) {
            if ((*current)->in_use) return -2; // File is in use
            struct arfs_file *to_delete = *current;
            *current = (*current)->next;
            free(to_delete->filename);
            free(to_delete->data);
            free(to_delete);
            return 0;
        }
        current = &(*current)->next;
    }
    return -1; // File not found
}

struct arfs_file *arfs_open_file(const char *disk_name, const char *filename) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return NULL;

    struct arfs_file *current = disk->file_list;
    while (current) {
        if (strcmp(current->filename, filename) == 0) {
            current->in_use = 1;  // Mark file as in use
            return current;
        }
        current = current->next;
    }
    return NULL; // File not found
}

int arfs_read_file(struct arfs_file *file, void *buffer, size_t count) {
    if (!file || !buffer) return -1;
    if (count > file->inode.size) count = file->inode.size;
    memcpy(buffer, file->data, count);
    return (int)count;
}

int arfs_write_file(struct arfs_file *file, const void *buffer, size_t count) {
    if (!file || !buffer) return -1;
    uint8_t *new_data = (uint8_t *)realloc(file->data, count);
    if (!new_data) return -1;
    file->data = new_data;
    memcpy(file->data, buffer, count);
    file->inode.size = count;
    file->inode.modified_timestamp = time(NULL);
    return (int)count;
}

void arfs_close_file(struct arfs_file *file) {
    if (file) {
        file->in_use = 0;  // Mark file as not in use
    }
}

int arfs_create_directory(const char *disk_name, const char *dirname) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return -1;

    struct arfs_directory *current = disk->directory_list;
    while (current) {
        if (strcmp(current->dirname, dirname) == 0) {
            return -1; // Directory already exists
        }
        current = current->next;
    }

    struct arfs_directory *new_directory = (struct arfs_directory *)malloc(sizeof(struct arfs_directory));
    if (!new_directory) return -1;

    new_directory->dirname = strdup(dirname);
    if (!new_directory->dirname) {
        free(new_directory);
        return -1;
    }

    new_directory->inode.inode_number = disk->next_inode_number++;
    new_directory->inode.size = 0;
    new_directory->inode.created_timestamp = time(NULL);
    new_directory->inode.modified_timestamp = new_directory->inode.created_timestamp;
    new_directory->children = NULL;
    new_directory->num_children = 0;
    new_directory->next = disk->directory_list;
    new_directory->in_use = 0;  // Initially the directory is not in use
    disk->directory_list = new_directory;

    return 0;
}

int arfs_delete_directory(const char *disk_name, const char *dirname) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return -1;

    struct arfs_directory **current = &disk->directory_list;
    while (*current) {
        if (strcmp((*current)->dirname, dirname) == 0) {
            if ((*current)->in_use) return -2; // Directory is in use
            struct arfs_directory *to_delete = *current;
            *current = (*current)->next;
            free(to_delete->dirname);
            free(to_delete->children);
            free(to_delete);
            return 0;
        }
        current = &(*current)->next;
    }
    return -1; // Directory not found
}

struct arfs_inode *arfs_list_directory(const char *disk_name, const char *dirname, size_t *num_entries) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) {
        *num_entries = 0;
        return NULL;
    }

    struct arfs_directory *current = disk->directory_list;
    while (current) {
        if (strcmp(current->dirname, dirname) == 0) {
            *num_entries = current->num_children;
            return current->children;
        }
        current = current->next;
    }
    *num_entries = 0;
    return NULL; // Directory not found
}

int arfs_get_metadata(const char *disk_name, const char *name, struct arfs_inode *inode) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return -1;

    struct arfs_file *file = disk->file_list;
    while (file) {
        if (strcmp(file->filename, name) == 0) {
            *inode = file->inode;
            return 0;
        }
        file = file->next;
    }

    struct arfs_directory *directory = disk->directory_list;
    while (directory) {
        if (strcmp(directory->dirname, name) == 0) {
            *inode = directory->inode;
            return 0;
        }
        directory = directory->next;
    }

    return -1; // File or directory not found
}

int arfs_update_metadata(const char *disk_name, const char *name, const struct arfs_inode *inode) {
    struct arfs_disk *disk = arfs_find_disk(disk_name);
    if (!disk) return -1;

    struct arfs_file *file = disk->file_list;
    while (file) {
        if (strcmp(file->filename, name) == 0) {
            if (file->in_use) return -2; // File is in use
            file->inode = *inode;
            return 0;
        }
        file = file->next;
    }

    struct arfs_directory *directory = disk->directory_list;
    while (directory) {
        if (strcmp(directory->dirname, name) == 0) {
            if (directory->in_use) return -2; // Directory is in use
            directory->inode = *inode;
            return 0;
        }
        directory = directory->next;
    }

    return -1; // File or directory not found
}