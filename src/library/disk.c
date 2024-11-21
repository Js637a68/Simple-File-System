/* disk.c: SimpleFS disk emulator */

#include "sfs/disk.h"
#include "sfs/logging.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

/* Internal Prototyes */

bool    disk_sanity_check(Disk *disk, size_t blocknum, const char *data);

/* External Functions */

/**
 *
 * Opens disk at specified path with the specified number of blocks by doing
 * the following:
 *
 *  1. Allocates Disk structure and sets appropriate attributes.
 *
 *  2. Opens file descriptor to specified path.
 *
 *  3. Truncates file to desired file size (blocks * BLOCK_SIZE).
 *
 * @param       path        Path to disk image to create.
 * @param       blocks      Number of blocks to allocate for disk image.
 *
 * @return      Pointer to newly allocated and configured Disk structure (NULL
 *              on failure).
 **/
Disk *	disk_open(const char *path, size_t blocks) {
    Disk *disk = (Disk*)malloc(sizeof(Disk));
    if(disk == NULL) return NULL;
    disk->fd = open(path, O_RDWR);
    if (ftruncate(disk->fd, blocks * BLOCK_SIZE) < 0) 
    {
        close(disk->fd);
        free(disk);
        return NULL;
    }
    disk->blocks = blocks;
    disk->writes = disk->reads = 0;
    return disk;
}
/**
 * Close disk structure by doing the following:
 *
 *  1. Close disk file descriptor.
 *
 *  2. Report number of disk reads and writes.
 *
 *  3. Releasing disk structure memory.
 *
 * @param       disk        Pointer to Disk structure.
 */
void	disk_close(Disk *disk) {
    if(disk != NULL)
    {
        if(disk->fd >= 0) close(disk->fd);
        printf("%lu disk block reads\n", disk->reads);
        printf("%lu disk block writes\n", disk->writes);
        free(disk);
    }
}

/**
 * Read data from disk at specified block into data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified block.
 *
 *  3. Reading from block to data buffer (must be BLOCK_SIZE).
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes read.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_read(Disk *disk, size_t block, char *data) {
    if(!disk_sanity_check(disk, block, data)) return DISK_FAILURE;

    off_t position = (off_t)block * BLOCK_SIZE;
    if (lseek(disk->fd, position, SEEK_SET) == (off_t)-1) {
        return DISK_FAILURE;
    }

    ssize_t bytesRead = read(disk->fd, data, BLOCK_SIZE);
    if (bytesRead < 0) {
        return DISK_FAILURE;
    }

    disk->reads++;
    return bytesRead;
}

/**
 * Write data to disk at specified block from data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified block.
 *
 *  3. Writing data buffer (must be BLOCK_SIZE) to disk block.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes written.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_write(Disk *disk, size_t block, char *data) {
    if(!disk_sanity_check(disk, block, data)) return DISK_FAILURE;

    off_t position = (off_t)block * BLOCK_SIZE;
    if (lseek(disk->fd, position, SEEK_SET) == (off_t)-1) {
        return DISK_FAILURE;
    }

    ssize_t bytesWritten = write(disk->fd, data, BLOCK_SIZE);
    if (bytesWritten < 0) {
        return DISK_FAILURE;
    }

    disk->writes++;
    return bytesWritten;
}

/* Internal Functions */

/**
 * Perform sanity check before read or write operation:
 *
 *  1. Check for valid disk.
 *
 *  2. Check for valid block.
 *
 *  3. Check for valid data.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Whether or not it is safe to perform a read/write operation
 *              (true for safe, false for unsafe).
 **/
bool    disk_sanity_check(Disk *disk, size_t block, const char *data) {
    if(disk != NULL && data != NULL)
    {
        if( block < disk->blocks)
            return true;
    }
    return false;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
