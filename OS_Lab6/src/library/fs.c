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
bool mounted = false;
Disk *device;                       /* Store the mounted disk */
SuperBlock metadata;                /* Store SuperBlock to access metadata */
bool *block_in_use;                 /* Free block bit map. Stores true if the block is in use; false otherwise */
int *inode_counter;                 /* Stores the amount of inodes contained per inode block */

// Useful functions
bool load_inode(size_t inumber, Inode *node);
void read_helper(int blocknum, int offset, size_t *length, char **data, char **tail);

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
    mounted = true;

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
    // Fail if device not mounted
    if(!mounted){
        return -1;
    }

    // Locate free inode in inode table
    Block block;
    for(int inodeBlock=1; inodeBlock<=metadata.InodeBlocks; inodeBlock++){
        // Continue if block is full
        if(inode_counter[inodeBlock-1] == INODES_PER_BLOCK)
            continue;

        // Find the first empty inode in inode table
        device->readDisk(device, inodeBlock, block.Data);
        for(unsigned int currentInode=0; currentInode<INODES_PER_BLOCK; currentInode++){
            if(!block.Inodes[currentInode].Valid) {
                // Make a valid inode with default values
                block.Inodes[currentInode].Valid = true;
                block.Inodes[currentInode].Size = 0;
                block.Inodes[currentInode].Indirect = 0;
                for(unsigned int direct=0; direct<POINTERS_PER_INODE; direct++){
                    block.Inodes[currentInode].Direct[direct] = 0;
                }
                // Update global variables
                block_in_use[inodeBlock] = true;
                inode_counter[inodeBlock-1]++;
                // Write to disk
                device->writeDisk(device, inodeBlock, block.Data);

                return ((inodeBlock-1)*INODES_PER_BLOCK + currentInode);
            }
        }
    }

    // Record inode if found
    return -1;
}

// Remove inode ----------------------------------------------------------------

bool removeInode(size_t inumber) {
    if(!mounted){
        return false;
    }
    // Load inode information
    Inode inode;
    if(!load_inode(inumber, &inode)){
        return false;
    }

    //Verify if block is emptied by removal and mark as not in use if necessary
    int inodeBlock = inumber/INODES_PER_BLOCK;
    if(--inode_counter[inodeBlock] == 0){
        block_in_use[inodeBlock+1] = false;
    }

    // Reset inode info
    inode.Valid = false;
    inode.Size = 0;

    // Free direct blocks
    for(unsigned int direct=0; direct<POINTERS_PER_INODE; direct++){
        inode.Direct[direct] = 0;
        block_in_use[inode.Direct[direct]] = false;
    }

    // Free indirect blocks
    if(inode.Indirect){
        Block indirect;
        device->readDisk(device, inode.Indirect, indirect.Data);
        block_in_use[inode.Indirect] = false;
        inode.Indirect = 0;
        for(unsigned int i=0; i<POINTERS_PER_BLOCK; i++){
            if(indirect.Pointers[i])
                block_in_use[indirect.Pointers[i]] = false;
        }
    }

    // Clear inode in inode table
    Block block;
    int inodeIndex = inumber==0? 0:inumber%INODES_PER_BLOCK;
    device->readDisk(device, inodeBlock+1, block.Data);
    block.Inodes[inodeIndex] = inode;
    device->writeDisk(device, inodeBlock+1, block.Data);

    return true;
}

// Inode stat ------------------------------------------------------------------

size_t stat(size_t inumber) {
    if(!mounted){
        return -1;
    }

    // Load inode information
    Inode inode;
    return load_inode(inumber, &inode)? inode.Size:-1;
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

// Useful function implementations ------------------------------------------------
bool load_inode(size_t inumber, Inode *node){
    if(!mounted)
        return false;
    if(inumber < 0 || inumber > metadata.Inodes)
        return false;

    // Calculate inode location
    int inodeBlock = inumber/INODES_PER_BLOCK;
    int inodeIndex = inumber==0? 0:inumber%INODES_PER_BLOCK;

    // Load into *node
    if(inode_counter[inodeBlock]){
        Block block;
        device->readDisk(device, inodeBlock+1, block.Data);
        if(block.Inodes[inodeIndex].Valid){
            *node = block.Inodes[inodeIndex];
            return true;
        }
    }

    return false;
}

void read_helper(int blocknum, int offset, size_t *length, char **data, char **tail){
    /* Read block from disk and change pointers accordingly */
    device->readDisk(device, blocknum, *tail);
    *data += offset;
    *tail += BLOCK_SIZE;
    *length -= (BLOCK_SIZE-offset);
    return;
}