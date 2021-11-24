// fs.cpp: File System

#include "sfs/fs.h"

// #include <algorithm>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// Global Variables
Disk *device;                       /* Store the mounted disk */
SuperBlock metadata;                /* Store SuperBlock to access metadata */
bool *block_in_use;                 /* Free block bit map. Stores true if the block is in use; false otherwise */
int *inode_counter;                 /* Stores the amount of inodes contained per inode block */

// Debug file system -----------------------------------------------------------

void debug(Disk *disk) {
    Block block;

    // Read Superblock
    
    disk->readDisk(disk, 0, block.Data);
    
    printf("SuperBlock:\n");
    printf("    magic number is %s\n", (block.Super.MagicNumber == MAGIC_NUMBER? "valid":"invalid"));
    printf("    %u blocks\n"         , block.Super.Blocks);
    printf("    %u inode blocks\n"   , block.Super.InodeBlocks);
    printf("    %u inodes\n"         , block.Super.Inodes);
    
    
    // Read Inode blocks
    int numberOfInodeBlocks = block.Super.InodeBlocks;
    for(int inodeBlock=1; inodeBlock<=numberOfInodeBlocks; inodeBlock++){
        disk->readDisk(disk, inodeBlock, block.Data);
        for(unsigned int currentInode=0;currentInode<INODES_PER_BLOCK;currentInode++){
            Inode inode = block.Inodes[currentInode];
            if(inode.Valid == 0)
                continue;
            printf("Inode %d:\n", currentInode);
            printf("    size: %u bytes\n", inode.Size);
            printf("    direct blocks:");
            for(unsigned int directBlock=0; directBlock<POINTERS_PER_INODE; directBlock++){
                if(inode.Direct[directBlock]){
                    printf(" %u", inode.Direct[directBlock]);
                }
            }
            printf("\n");
            int indirectBlock = inode.Indirect;
            if(indirectBlock){
                Block iBlock;
                printf("    indirect block: %d\n", indirectBlock);
                disk->readDisk(disk, indirectBlock, iBlock.Data);
                printf("    indirect data blocks:");
                for(unsigned int indirectDataBlock=0; indirectDataBlock<POINTERS_PER_BLOCK; indirectDataBlock++){
                    if(iBlock.Pointers[indirectDataBlock]){
                        printf(" %u", iBlock.Pointers[indirectDataBlock]);
                    }
                }
                printf("\n");
            }
        }
    }
}

// Format file system ----------------------------------------------------------

bool format(Disk *disk) {
    // Do nothing and return false if the disk is already mounted
    if(disk->mounted(disk)){
        return false;
    }

    // Initialize SuperBlock
    Block superBlock;
    memset(&superBlock, 0, BLOCK_SIZE);
    SuperBlock super;
    super.MagicNumber = MAGIC_NUMBER;
    super.Blocks = disk->Blocks;
    super.InodeBlocks = ceil(disk->Blocks*0.10);
    super.Inodes = INODES_PER_BLOCK * super.InodeBlocks;
    superBlock.Super = super;
    disk->writeDisk(disk, 0, superBlock.Data);

    // Clear Inode Blocks
    int numberOfInodeBlocks = super.InodeBlocks;
    for(int inodeBlock=1; inodeBlock<=numberOfInodeBlocks; inodeBlock++){
        Block iBlock;
        // Clear all inodes in block
        for(unsigned int currentInode=0; currentInode<INODES_PER_BLOCK; currentInode++){
            iBlock.Inodes[currentInode].Valid = 0;
            iBlock.Inodes[currentInode].Size = 0;
            for(unsigned int directBlock=0; directBlock<POINTERS_PER_INODE; directBlock++){
                iBlock.Inodes[currentInode].Direct[directBlock] = 0;
            }
            iBlock.Inodes[currentInode].Indirect = 0;
            //TODO: Clear Double Indirect
        }
        disk->writeDisk(disk, inodeBlock, iBlock.Data);
    }

    // Clear Data Blocks
    for(unsigned int dataBlock=super.InodeBlocks+1; dataBlock<super.Blocks; dataBlock++){
        Block dBlock;
        memset(&dBlock.Data, 0, BLOCK_SIZE);
        disk->writeDisk(disk, dataBlock, dBlock.Data);
    }

    return true;
}



// Mount file system -----------------------------------------------------------

bool mount(Disk *disk) {
    // Do nothing and return false if the disk is already mounted
    if(disk->mounted(disk)){
        return false;
    }

    // Read SuperBlock
    Block block;
    disk->readDisk(disk, 0, block.Data);

    // Verify if SuperBlock information is valid
    if(block.Super.MagicNumber != MAGIC_NUMBER || block.Super.InodeBlocks != ceil(block.Super.Blocks*0.10) ||
    block.Super.Inodes != block.Super.InodeBlocks*INODES_PER_BLOCK) {
        return false;
    }

    // Set device and mount
    disk->mount(disk);
    device = disk;

    // Copy metadata
    metadata = block.Super;

    // Allocate free block bitmap
    block_in_use = (bool *) calloc(metadata.Blocks, sizeof(bool));
    inode_counter = (int *) calloc(metadata.InodeBlocks, sizeof(int));
    block_in_use[0] = true;    /* Mark SuperBlock as in use */

    // Read Inode Blocks and mark the ones in use. Also mark any direct or indirect block referenced.
    int numberOfInodeBlocks = metadata.InodeBlocks;
    for(int inodeBlock=1; inodeBlock<=numberOfInodeBlocks; inodeBlock++){
        disk->readDisk(disk, inodeBlock, block.Data);
        for(unsigned int currentInode=0; currentInode<INODES_PER_BLOCK; currentInode++) {
            Inode inode = block.Inodes[currentInode];
            if(inode.Valid == 0){
                continue;
            }
            // Valid Inode was found. Mark the Inode Block as in use and add it to count
            block_in_use[inodeBlock] = true;
            inode_counter[inodeBlock - 1]++;
            // Mark any direct pointer as in use
            for(unsigned int direct=0; direct<POINTERS_PER_INODE; direct++){
                if(inode.Direct[direct]){
                    if(inode.Direct[direct] < metadata.Blocks) {
                        block_in_use[direct] = true;
                    } else {
                        return false;
                    }
                }
            }
            // Mark any indirect block and all pointed blocks as in use
            if(inode.Indirect){
                if(inode.Indirect < metadata.Blocks){
                    block_in_use[inode.Indirect] = true;
                    Block indirectBlock;
                    device->readDisk(disk, inode.Indirect, indirectBlock.Data);
                    for(unsigned int indirect=0; indirect<POINTERS_PER_BLOCK; indirect++){
                        if(indirectBlock.Pointers[indirect]){
                            if(indirectBlock.Pointers[indirect] < metadata.Blocks){
                                block_in_use[indirectBlock.Pointers[indirect]] = true;
                            } else{
                                return false;
                            }
                        }
                    }
                } else{
                    return false;
                }
            }
        }
    }

    return true;
}

// Create inode ----------------------------------------------------------------

size_t create() {
    // Locate free inode in inode table

    // Record inode if found
    return 0;
}

// Remove inode ----------------------------------------------------------------

bool removeInode(size_t inumber) {
    // Load inode information

    // Free direct blocks

    // Free indirect blocks

    // Clear inode in inode table
    return true;
}

// Inode stat ------------------------------------------------------------------

size_t stat(size_t inumber) {
    // Load inode information
    return 0;
}

// Read from inode -------------------------------------------------------------

size_t readInode(size_t inumber, char *data, size_t length, size_t offset) {
    // Load inode information

    // Adjust length

    // Read block and copy to data
    return 0;
}

// Write to inode --------------------------------------------------------------

size_t writeInode(size_t inumber, char *data, size_t length, size_t offset) {
    // Load inode
    
    // Write block and copy to data
    return 0;
}
