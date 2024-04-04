#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/text/stringdrawer.h"
#include "header/interrupt/interrupt.h"
#include "header/interrupt/idt.h"
#include "header/driver/disk.h"
#include "header/driver/keyboard.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"


/**
 * Keyboard havent handle enter key perfectly:
 * - Not handling edge cases
 * 
*/

char frame[WIDTH][HEIGHT];

void handleEnterKey(int *row, int *col) {
    *row += 1;
    *col = 0;
    framebuffer_set_cursor(*row, *col);
}

void kernel_setup(void) {
    
    // === Milestone 0 ===
    // load_gdt(&_gdt_gdtr);

    // === Milestone 1.1 ===
    // framebuffer_clear();
    // drawWelcomeScreen();
    // framebuffer_set_cursor(79, 24);

    // === Milestone 1.2 ===
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // framebuffer_clear();
    // framebuffer_set_cursor(0, 0);
    // __asm__("int $0x4");

    // === Milestone 1.3 ===
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // activate_keyboard_interrupt();
    // framebuffer_clear();
    // framebuffer_set_cursor(0, 0);
        
    // int col = 0, row = 0;
    // keyboard_state_activate();
    // while (true) {
    //      char c;
    //      get_keyboard_buffer(&c);
    //      if (c == '\b') {
    //         if (col == 0 && row == 0) continue;

    //         if (col == 0 && row > 0) {
    //             row--;
    //             col = 79;
    //         } else {
    //             col--;
    //         }

    //         framebuffer_write(row, col, ' ', 0xF, 0);
    //         framebuffer_set_cursor(row, col);



    //      } else if (c == '\n') {
    //         handleEnterKey(&row, &col);
    //      }  else if (c) {
    //         framebuffer_write(row, col++, c, 0xF, 0);
    //         framebuffer_set_cursor(row, col);
    //      }

    //      if (col == 80) {
    //         col = 0;
    //         row++;
    //         framebuffer_set_cursor(row, col);
    //      }
    // }


    // === Milestone 1.4 ===
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // activate_keyboard_interrupt();
    // initialize_idt();


    // framebuffer_clear();
    // read_clusters(&fat32driver_state.dir_table_buf, 0, 1);
    // framebuffer_write_length(0, 0, fat32driver_state.dir_table_buf.table, 1*CLUSTER_MAP_SIZE, White, Black);
    // read_clusters(&fat32driver_state.dir_table_buf, 2, 1);
    // framebuffer_write_length(8, 0, fat32driver_state.dir_table_buf.table, 1*CLUSTER_MAP_SIZE, White, Black);
    // return;

    // [Test write_blocks]
    // struct BlockBuffer b;
    // for (int i = 0; i < CLUSTER_MAP_SIZE; i++) b.buf[i] = 3;
    // write_blocks(&b, 0, 1);

    // [Test read_blocks]
    // struct BlockBuffer b2;
    // read_blocks(&b2, 8, 1);
    // framebuffer_clear();
    // framebuffer_write_length(0, 0, b2.buf, 1*CLUSTER_MAP_SIZE, White, Black);
    // return;

    
    // [Test create_fat32]
    // initialize_filesystem_fat32();
    // framebuffer_clear();
    // framebuffer_write_length(0, 0, fat32driver_state.dir_table_buf.table, 1*CLUSTER_MAP_SIZE, White, Black);
    // return;
    
    // framebuffer_clear();
    // read_clusters(&fat32driver_state.fat_table, 1, 1);
    // for(int i = 0; i < HEIGHT*4; i++){
    //     framebuffer_write_int(i/4, (i % 4)*12, fat32driver_state.fat_table.cluster_map[i], White, Black);
    // }
    // return;

    // [Test read_clusters]
    // struct BlockBuffer b2;
    // read_clusters(&b2, 1, 1);
    // framebuffer_clear();
    // framebuffer_write_length(0, 0, b2.buf, CLUSTER_MAP_SIZE*CLUSTER_BLOCK_COUNT, White, Black);

    // [Test read_directory]
    // framebuffer_clear();
    // struct FAT32DriverRequest request;
    // request.parent_cluster_number = 2;
    // memcpy(request.name, "root", 4);
    // // memcpy(request.name, "blah", 4); // Case not found
    // // memcpy(request.name, "kano", 4); // Case not a folder
    // int8_t error_code = read_directory(request);
    // if(error_code == 0) {
    //     framebuffer_write_length(0, 0, "Found name:", 11, White, Black);
    //     framebuffer_write_length(0, 13, request.name, 11, White, Black);
    //     framebuffer_write_length(1, 0, "request.buf:", 12, White, Black);
    //     framebuffer_write_length(2, 0, request.buf, CLUSTER_MAP_SIZE, White, Black);
    // }
    // else {
    //     framebuffer_write_length(0, 0, "Error Code:", 11, White, Black);
    //     framebuffer_write_int(0, 12, error_code, White, Black);
    //     switch (error_code)
    //     {
    //         case 1: framebuffer_write_length(0, 14, "| Not a folder", 14, White, Black); break;
    //         case 2: framebuffer_write_length(0, 14, "| Not found", 11, White, Black); break;
    //     }
    //     framebuffer_write_length(1, 0, "fat32driver_state.dir_table_buf.table:", 38, White, Black);
    //     framebuffer_write_length(2, 0, fat32driver_state.dir_table_buf.table, CLUSTER_MAP_SIZE*CLUSTER_BLOCK_COUNT, White, Black);
    // }


    // [Test read]
    // framebuffer_clear();
    // struct FAT32DriverRequest request2;
    // memcpy(request2.name, "kano", 4);
    // // memcpy(request2.name, "blah", 4); // Case not found
    // // memcpy(request2.name, "folder1", 7); // Case not a file
    // memset(request2.ext, 0, 3);
    // request2.buffer_size = 0x13C5;
    // // request2.buffer_size = 0x13C4; // Case not enough buffer
    // request2.parent_cluster_number = 2; 

    // // memcpy(request2.name, "new1", 4); // for [Test write]
    // // memcpy(request2.ext, "txt", 3); // for [Test write]
    // // request2.parent_cluster_number = 8; // for [Test write]
    // int8_t error_code_2 = read(request2);
    // if(error_code_2 == 0) {
    //     framebuffer_write_length(0, 0, "Found name:", 11, White, Black);
    //     framebuffer_write_length(0, 13, request2.name, 11, White, Black);
    //     framebuffer_write_length(1, 0, "request.buf:", 12, White, Black);
    //     framebuffer_write_length(2, 0, request2.buf, CLUSTER_MAP_SIZE, White, Black);
    // }
    // else {
    //     framebuffer_write_length(0, 0, "Error Code:", 11, White, Black);
    //     framebuffer_write_int(0, 12, error_code_2, White, Black);
    //     switch (error_code_2)
    //     {
    //         case 1: framebuffer_write_length(0, 14, "| Not a file", 12, White, Black); break;
    //         case 2: framebuffer_write_length(0, 14, "| Not enough buffer", 19, White, Black); break;
    //         case 3: framebuffer_write_length(0, 14, "| Not found", 11, White, Black); break;
    //     }
    //     framebuffer_write_length(1, 0, "fat32driver_state.dir_table_buf.table:", 38, White, Black);
    //     framebuffer_write_length(2, 0, fat32driver_state.dir_table_buf.table, CLUSTER_MAP_SIZE*CLUSTER_BLOCK_COUNT, White, Black);
    // }
    // return;


    // [Test write]
    read_clusters(&fat32driver_state.fat_table, 1, 1); // Or call initialize_filesystem_fat32
    
    // framebuffer_clear();
    // for(int i = 0; i < HEIGHT*4; i++){
    //     framebuffer_write_int(i/4, (i % 4)*12, fat32driver_state.fat_table.cluster_map[i], White, Black);
    // }
    // return;

    framebuffer_clear();
    struct FAT32DriverRequest request3;
    memcpy(request3.name, "kano", 4);
    memset(request3.ext, 0, 3);
    request3.parent_cluster_number = 2;
    request3.buffer_size = 0x13C5;
    read(request3);

    memcpy(request3.name, "new1", 4);
    memcpy(request3.ext, "txt", 3);
    // request3.parent_cluster_number = 3; // Case is a file, therefore invalid parent cluster
    request3.parent_cluster_number = 8;
    int8_t error_code_3 = write(request3);
    if(error_code_3 == 0) {
        framebuffer_write_length(0, 0, "Write Successful:", 17, White, Black);
        framebuffer_write_length(0, 19, request3.name, 11, White, Black);
        framebuffer_write_length(1, 0, "request.buf:", 12, White, Black);
        framebuffer_write_length(2, 0, request3.buf, CLUSTER_MAP_SIZE, White, Black);
    }
    else {
        framebuffer_write_length(0, 0, "Error Code:", 11, White, Black);
        framebuffer_write_int(0, 12, error_code_3, White, Black);
        switch (error_code_3)
        {
            case 1: framebuffer_write_length(0, 14, "| File/Folder already exist", 27, White, Black); break;
            case 2: framebuffer_write_length(0, 14, "| Invalid parent cluster", 24, White, Black); break;
        }
        framebuffer_write_length(1, 0, "fat32driver_state.dir_table_buf.table:", 38, White, Black);
        framebuffer_write_length(2, 0, fat32driver_state.dir_table_buf.table, CLUSTER_MAP_SIZE*CLUSTER_BLOCK_COUNT, White, Black);
    }


    while (true);
}