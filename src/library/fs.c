/* fs.c: SimpleFS file system */

#include "sfs/fs.h"
#include "sfs/logging.h"
#include "sfs/utils.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* External Functions */

/**
 * Debug FileSystem by doing the following
 *
 *  1. Read SuperBlock and report its information.
 *
 *  2. Read Inode Table and report information about each Inode.
 *
 * @param       disk        Pointer to Disk structure.
 **/
void    fs_debug(Disk *disk) {
    Block block;
    /* Read SuperBlock */
    assert(disk_read(disk, 0, block.data) != DISK_FAILURE);

    printf("SuperBlock:\n");
    printf("    magic number is valid\n");
    printf("    %u blocks\n"         , block.super.blocks);
    if(block.super.inodes == 0) return;
    printf("    %u inode blocks\n"   , block.super.inode_blocks);
    printf("    %u inodes\n"         , block.super.inodes);


    /* Read Inodes */
    uint32_t inode_blocks = block.super.inode_blocks;
    for(uint32_t index = 1; index <= inode_blocks; ++index)
    {
        memset(block.data, 0, BLOCK_SIZE);
        assert(disk_read(disk, index, block.data) != DISK_FAILURE);
        for(int i = 0; i < INODES_PER_BLOCK; ++i)
        {
            const Inode node = block.inodes[i];
            if(!node.valid) continue;
            printf("Inode %d:\n", i);    
            printf("    size: %u bytes\n", node.size);
            printf("    direct blocks:");
            for(uint32_t j = 0; j < POINTERS_PER_INODE; ++j)
                if(node.direct[j])
                    printf(" %u", node.direct[j]);
            printf("\n");
            if(node.indirect != 0)
            {
                printf("    indirect block: %u\n", node.indirect);
                Block point = {0};
                assert(disk_read(disk, node.indirect, point.data) != DISK_FAILURE);
                printf("    indirect data blocks:");
                for(uint32_t z = 0; z < POINTERS_PER_BLOCK; ++z)
                {
                    if(!point.pointers[z]) break;
                    printf(" %u", point.pointers[z]);
                }
                printf("\n");
            }
        }
    }
}

/**
 * Format Disk by doing the following:
 *
 *  1. Write SuperBlock (with appropriate magic number, number of blocks,
 *  number of inode blocks, and number of inodes).
 *
 *  2. Clear all remaining blocks.
 *
 * Note: Do not format a mounted Disk!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not all disk operations were successful.
 **/
bool    fs_format(FileSystem *fs, Disk *disk) {
    if(!fs) return false;
    size_t blocks = disk->blocks;
    if (blocks < 2) {
        return false; // 磁盘块数不足，无法格式化
    }

    if(fs->disk != NULL) return false;

    // 计算inode块数和总inode数
    size_t inode_blocks = blocks / 10 == 0 ? 1 : blocks /  10; // 将10%的块分配给inode块
    size_t inodes = inode_blocks * INODES_PER_BLOCK;

    Block block = {0};
    block.super.magic_number = MAGIC_NUMBER;
    block.super.blocks = blocks;
    block.super.inode_blocks = inode_blocks;
    block.super.inodes = inodes;
    // 写入超级块到第0块
    if (disk_write(disk, 0, block.data) == DISK_FAILURE) {
        return false; // 写入失败
    }

    memset(block.data, 0, BLOCK_SIZE);
    for(int i = 1; i < blocks; ++i)
    {
        assert(disk_write(disk, i, block.data) != DISK_FAILURE);
    }

    return true;
}

/**
 * Mount specified FileSystem to given Disk by doing the following:
 *
 *  1. Read and check SuperBlock (verify attributes).
 *
 *  2. Verify and record FileSystem disk attribute. 
 *
 *  3. Copy SuperBlock to FileSystem meta data attribute
 *
 *  4. Initialize FileSystem free blocks bitmap.
 *
 * Note: Do not mount a Disk that has already been mounted!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not the mount operation was successful.
 **/
bool    fs_mount(FileSystem *fs, Disk *disk) {
    if(fs == NULL || fs->disk != NULL)
        return false;

    Block block = {0};
    assert(disk_read(disk, 0, block.data) != DISK_FAILURE);
    if (block.super.magic_number != MAGIC_NUMBER || block.super.blocks != disk->blocks)
        return false;
    if(block.super.inodes != block.super.inode_blocks * INODES_PER_BLOCK) return false;

    
    fs->disk = disk;
    fs->meta_data = block.super;
    return fs_initialize_free_block_bitmap(fs); 
}

/**
 * Unmount FileSystem from internal Disk by doing the following:
 *
 *  1. Set FileSystem disk attribute.
 *
 *  2. Release free blocks bitmap.
 *
 * @param       fs      Pointer to FileSystem structure.
 **/
void    fs_unmount(FileSystem *fs) {
    if(fs->free_blocks)
    {
        free(fs->free_blocks);
    }
    fs->free_blocks = NULL;
    fs->disk = NULL;
}

/**
 * Allocate an Inode in the FileSystem Inode table by doing the following:
 *
 *  1. Search Inode table for free inode.
 *
 *  2. Reserve free inode in Inode table.
 *
 * Note: Be sure to record updates to Inode table to Disk.
 *
 * @param       fs      Pointer to FileSystem structure.
 * @return      Inode number of allocated Inode.
 **/
ssize_t fs_create(FileSystem *fs) {
    uint32_t inode_blocks = fs->meta_data.inode_blocks;
    for(uint32_t i = 1; i <= inode_blocks; ++i)
    {
        Block inode_table = {0};
        assert(disk_read(fs->disk, i, inode_table.data) != DISK_FAILURE);
        for(uint32_t j = 0; j < INODES_PER_BLOCK; ++j)
        {
            ssize_t index = (i-1)*INODES_PER_BLOCK+j;
            if(!inode_table.inodes[j].valid) 
            {
                Inode node;
                memset(&node, 0, sizeof(Inode));
                node.valid = 1;
                node.indirect = 0;
                assert(fs_save_inode(fs, index, &node));
                return index;
            }
        }
    }
    return -1;
}

/**
 * Remove Inode and associated data from FileSystem by doing the following:
 *
 *  1. Load and check status of Inode.
 *
 *  2. Release any direct blocks.
 *
 *  3. Release any indirect blocks.
 *
 *  4. Mark Inode as free in Inode table.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Whether or not removing the specified Inode was successful.
 **/
bool    fs_remove(FileSystem *fs, size_t inode_number) {
    Inode inode = {0};
    if(!fs_load_inode(fs, inode_number, &inode)) return false;

    
    for(uint32_t i = 0 ; i < POINTERS_PER_INODE; ++i)
    {
        if(inode.direct[i] == 0) break;
        fs->free_blocks[inode.direct[i]] = false;
    }
    if(inode.indirect != 0)
    {
        Block block = {0};
        assert(disk_read(fs->disk, inode.indirect, block.data) != DISK_FAILURE);  
        uint32_t i = 0;
        for(; i < POINTERS_PER_BLOCK; ++i) 
        {
            if(block.pointers[i] == 0) break;
            fs->free_blocks[block.pointers[i]] = false;
        }
        if(i!= 0)
            fs->free_blocks[inode.indirect] = false;
    }

    memset(&inode, 0, sizeof(Inode));
    assert(fs_save_inode(fs, inode_number, &inode));
    return true;
}

/**
 * Return size of specified Inode.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Size of specified Inode (-1 if does not exist).
 **/
ssize_t fs_stat(FileSystem *fs, size_t inode_number) {
    Inode inode = {0};
    if(!fs_load_inode(fs, inode_number, &inode)) return -1;
    return inode.size;
}
/**
 * Read from the specified Inode into the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously read blocks and copy data to buffer.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to read data from.
 * @param       data            Buffer to copy data to.
 * @param       length          Number of bytes to read.
 * @param       offset          Byte offset from which to begin reading.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_read(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset) {
    Inode node = {0};
    if (!fs_load_inode(fs, inode_number, &node)) return -1;

    if (offset >= node.size) return 0;
    if (length == 0) return 0;
    if (offset + length >= node.size) length = node.size - offset;

    size_t sum = 0;
    uint32_t i = offset / BLOCK_SIZE;
    for(; i < POINTERS_PER_INODE && sum < length; ++i)
    {
        if (node.direct[i] == 0) break; 
        char block_data[BLOCK_SIZE];
        memset(block_data, 0, BLOCK_SIZE);
        assert(disk_read(fs->disk, node.direct[i], block_data) != DISK_FAILURE);

        size_t read_size = (length - sum) < BLOCK_SIZE ? (length - sum) : BLOCK_SIZE;
        memcpy(data + sum, block_data + offset % BLOCK_SIZE, read_size);
        sum += read_size;
        offset += read_size;
    }
    if (node.indirect != 0 && sum < length) 
    {
        Block block;
        i -= POINTERS_PER_INODE;
        assert(i >= 0);
        assert(disk_read(fs->disk, node.indirect, block.data) != DISK_FAILURE);
        for (; i < POINTERS_PER_BLOCK && sum < length; ++i) {
            if (block.pointers[i] == 0) break; 
            char block_data[BLOCK_SIZE];
            memset(block_data, 0, BLOCK_SIZE);
            assert(disk_read(fs->disk, block.pointers[i], block_data) != DISK_FAILURE); 

            size_t read_size = (length - sum) < BLOCK_SIZE ? (length - sum) : BLOCK_SIZE;
            memcpy(data + sum, block_data + offset % BLOCK_SIZE, read_size);
            sum += read_size;
            offset += read_size;
        }
    }
    return sum; 
}
/**
 * Write to the specified Inode from the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously copy data from buffer to blocks.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to write data to.
 * @param       data            Buffer with data to copy
 * @param       length          Number of bytes to write.
 * @param       offset          Byte offset from which to begin writing.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_write(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset) {
    Inode node;
    if(!fs_load_inode(fs, inode_number, &node)) return -1;
    if(offset > node.size) return 0;

    ssize_t point = offset / BLOCK_SIZE;
    ssize_t written = 0;
    for(; point < POINTERS_PER_INODE && length > 0; point++)
    {
        Block block;
        memset(block.data, 0, BLOCK_SIZE);
        if(node.direct[point] == 0) 
        {
            node.direct[point] = fs_allocate_free_block(fs);
            if(node.direct[point] == 0) goto last;  
        }
        else assert(disk_read(fs->disk, node.direct[point], block.data) != DISK_FAILURE);

        const ssize_t remaining = BLOCK_SIZE - offset % BLOCK_SIZE;
        const ssize_t copy_size = remaining < length? remaining : length;
        memcpy(block.data + offset % BLOCK_SIZE, data + written, copy_size);
        assert(disk_write(fs->disk, node.direct[point], block.data) != DISK_FAILURE);
        length -= copy_size;
        offset += copy_size;
        written += copy_size;
    }
    if(length > 0)
    {
        point -= POINTERS_PER_INODE;
        assert(point >= 0);
        Block block;
        memset(block.data, 0, BLOCK_SIZE);
        if(node.indirect == 0) 
        {
            assert(offset%BLOCK_SIZE == 0);
            node.indirect = fs_allocate_free_block(fs);
            if(node.indirect == 0) goto last;
        }
        else assert(disk_read(fs->disk, node.indirect, block.data) != DISK_FAILURE);
        while(length > 0)
        {
            Block data_block;
            memset(data_block.data, 0, BLOCK_SIZE);
            if(block.pointers[point] == 0) 
            {
                block.pointers[point] = fs_allocate_free_block(fs);
                if(block.pointers[point] == 0) break;
            }
            else assert(disk_read(fs->disk, block.pointers[point], data_block.data) != DISK_FAILURE);

            const ssize_t remaining = BLOCK_SIZE - offset % BLOCK_SIZE;
            const ssize_t copy_size = remaining < length? remaining : length;
            memcpy(data_block.data + offset%BLOCK_SIZE, data + written, copy_size);     
            assert(disk_write(fs->disk, block.pointers[point], data_block.data) != DISK_FAILURE);
            point++;
            length -= copy_size;
            offset += copy_size;
            written += copy_size;
        }
        assert(disk_write(fs->disk, node.indirect, block.data) != DISK_FAILURE);
    }
last:
    node.size = offset > node.size ? offset : node.size;
    assert(fs_save_inode(fs, inode_number, &node));
    return written;
}

bool    fs_initialize_free_block_bitmap(FileSystem *fs)
{
    uint32_t inode_blocks = fs->meta_data.inode_blocks;
    fs->free_blocks = malloc(sizeof(bool) * fs->meta_data.blocks);
    if(!fs->free_blocks) return false;
    memset(fs->free_blocks, 0, sizeof(bool) * fs->meta_data.blocks);
    fs->free_blocks[0] = true;     // superblock使用
    for(uint32_t i = 1; i <= inode_blocks; ++i)
    {
        fs->free_blocks[i] = true;     // 被inode使用
        Block b = {0};
        // 读取inode表
        assert(disk_read(fs->disk, i, b.data) != DISK_FAILURE);

        for(int p = 0; p < INODES_PER_BLOCK; ++p)
        {
            const Inode inode = b.inodes[p];
            if(!inode.valid) continue;
            // 直接block
            for(uint32_t j = 0 ; j < POINTERS_PER_INODE; ++j)
            {
                if(inode.direct[j] == 0) break;
                fs->free_blocks[inode.direct[j]] = true;
            }
            // 间接block
            if(inode.indirect == 0) continue;
            fs->free_blocks[inode.indirect] = true;
            Block indirect_point = {0};
            assert(disk_read(fs->disk, inode.indirect, indirect_point.data) != DISK_FAILURE);

            for(int j = 0; j < POINTERS_PER_BLOCK; ++j)
            {
                if(indirect_point.pointers[j] == 0) break;
                fs->free_blocks[indirect_point.pointers[j]] = true;
            }
        }
    }
    return true;
}

uint32_t fs_allocate_free_block(FileSystem *fs)
{
    for (uint32_t i = 1; i < fs->meta_data.blocks; i++) {
        if (!fs->free_blocks[i]) {        
            fs->free_blocks[i] = true;
            return i;
        }
    }
    return 0; // No free block available
}

bool    fs_load_inode(FileSystem *fs, size_t inode_number, Inode *node)
{
    Block block = {0};
    if(inode_number >= fs->meta_data.inodes) return false;
    assert(disk_read(fs->disk, inode_number/INODES_PER_BLOCK + 1, block.data) != DISK_FAILURE);
    if(!block.inodes[inode_number].valid) return false;
    *node = block.inodes[inode_number % INODES_PER_BLOCK];
    return true;
}

bool    fs_save_inode(FileSystem *fs, const size_t inode_number, Inode *node)
{
    Block block = {0};
    if(inode_number >= fs->meta_data.inodes) return false;
    assert(disk_read(fs->disk, inode_number/INODES_PER_BLOCK + 1, block.data) != DISK_FAILURE);
    memcpy(&block.inodes[inode_number % INODES_PER_BLOCK], node, sizeof(Inode));
    assert(disk_write(fs->disk, inode_number/INODES_PER_BLOCK + 1, block.data) != DISK_FAILURE);
    return true;
}
/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
