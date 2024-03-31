#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/driver/disk.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

struct FAT32DriverState fat32driver_state;


/* -- Driver Interfaces -- */

/**
 * Convert cluster number to logical block address
 * 
 * @param cluster Cluster number to convert
 * @return uint32_t Logical Block Address
 */
uint32_t cluster_to_lba(uint32_t cluster){
    return cluster * CLUSTER_SIZE;
}

/**
 * Initialize DirectoryTable value with 
 * - Entry-0: DirectoryEntry about itself
 * - Entry-1: Parent DirectoryEntry
 * 
 * @param dir_table          Pointer to directory table
 * @param name               8-byte char for directory name
 * @param parent_dir_cluster Parent directory cluster number
 */
void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    int nameLength = 8;
    memcpy(dir_table->table[0].name, name, nameLength);
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[0].cluster_high = (parent_dir_cluster >> 16) & 0xFFFF;
    dir_table->table[0].cluster_low = parent_dir_cluster & 0xFFFF;

    for (int i = 1; i < DIRECTORY_TABLE_SIZE; i++) 
    {
        dir_table->table[i].user_attribute = 0; // UATTR_EMPTY
    }
    write_clusters(dir_table->table, parent_dir_cluster, 1);


    fat32driver_state.fat_table.cluster_map[parent_dir_cluster] = FAT32_FAT_END_OF_FILE;
    write_clusters(&fat32driver_state.fat_table, 1, 1);
}

/**
 * Checking whether filesystem signature is missing or not in boot sector
 * 
 * @return True if memcmp(boot_sector, fs_signature) returning inequality
 */
bool is_empty_storage(void){
    struct BlockBuffer b;
    read_blocks(b.buf, BOOT_SECTOR, 1);
    return memcmp(b.buf, fs_signature, BLOCK_SIZE) != 0;
}

/**
 * Create new FAT32 file system. Will write fs_signature into boot sector and 
 * proper FileAllocationTable (contain CLUSTER_0_VALUE, CLUSTER_1_VALUE, 
 * and initialized root directory) into cluster number 1
 */
void create_fat32(void){
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    // AllocationTable
    fat32driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    fat32driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    fat32driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    for(int i = 3; i < CLUSTER_MAP_SIZE; i++) {
        fat32driver_state.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    }

    // DirectoryTable
    write_clusters(&fat32driver_state.fat_table, 1, 1);
    init_directory_table(&fat32driver_state.dir_table_buf, "root", 2);
}

/**
 * Initialize file system driver state, if is_empty_storage() then create_fat32()
 * Else, read and cache entire FileAllocationTable (located at cluster number 1) into driver state
 */
void initialize_filesystem_fat32(void){
    if(is_empty_storage()) create_fat32();
    else read_clusters(&fat32driver_state.fat_table, 1, 1);
}

/**
 * Write cluster operation, wrapper for write_blocks().
 * Recommended to use struct ClusterBuffer
 * 
 * @param ptr            Pointer to source data
 * @param cluster_number Cluster number to write
 * @param cluster_count  Cluster count to write, due limitation of write_blocks block_count 255 => max cluster_count = 63
 */
void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT * cluster_count);
}

/**
 * Read cluster operation, wrapper for read_blocks().
 * Recommended to use struct ClusterBuffer
 * 
 * @param ptr            Pointer to buffer for reading
 * @param cluster_number Cluster number to read
 * @param cluster_count  Cluster count to read, due limitation of read_blocks block_count 255 => max cluster_count = 63
 */
void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT * cluster_count);
}





/* -- CRUD Operation -- */

/**
 *  FAT32 Folder / Directory read
 *
 * @param request buf point to struct FAT32DirectoryTable,
 *                name is directory name,
 *                ext is unused,
 *                parent_cluster_number is target directory table to read,
 *                buffer_size must be exactly sizeof(struct FAT32DirectoryTable)
 * @return Error code: 0 success - 1 not a folder - 2 not found - -1 unknown
 */
int8_t read_directory(struct FAT32DriverRequest request){
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = fat32driver_state.dir_table_buf.table;
    
    for (uint8_t i = 1; i < DIRECTORY_TABLE_SIZE; i++) {
        if (memcmp(table[i].name, request.name, 8) == 0) {
            if (table[i].attribute == 1) {
                read_clusters(request.buf, table[i].cluster_low, 1);
                return 0; // success
            }
            else return 1; // not a folder
        }
    }
    return 2; // not found
}


/**
 * FAT32 read, read a file from file system.
 *
 * @param request All attribute will be used for read, buffer_size will limit reading count
 * @return Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown
 */
int8_t read(struct FAT32DriverRequest request){
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = fat32driver_state.dir_table_buf.table;

    for(uint8_t i = 1; i < DIRECTORY_TABLE_SIZE;i++){
        // If name same or extension same
        if(!memcmp(table[i].name,request.name,8) && !memcmp(table[i].ext,request.ext,3)){
            // Check if it's a file or not
            if(table[i].attribute == 1){
                return 1; 
            }
            // Check if the request has enough buffer size
            if(request.buffer_size < table[i].filesize){
                return 2;
            }
            // Load the requested buffer
            uint16_t count_cluster = (table[i].filesize + CLUSTER_SIZE - 1) / CLUSTER_SIZE; // Rounded-up division
            uint16_t cluster = table[i].cluster_low;
            for(uint16_t j=0;j<count_cluster;j++){
                read_clusters(request.buf + j * CLUSTER_SIZE,cluster,1);
                cluster = fat32driver_state.fat_table.cluster_map[cluster];
            }

            return 0;
        }
    }   
    // Not found
    return 0;
}

/**
 * FAT32 write, write a file or folder to file system.
 *
 * @param request All attribute will be used for write, buffer_size == 0 then create a folder / directory
 * @return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
 */
int8_t write(struct FAT32DriverRequest request){
    
}


/**
 * FAT32 delete, delete a file or empty directory (only 1 DirectoryEntry) in file system.
 *
 * @param request buf and buffer_size is unused
 * @return Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
 */
int8_t delete(struct FAT32DriverRequest request){

}