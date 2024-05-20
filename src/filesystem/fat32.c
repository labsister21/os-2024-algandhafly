#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/driver/disk.h"
#include "header/driver/clock.h"

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
    return cluster * CLUSTER_BLOCK_COUNT;
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
    memcpy(dir_table->table[0].name, name, 8);
    memset(dir_table->table[0].ext, 0, 3);
    dir_table->table[0].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[0].cluster_high = (parent_dir_cluster >> 16) & 0xFFFF;
    dir_table->table[0].cluster_low = parent_dir_cluster & 0xFFFF;

    memcpy(dir_table->table[1].name, "..", 2);
    memset(dir_table->table[1].ext, 0, 3);
    dir_table->table[1].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[1].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[1].cluster_high = (parent_dir_cluster >> 16) & 0xFFFF;
    dir_table->table[1].cluster_low = parent_dir_cluster & 0xFFFF;

    for (int i = 2; i < DIRECTORY_TABLE_SIZE; i++) 
    {
        dir_table->table[i].user_attribute = !UATTR_NOT_EMPTY; // UATTR_EMPTY
    }

    fat32driver_state.fat_table.cluster_map[parent_dir_cluster] = FAT32_FAT_END_OF_FILE; 
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
    init_directory_table(&fat32driver_state.dir_table_buf, "root\0\0\0\0", ROOT_CLUSTER_NUMBER);
    write_clusters(fat32driver_state.dir_table_buf.table, ROOT_CLUSTER_NUMBER, 1);
    
    write_clusters(&fat32driver_state.fat_table, 1, 1);
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
    
    for (uint8_t i = 0; i < DIRECTORY_TABLE_SIZE; i++) {
        if (memcmp(table[i].name, request.name, 8) == 0) {
            if (table[i].attribute == ATTR_SUBDIRECTORY) {
                read_clusters(request.buf, table[i].cluster_low, 1);
                return 0; // success
            }
            else return 1; // not a folder
        }
    }
    return 2; // not found
}

int8_t read_directory_child(struct FAT32DriverRequest request){
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = fat32driver_state.dir_table_buf.table;
    
    for (uint8_t i = 2; i < DIRECTORY_TABLE_SIZE; i++) {
        if (memcmp(table[i].name, request.name, 8) == 0) {
            if (table[i].attribute == ATTR_SUBDIRECTORY) {
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

    for(uint8_t i = 0; i < DIRECTORY_TABLE_SIZE;i++){
        // If name same and extension same
        if(table[i].user_attribute == UATTR_NOT_EMPTY && !memcmp(table[i].name,request.name,8) && !memcmp(table[i].ext,request.ext,3)){
            
            // Check if it's a file or not
            if(table[i].attribute == ATTR_SUBDIRECTORY){
                return 1; // Is not a File
            }
            // Check if the request has enough buffer size
            if(request.buffer_size < table[i].filesize){
                return 2; // Not enough buffer size
            }
            // Load the requested buffer
            uint16_t count_cluster = (table[i].filesize + CLUSTER_SIZE - 1) / CLUSTER_SIZE; // Rounded-up division
            uint16_t cluster = table[i].cluster_low;
            for(uint16_t j=0;j<count_cluster;j++){ // Bisa pake while (cluster != END_OF_BLALAL)
                read_clusters(request.buf + j * CLUSTER_SIZE,cluster,1);
                cluster = fat32driver_state.fat_table.cluster_map[cluster];
            }

            return 0;
        }
    }   
    // Not found
    return 3;
}




/**
 * FAT32 write, write a file or folder to file system.
 *
 * @param request All attribute will be used for write, buffer_size == 0 then create a folder / directory
 * @return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
 */
int8_t write(struct FAT32DriverRequest request){
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = fat32driver_state.dir_table_buf.table;

    if(fat32driver_state.fat_table.cluster_map[request.parent_cluster_number] != FAT32_FAT_END_OF_FILE){
        return 2;
    }

    uint16_t cluster_amount = (request.buffer_size + CLUSTER_SIZE) / CLUSTER_SIZE; // Rounded-up division
    uint16_t locations[cluster_amount];
    uint16_t current_cluster = 3; // Starts from cluster 3
    uint16_t cluster_count = 0;
    uint16_t directory_location = -1;

    // Iterate directory table
    for(uint16_t i=2; i<DIRECTORY_TABLE_SIZE;i++){
        // If there is an entry empty
        if(table[i].user_attribute != UATTR_NOT_EMPTY){
            // Iterate cluster
            while(cluster_count < cluster_amount && current_cluster < CLUSTER_MAP_SIZE){
                // Check if cluster available
                if(fat32driver_state.fat_table.cluster_map[current_cluster] == FAT32_FAT_EMPTY_ENTRY){
                    locations[cluster_count] = current_cluster;
                    cluster_count++;
                }
                current_cluster++;
            }
            directory_location = i;
            break;
        }

        // If name already exists
        if(!memcmp(table[i].name,request.name,8)){
            // Check if folder
            if(request.buffer_size == 0 && table[i].attribute == ATTR_SUBDIRECTORY){
                return 1;
            }
            // Check if file
            else if(memcmp(table[i].ext, request.ext, 3)){
                return 1;
            }
        }
    }
    // If cluster available is less then it needed
    if (cluster_count < cluster_amount){
        return -1;
    }

    // Set designated entry table value, excluding file or folder check
    memcpy(table[directory_location].name,request.name,8);
    table[directory_location].filesize = request.buffer_size;
    table[directory_location].cluster_low = locations[0] & 0xFFFF;
    table[directory_location].cluster_high = (locations[0] >> 16) & 0xFFFF;
    table[directory_location].user_attribute = UATTR_NOT_EMPTY;
    
    // time_t t;
    // get_indonesian_time(&t);
    // uint16_t date = date_to_byte(&t);
    // uint16_t time = time_to_byte(&t);

    // table[directory_location].create_date = date;
    // table[directory_location].create_time = time;
    // table[directory_location].modified_date = date;
    // table[directory_location].modified_time = time;


    // === Create whether file or folder ===
    // Requested only want folder
    if(request.buffer_size == 0){
        table[directory_location].attribute = ATTR_SUBDIRECTORY;
        write_clusters(table, request.parent_cluster_number, 1);
        fat32driver_state.fat_table.cluster_map[locations[0]] = FAT32_FAT_END_OF_FILE;
        write_clusters(&fat32driver_state.fat_table,1,1); 
        
        struct FAT32DirectoryTable new_dir_table;
        init_directory_table(&new_dir_table, request.name, request.parent_cluster_number);
        // it'll be way easier if we use locations[0] instead of request.parent_cluster_number for the 3rd parameters. but the sample.bin is like that so lets just follow it
        write_clusters(new_dir_table.table, locations[0], 1);
    }
    // Requested only want file
    else{
        table[directory_location].attribute = !ATTR_SUBDIRECTORY;
        memcpy(table[directory_location].ext,request.ext,3);

        // Iterate locations
        for(uint8_t j = 0; j < cluster_count;j++){
            // Write requested buffer into each location
            write_clusters(request.buf + j * CLUSTER_SIZE,locations[j],1);

            // Set next cluster as in linked list
            if(j == cluster_amount - 1){
                fat32driver_state.fat_table.cluster_map[locations[j]] = FAT32_FAT_END_OF_FILE;
            }
            else {
                fat32driver_state.fat_table.cluster_map[locations[j]] = locations[j+1];
            }
        }
        // Update file allocation table
        write_clusters(&fat32driver_state.fat_table,1,1); 
    }
    write_clusters(table,request.parent_cluster_number,1); // Update directory table

    return 0;
}


/**
 * FAT32 delete, delete a file or empty directory (only 1 DirectoryEntry) in file system.
 *
 * @param request buf and buffer_size is unused
 * @return Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
 */
int8_t delete(struct FAT32DriverRequest request){
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = fat32driver_state.dir_table_buf.table;

    /**
     * Search the requested folder / file in directory table
    */
    int designated_index = -1;
    bool isFound = false;
    bool isFolder = false;
    int idx = 0;
    // Iterate directory table
    while(!isFound && idx < DIRECTORY_TABLE_SIZE){
        // If the same name existed
        if(memcmp(table[idx].name,request.name,8) == 0){
            // If it's a folder
            if(table[idx].attribute == ATTR_SUBDIRECTORY){
                isFolder = true;
                designated_index = idx;
                if(table[idx].user_attribute == UATTR_NOT_EMPTY) isFound = true;
            } 
            // If it's a file and has the same extension
            else if(memcmp(table[idx].ext,request.ext,3) == 0){
                isFolder = false;
                designated_index = idx;
                if(table[idx].user_attribute == UATTR_NOT_EMPTY) isFound = true;
            }
        }
        idx++;
    }
    // If the requested file / folder doesn't exist
    if(!isFound){
        return 1;
    }

    /**
     * Deletion of the requested file / folder
    */

    // Folder Deletion
    if(isFolder){
        /**
         * Check if the folder's directory table has folder / file
        */

        // Accessing the folder's directory table
        struct FAT32DirectoryTable dt;
        read_clusters(&dt,table[designated_index].cluster_low,1);

        // Iterate the folder's directory table
        for(int i=2;i<DIRECTORY_TABLE_SIZE;i++){
            // If there exists an entry that is not empty
            if(dt.table[i].user_attribute == UATTR_NOT_EMPTY){
                return 2;
            }
        }

        // Set the entry into empty
        fat32driver_state.fat_table.cluster_map[table[designated_index].cluster_low] = FAT32_FAT_EMPTY_ENTRY;
        table[designated_index].user_attribute = !UATTR_NOT_EMPTY;
        table[designated_index].undelete = true; // For enabling restoration

        // Rewrite the clusters back into storage
        write_clusters(&fat32driver_state.fat_table,1,1);
        write_clusters(&fat32driver_state.dir_table_buf,request.parent_cluster_number,1);
        return 0;
    }

    // File Deletion
    else {
        /**
         * Clear the file's partitioned clusters in File Allocation Table
        */

        // Enumerate all the used clusters into used_clusters list
        int cluster_amount = (table[designated_index].filesize + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
        uint32_t used_clusters[cluster_amount];
        uint32_t current_cluster = table[designated_index].cluster_low;
        int idx = 0;
        while(current_cluster != FAT32_FAT_END_OF_FILE){
            used_clusters[idx] = current_cluster;
            current_cluster = fat32driver_state.fat_table.cluster_map[current_cluster];
            idx++;
        }


        // Set the cluster_map of used_cluster into FAT32_FAT_EMPTY_ENTRY
        for(int i=0;i<cluster_amount;i++){
            fat32driver_state.fat_table.cluster_map[used_clusters[i]] = FAT32_FAT_EMPTY_ENTRY;
        }
        // Set the entry into empty
        table[designated_index].user_attribute = !UATTR_NOT_EMPTY;
        table[designated_index].undelete = true; // For enabling restoration

        // Rewrite the clusters back into storage
        write_clusters(&fat32driver_state.fat_table,1,1);
        write_clusters(&fat32driver_state.dir_table_buf,request.parent_cluster_number,1);
        return 0;
    }
    return -1;
}


/**
 * @return Error code: 0 success - 1 source not found in cluster - 2 destination already exist
*/
int8_t move(struct FAT32DriverRequest request_src, struct FAT32DriverRequest request_dest){
    struct FAT32DirectoryTable src_dir_table;
    read_clusters(&src_dir_table, request_src.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *src_table = src_dir_table.table;
    
    for (uint8_t i = 2; i < DIRECTORY_TABLE_SIZE; i++) {
        if (!memcmp(src_table[i].name, request_src.name, 8) && !memcmp(src_table[i].ext, request_src.ext, 3)) {
            // Found source
            struct FAT32DirectoryTable dest_dir_table;
            read_clusters(&dest_dir_table, request_dest.parent_cluster_number, 1);
            struct FAT32DirectoryEntry *dest_table = dest_dir_table.table;

            // Check if destination already exist
            for(uint16_t j=2; j<DIRECTORY_TABLE_SIZE;j++){
                // found same name
                if(!memcmp(dest_table[j].name,request_src.name,8)){
                    // case moving folder and destination is folder
                    if(request_src.buffer_size == 0 && dest_table[j].attribute == ATTR_SUBDIRECTORY){
                        return 1;
                    }
                    // case moving file and destination is file
                    else if(request_src.buffer_size != 0 && memcmp(dest_table[j].ext, request_src.ext, 3)){
                        return 1;
                    }
                }
            }

            // Find empty destination
            for(uint16_t j=2; j<DIRECTORY_TABLE_SIZE;j++){
                // Found empty
                if(dest_table[j].user_attribute != UATTR_NOT_EMPTY){
                    // write metadata
                    memcpy(&dest_table[j], &src_table[i], sizeof(struct FAT32DirectoryEntry));
                    memcpy(&dest_table[j].name, request_dest.name, 8);
                    memcpy(&dest_table[j].ext, request_dest.ext, 3);
                    // mark source as empty
                    src_table[i].user_attribute = !UATTR_NOT_EMPTY;
                    write_clusters(src_table, request_src.parent_cluster_number, 1);
                    write_clusters(dest_table, request_dest.parent_cluster_number, 1);
                    return 0;
                }
            }
            return 1;
        }
    }
    return 2; // not found
}

// return 0 success - 1 not found
int8_t read_metadata(struct FAT32DriverRequest request){
    read_clusters(&fat32driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = fat32driver_state.dir_table_buf.table;
    
    for (uint8_t i = 0; i < DIRECTORY_TABLE_SIZE; i++) {
        if (memcmp(table[i].name, request.name, 8) == 0) {
            memcpy(request.buf, &table[i], sizeof(struct FAT32DirectoryEntry));
            return 0; // success
        }
    }
    return 1; // not found
}


