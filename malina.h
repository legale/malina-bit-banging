/* Raspberry Pi (malina) NAND flash bit-banging programmer
*  <malina.h>
*/
#include <stdint.h>
#include <unistd.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define MIN(a, b) (a < b ? a : b)
#define PRINTER(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )

void nand_command_send(void *gpio, uint8_t byte);
void nand_address_send(void *gpio, uint8_t *addr, int size);
void nand_rw_mode(void *gpio, int wp_pin);
void nand_write_byte(void *gpio, uint8_t byte); 
uint8_t nand_read_byte(void *gpio);
uint8_t nand_read_status(void *gpio);
void read_id(void *gpio);
void write_page_internal(char *filename, uint32_t page, uint32_t length);
void write_page(char *filename, uint32_t page, uint32_t length);
void write_file(char *pagebuf, uint32_t page, size_t length);
void read_page(void *gpio, uint32_t page);
int read_page_internal(uint8_t *addr, uint32_t length);
int flash_dump(size_t offset, size_t length);
void erase_block(void *gpio, uint32_t block);
int erase_flash(size_t from_offset, size_t to_offset);
