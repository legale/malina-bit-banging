/* iARPIGIO based Raspberry Pi  nand flash bit-bang programmer
 * <bit-bang.c>
 *
 * Copyright (c) 2021, legale <legale.legale@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h> /* integer types uint* */
#include <time.h> /* clock() nanosleep() */
#include <string.h>


#include "direct5.h"
#include "yarpio/yarpio.h"
#include "samsung_nand.h"

const char *commands[] = {
	"stat", "all_in", "all_out", "speed_test", "blink [pin] [delay]", 
    "read_id", 
    "read_data [offset] [length]", "pud_off [pin]",
	"write_page [filename] [page] [length]",
	"read_page [page]",
    "write_file [filename] [page] [length]", 
    "erase_block [block]",
    "pud_down [pin]", "pud_up [pin]", "in [pin]", "out [pin]", 
    "high [pin]", "low [pin]", "switch [pin]", "--help", "-h"
};
const int commands_size = sizeof commands / sizeof commands[0];

void help(){
	printf("Available commands: %d\n", commands_size);
    for(int i = 0; i < commands_size; i++){
        printf("%s\n", commands[i]);
    }
}
 
int main(int argc, char **argv){

  void *gpio; /* pointer to gpio */
  /* prepare gpio pointer for direct register access */
  setup_io(&gpio); /* pass the pointer to gpio pointer */
  if (argc == 1) help(); 
  for (int i = 1; i < argc; i++){
	DEBUG_PRINT("arg %d: %s\n", i, argv[i]);
	if      (strcmp("stat", argv[i])  == 0)  { stat(gpio); }
	else if (strcmp("all_in", argv[i]) == 0) { all_in(gpio); }
	else if (strcmp("all_out", argv[i]) == 0) { all_out(gpio); }
	else if (strcmp("speed_test", argv[i]) == 0){ speed_test(gpio); }
	else if (strcmp("blink", argv[i]) == 0 && (i + 2) < argc){ blink(gpio, atoi(argv[++i]), strtoul(argv[++i], NULL,10)); } 
	else if (strcmp("read_id", argv[i]) == 0){ read_id(gpio); }
	else if (strcmp("write_page", argv[i]) == 0 && (i + 2) < argc){ write_page(gpio, argv[++i], atoi(argv[++i]), atoi(argv[++i])); }
	else if (strcmp("write_file", argv[i]) == 0 && (i + 2) < argc){ write_file(gpio, argv[++i], atoi(argv[++i]), atoi(argv[++i])); }
	else if (strcmp("read_data", argv[i]) == 0 && (i + 2) < argc){ read_data(gpio, atoi(argv[++i]), atoi(argv[++i])); }
    else if (strcmp("read_page", argv[i]) == 0 && (i + 1) < argc){ read_page(gpio, atoi(argv[++i])); }
    else if (strcmp("erase_block", argv[i]) == 0 && (i + 1) < argc){ erase_block(gpio, atoi(argv[++i])); }
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
	else 	{printf("command not found\n");}
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
        printf("%i 0x%02X\n", i, byte);
    }
    GPIO_LOW(gpio, CE); /* Chip Enabled low */
}

/* function to write nand page from file */
void write_page(void *gpio, char *filename, uint32_t page, uint32_t length){
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
    byte = fgetc(fp);
    int i = 0;
    while(feof(fp) == 0 && i < length){
        nand_write_byte(gpio, byte);
        DEBUG_PRINT("%d '%c' \n", i++, byte);
        byte = fgetc(fp);
    }
    fclose(fp);
    
    nand_command_send(gpio, 0x10); /*program command. 0x10 */
    nsleep(tWB + tPROG);  /* time WE high to busy + program time */

    /* get status */
    printf("Program status: 0x%02X\n", nand_read_status(gpio));
}


void write_file(void *gpio, char *filename, uint32_t page, uint32_t length){
    
}

void read_page(void *gpio, uint32_t page){
    /* page_to_address */
    uint8_t addr[4] = {
        0,
        0,
        page & 0xff, /* page (row) first byte */
        page >> 8 & 0xff /* page (row) second byte */
    };
    PRINTER("Reading page: %d from: 0x%08X to: 0x%08X\n", page, page * PAGESIZE, page * PAGESIZE + PAGESIZE); 

    all_out(gpio);
    GPIO_CLR(gpio);
    GPIO_HIGH(gpio, CE); /* standby mode */
    nand_rw_mode(gpio, 0); /* enable rw mode. WP pin low */
    nand_command_send(gpio, 0x0); /* read command first cycle */
    nand_address_send(gpio, (uint8_t *)&addr, 4); /* address to read */
    nand_command_send(gpio, 0x30); /* read command second cycle */
    GPIO_SET(gpio, CLE, 0);    

    GPIO_IN_BYTE(gpio, IO0);    
    nsleep(tWHR); /* time WE high to RE low */
    for (int i = 0; i < PAGESIZE; i++){
        printf("%c", nand_read_byte(gpio));
    }
}

void erase_block(void *gpio, uint32_t block){
    /* block to page (row) */
    uint16_t page = block * PAGES_PER_BLOCK;
    uint8_t addr[2] = {
        page & 0xff, /* first byte */
        page >> 8 & 0xff /* second byte */
    };
    printf("Erasing block: %d of length: %d page from: %d to: %d from: 0x%08X to: 0x%08X\n", block,
            block * BLOCKSIZE, page, page + PAGES_PER_BLOCK, page * PAGESIZE, (page + PAGES_PER_BLOCK) * PAGESIZE);

    all_out(gpio);
    GPIO_CLR(gpio);
    GPIO_HIGH(gpio, CE); /* standby mode */
    nand_rw_mode(gpio, 1); /* enable rw mode. WP pin high */
    nand_command_send(gpio, 0x60); /*erase block first command */
    nand_address_send(gpio, (uint8_t *)&addr, 2); /* page (row) address */
    nand_command_send(gpio, 0xd0); /*erase block second command */
    nsleep(tWB + tBERS); /* time WE high to busy + time block erase */

    printf("erase status: 0x%02X\n", nand_read_status(gpio));
}
void read_data(void *gpio, uint32_t offset, uint32_t length){
}

