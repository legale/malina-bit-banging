/* Raspberry Pi (malina) NAND flash bit-banging programmer
 * <malina.c>
 *
 * Copyright (c) 2021, legale <legale.legale@gmail.com>
 */

#include <stdio.h>
int fileno(FILE *stream); /* declare fileno() to get file descriptor from FILE pointer */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h> /* integer types uint* */
#include <time.h> /* clock() nanosleep() */
#include <string.h>


#include "malina.h"
#include "yarpio/yarpio.h"
#include "samsung_nand.h"

const char *commands[] = {
	"stat", "all_in", "all_out", "speed_test", "blink [pin] [delay]", 
    "read_id", 
	"read_page [page]",
    "flash_dump [offset] [length]",
    "write_file [filename] [from page] [length]", 
    "write_file [filename] [page] [length]", 
    "erase_block [block]",
    "erase_flash [from_offset] [to_offset]",
    "pud_off [pin]", "pud_down [pin]", "pud_up [pin]", "in [pin]", "out [pin]", 
    "high [pin]", "low [pin]", "switch [pin]", "--help", "-h"
};
const int commands_size = sizeof commands / sizeof commands[0];

void help(){
	printf("Available commands: %d\n", commands_size);
    for(int i = 0; i < commands_size; i++){
        PRINTER("%s\n", commands[i]);
    }
}
 
static void *gpio; /* pointer to gpio */

int main(int argc, char **argv){
  /* prepare gpio pointer for direct register access */
  setup_io(&gpio); /* pass the pointer to gpio pointer */
  if (argc == 1) help(); 
  for (int i = 1; i < argc; i++){
	DEBUG_PRINT("arg %d: %s\n", i, argv[i]);
	if      (strcmp("stat", argv[i])  == 0)  { status(gpio); }
	else if (strcmp("all_in", argv[i]) == 0) { all_in(gpio); }
	else if (strcmp("all_out", argv[i]) == 0) { all_out(gpio); }
	else if (strcmp("speed_test", argv[i]) == 0){ speed_test(gpio); }
	else if (strcmp("blink", argv[i]) == 0 && (i + 2) < argc){ blink(gpio, atoi(argv[++i]), strtoul(argv[++i], NULL,10)); } 
	else if (strcmp("read_id", argv[i]) == 0){ read_id(gpio); }
	else if (strcmp("write_file", argv[i]) == 0 && (i + 2) < argc){ write_file(argv[++i], atoi(argv[++i]), atoi(argv[++i])); }
	else if (strcmp("write_page", argv[i]) == 0 && (i + 2) < argc){ write_page(argv[++i], atoi(argv[++i]), atoi(argv[++i])); }
    else if (strcmp("read_page", argv[i]) == 0 && (i + 1) < argc){ read_page(gpio, atoi(argv[++i])); }
	else if (strcmp("flash_dump", argv[i]) == 0 && (i + 2) < argc){ flash_dump(atoi(argv[++i]), atoi(argv[++i])); }
    else if (strcmp("erase_block", argv[i]) == 0 && (i + 1) < argc){ erase_block(gpio, atoi(argv[++i])); }
    else if (strcmp("erase_flash", argv[i]) == 0 && (i + 2) < argc){ erase_flash(atoi(argv[++i]), atoi(argv[++i])); }
    else if (strcmp("pud_off", argv[i]) == 0 && (i + 1) < argc){ pud_off(gpio, atoi(argv[++i])); }
	else if (strcmp("pud_down", argv[i]) == 0  && (i + 1) < argc){ pud_down(gpio, atoi(argv[++i])); }
	else if (strcmp("pud_up", argv[i]) == 0 && (i + 1) < argc){ pud_up(gpio, atoi(argv[++i])); }
	else if (strcmp("in", argv[i]) == 0 && (i + 1) < argc){ gpio_in(gpio, atoi(argv[++i])); }
	else if (strcmp("out", argv[i]) == 0 && (i + 1) < argc){ gpio_out(gpio, atoi(argv[++i])); }
	else if (strcmp("high", argv[i]) == 0 && (i + 1) < argc){ gpio_high(gpio, atoi(argv[++i])); }
	else if (strcmp("low", argv[i]) == 0 && (i + 1) < argc){ gpio_low(gpio, atoi(argv[++i])); }
	else if (strcmp("switch", argv[i]) == 0 && (i + 1) < argc){ gpio_switch(gpio, atoi(argv[++i])); }
	else if (strcmp("-h", argv[i]) == 0) {help(); }
	else if (strcmp("--help", argv[i]) == 0) {help(); }
	else 	{PRINTER("%s command not found\n", argv[i]);}
  }
  return 0;
}


void nand_write_byte(void *gpio, uint8_t byte){
    GPIO_CLR_BYTE(gpio, IO0);
    GPIO_SET(gpio, WE, 0);
    nsleep(tWP); /* WE pulse width. WE low time delay  */
    GPIO_SET_BYTE(gpio, IO0, byte);
    nsleep(tWH); /* WE high time delay  */
    GPIO_SET(gpio, WE, 1);
        DEBUG_PRINT("\t\tIO W: 0x%02X 0x%02X\n", byte, GPIO_GET_BYTE(gpio, IO0));          
}


uint8_t nand_read_byte(void *gpio){
    uint8_t byte = 0;
    GPIO_SET(gpio, RE, 0);
    nsleep(tRP); /* RE pulse width. RE low time delay */
    byte = GPIO_GET_BYTE(gpio, IO0);
    GPIO_SET(gpio, RE, 1);
    DEBUG_PRINT("\t\tIO R: 0x%02X\n", byte);          
    nsleep(tREH); /* RE high hold time */
    return byte;
}


void nand_command_send(void *gpio, uint8_t byte){
    GPIO_SET(gpio, CLE, 1);
    nand_write_byte(gpio, byte);
    GPIO_SET(gpio, CLE, 0);
}


void nand_address_send(void *gpio, uint8_t *addr, int size){
    GPIO_SET(gpio, ALE, 1);
    for (int i = 0; i < size; i++){
        nand_write_byte(gpio, addr[i]);
    }
    GPIO_SET(gpio, ALE, 0);
}

void nand_rw_mode(void *gpio, int wp_pin){
    GPIO_SET(gpio,  CE,   0);
    GPIO_SET(gpio,  WE,   1);
    GPIO_SET(gpio,  RE,   1);
    GPIO_SET(gpio,  WP,   wp_pin);
}

uint8_t nand_read_status(void *gpio){
    GPIO_SET(gpio, CLE, 1); /* command cycle */
    nand_write_byte(gpio, 0x70); /* status command */
    GPIO_SET(gpio, CLE, 0);
    GPIO_IN_BYTE(gpio, IO0);  /* set IO in */  
    nsleep(tWHR); /* time WE high to RE low */
    return nand_read_byte(gpio);

}

void read_id(void *gpio){
    /* set gpio out */
    all_out(gpio);
    /* standby mode. CE high */
    GPIO_HIGH(gpio, CE); 
    uint8_t byte = 0; /* data buffer */
    nand_rw_mode(gpio, 0); /* enable rw mode WP pin = 0 (write protect) */

    uint8_t addr[1] = { 0x0 };
    nand_command_send(gpio, 0x90); /* read chip id command. 0x90 */
    nand_address_send(gpio, (uint8_t *)&addr, 1); /* address 0x0 */

    GPIO_IN_BYTE(gpio, IO0); /* switch data pins in before reading */
    nsleep(tWHR); /* delay between WE high and RE low */
    for(int i = 0; i < 5; i++){
        byte = nand_read_byte(gpio);
        PRINTER("%i 0x%02X\n", i, byte);
    }
    GPIO_LOW(gpio, CE); /* Chip Enabled low */
}


/* function to write nand page from file */
void write_page(char *filename, uint32_t page, uint32_t length){
    /* set gpio out */
    all_out(gpio);
    /* standby mode. CE high */
    GPIO_HIGH(gpio, CE);
    GPIO_IN(gpio, RB); /* set Read/Busy pin in */

    uint8_t byte = 0;
    FILE *fp;
    fp = fopen(filename, "rb");
    if(fp == NULL){
            printf("Unable to open file %s\n", filename);
                exit(1);
    }
    if(length == 0){ 
            printf("length = 0, nothing to write.\n"); 
                exit(1);
    }
    printf("Writing file: %s of length: %d page: %d from: 0x%08X to: 0x%08X\n", filename, length, page, page * PAGESIZE, page * PAGESIZE + length);


    /* convert page to address */
    uint8_t addr[4] = {
        0,
        0,
        page & 0xff,
        page >> 8 & 0xff
    };
    for(int i = 0; i < 4; i++){
        DEBUG_PRINT("%d 0x%02X\n", i, addr[i]);
    }

    nand_rw_mode(gpio, 1); /* enable rw mode. WP pin high */

    nand_command_send(gpio, 0x80); /* data input command. 0x80 */
    nand_address_send(gpio, (uint8_t *)&addr, 4);

    nsleep(tADL); /* delay Address Data Loading  */

    /* send data */
    int i = 0;
    while(i < length){
        byte = fgetc(fp);
        if(feof(fp)) break;
        nand_write_byte(gpio, byte);
        DEBUG_PRINT("%d '%c' \n", i++, byte);
    }
    fclose(fp);

    nand_command_send(gpio, 0x10); /*program command. 0x10 */
    nsleep(tWB + tPROG);  /* time WE high to busy + program time */

    /* get status */
    printf("Program status: 0x%02X\n", nand_read_status(gpio));
}

/* function to write nand page from file */
void write_page_internal(char *pagebuf, uint32_t page, uint32_t length){
    /* set gpio out */
    all_out(gpio);
    /* standby mode. CE high */
    GPIO_HIGH(gpio, CE);
    GPIO_IN(gpio, RB); /* set Read/Busy pin in */

    PRINTER("Writing page: %d from: 0x%08X to: 0x%08X\n", page, page * PAGESIZE, page * PAGESIZE + length);
    
    /* convert page to address */
    uint8_t addr[4] = {
            0,
            0,
            page & 0xff,
            page >> 8 & 0xff
    };

    
    nand_rw_mode(gpio, 1); /* enable rw mode. WP pin high */

    nand_command_send(gpio, 0x80); /* data input command. 0x80 */
    nand_address_send(gpio, (uint8_t *)&addr, 4);

    nsleep(tADL); /* delay Address Data Loading  */

    /* send data */
    int i = 0;
    while(i < length){
        nand_write_byte(gpio, pagebuf[i++]);
    }
    
    nand_command_send(gpio, 0x10); /*program command. 0x10 */
    nsleep(tWB + tPROG);  /* time WE high to busy + program time */

    /* get status */
    PRINTER("Program status: 0x%02X\n", nand_read_status(gpio));
}


void write_file(char *filename, uint32_t page, size_t length){
    FILE *fp;
    char pagebuffer[PAGESIZE + 1];
    char *pagebuf = pagebuffer;
    struct stat st;
    fp = fopen(filename, "rb");
    if(fp == NULL){
        PRINTER("Error: unable to open file %s\n", filename);
        exit(1);
    }
    if(length == 0){ 
        PRINTER("Error: length = 0, nothing to write.\n"); 
        exit(1);\
    }
    /* get file status */
    int fd = fileno(fp);
    fstat(fd, &st);
    if(st.st_size < length){
        PRINTER("Warning: file: %s length: %lu is less than passed length to write (%lu). New length is set to %lu\n", filename, (size_t)st.st_size, \
                length, (size_t)st.st_size);
        length = st.st_size;
    }

    printf("Writing file: %s of length: %lu page: %u from: 0x%08X to: 0x%08X\n", filename, length, page, \
            page * PAGESIZE, page * PAGESIZE + (uint32_t)length);
    
    /* reading file data */
    size_t bytesleft = length;
    int readsize;
    while(1){
        readsize = MIN(bytesleft, PAGESIZE);
        fread(pagebuf, 1, readsize, fp);
        DEBUG_PRINT("page %d size: %d: data: %s\n", page, readsize, pagebuf);
        write_page_internal(pagebuf, page++, readsize);
        bytesleft -= readsize;
        DEBUG_PRINT("bytesleft: %lu\n", bytesleft);
        if(!bytesleft) break;
    }
    fclose(fp);
    
}

void read_page(void *gpio, uint32_t page){
    /* page_to_address */
    uint8_t addr[4] = {
        0,
        0,
        page & 0xff, /* page (row) first byte */
        page >> 8 & 0xff /* page (row) second byte */
    };
    PRINTER("Reading page: %u from: 0x%08X to: 0x%08X\n", page, page * PAGESIZE, page * PAGESIZE + PAGESIZE); 

    all_out(gpio);
    GPIO_CLR(gpio);
    GPIO_HIGH(gpio, CE); /* standby mode */
    nand_rw_mode(gpio, 0); /* enable rw mode. WP pin low */
    nand_command_send(gpio, 0x0); /* read command first cycle */
    nand_address_send(gpio, (uint8_t *)&addr, 4); /* address to read */
    nand_command_send(gpio, 0x30); /* read command second cycle */

    GPIO_IN_BYTE(gpio, IO0);    
    nsleep(tWHR); /* time WE high to RE low */
    for (int i = 0; i < PAGESIZE; i++){
            printf("%c", nand_read_byte(gpio));
    }
}

int flash_dump(size_t offset, size_t length){
    #ifdef DEBUG
        setbuf(stdout, NULL);
    #endif

    if(length == 0){ 
        printf("Error: length = 0, nothing to read.\n"); 
        exit(1);
    }
    /* check offset bounds */
    if(offset >= TOTALSIZE){
        PRINTER("Error: offset 0x%08X must be lower than total size: 0x%08X\n", (uint32_t)offset, (uint32_t)TOTALSIZE);
        exit(1);
    }
    /* check max read length */
    size_t max_length = TOTALSIZE - offset;
    if(max_length < length){
        PRINTER("Warning: length %lu (0x%08X) is longer than maximum length %lu (0x%08X). New lenth is set to: %lu", \
                length, (uint32_t)length, max_length, (uint32_t)max_length, max_length);
        length = max_length;
    }

    uint16_t page = offset / PAGESIZE; /* row */
    uint16_t inpage_offset = offset % PAGESIZE; /* col */
    int pagesleft = length / PAGESIZE;
    if(length % PAGESIZE != 0) pagesleft += 1;
    PRINTER("Reading: length: %lu pages: %d from page: %d to page: %d offset from: 0x%08X offset to: 0x%08X\n", \
            length, pagesleft, page, page + pagesleft - 1, page * PAGESIZE, page * PAGESIZE + (uint32_t)length); 
    
    /* Page_to_address. First address with possible inpage offset */
     uint8_t addr[4] = {
        inpage_offset & 0xff,
        inpage_offset >> 8 & 0xff,
        page & 0xff, /* page (row) first byte */
        page >> 8 & 0xff /* page (row) second byte */
    };

    /* Prepare to read */
    all_out(gpio);
    GPIO_CLR(gpio);
    GPIO_HIGH(gpio, CE); /* standby mode */
    nand_rw_mode(gpio, 0); /* enable rw mode. WP pin low */

    /*read pages cycle */
    while(pagesleft--){
        PRINTER("\nReading page: %d from: 0x%08X to: 0x%08X\n", page, page * PAGESIZE, page * PAGESIZE + PAGESIZE); 
     
        uint32_t readsize = MIN(length, PAGESIZE);
        read_page_internal((uint8_t *)&addr, readsize);
        length -= readsize;
        
        page++;
        /* next address */
        addr[0] = 0;
        addr[1] = 0;
        addr[2] = page & 0xff; /* page (row) first byte */
        addr[3] = page >> 8 & 0xff; /* page (row) second byte */
    }
    return 0;
}

int read_page_internal(uint8_t *addr, uint32_t length){
    GPIO_OUT_BYTE(gpio, IO0);
    nand_command_send(gpio, 0x0); /* read command first cycle */
    nand_address_send(gpio, addr, 4); /* address to read */
    nand_command_send(gpio, 0x30); /* read command second cycle */

    GPIO_IN_BYTE(gpio, IO0); /* switch data pins to recieve data */   
    nsleep(tWHR); /* time WE high to RE low */
    int i = 0;
    while(i++ < PAGESIZE && length--){
       printf("%c", nand_read_byte(gpio));
    }
    /* write NULL char at the end */
    return 0;
}

void erase_block(void *gpio, uint32_t block){
    /* block to page (row) */
    uint16_t page = block * PAGES_PER_BLOCK;
    uint8_t addr[2] = {
        page & 0xff, /* first byte */
        page >> 8 & 0xff /* second byte */
    };
    uint16_t to_page = page + PAGES_PER_BLOCK; 
    size_t from_offset = page * PAGESIZE;
    size_t to_offset = from_offset + BLOCKSIZE;

    PRINTER("Erasing block: %d of length: %d page from: %d to: %d from: 0x%08X to: 0x%08X\n", \
            block, BLOCKSIZE, page, to_page, (uint32_t)from_offset, (uint32_t)to_offset);

    all_out(gpio);
    GPIO_CLR(gpio);
    GPIO_HIGH(gpio, CE); /* standby mode */
    nand_rw_mode(gpio, 1); /* enable rw mode. WP pin high */
    nand_command_send(gpio, 0x60); /*erase block first command */
    nand_address_send(gpio, (uint8_t *)&addr, 2); /* page (row) address */
    nand_command_send(gpio, 0xd0); /*erase block second command */
    nsleep(tWB + tBERS); /* time WE high to busy + time block erase */

    PRINTER("erase status: 0x%02X\n", nand_read_status(gpio));
}

int erase_flash(size_t from_offset, size_t to_offset){
    /* Offsets must be multiples of block size */
    if(from_offset % BLOCKSIZE != 0 || to_offset % BLOCKSIZE != 0){
        PRINTER("Error: from_offset: 0x%08X and to_offset: 0x%08X is not multiple of block size: 0x%08X\n", (uint32_t)from_offset, \
                (uint32_t)to_offset, (uint32_t)BLOCKSIZE);
        return 1;
    }
    uint32_t block = from_offset / BLOCKSIZE;
    uint32_t blocksleft = (to_offset - from_offset) / BLOCKSIZE;
    while(blocksleft--){
        erase_block(gpio, block++);
    }
    return 0;
}
