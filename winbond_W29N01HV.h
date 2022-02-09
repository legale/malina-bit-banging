/* Samsung NAND flash memory chip WINBOND k29N01HV ID EF F1 00 95 00 macros */
#define RB 16    /* pin 7 */
#define RE 4    /* pin 8 */
#define CE 2    /* pin 9 */
#define VCC 21    /* pin 12,37 */
#define CLE 19    /* pin 16 */
#define ALE 1   /* pin 17 */
#define WE 3    /* pin 18 */
#define WP 20    /* pin 19 */

#define IO0 8    /* pin 29 */
#define IO1 9    /* pin 30 */
#define IO2 10   /* pin 31 */
#define IO3 11   /* pin 32 */
#define IO4 12   /* pin 41 */
#define IO5 13   /* pin 42 */
#define IO6 14   /* pin 43 */
#define IO7 15   /* pin 44 */

/* VSS GPIO pin 39 pin 13,36 */

/* timings */
/* command, address, data input */
#define tCLS    10
#define tCLH    5
#define tCS     15
#define tCH     5
#define tWP     12
#define tALS    10
#define tALH    5
#define tDS     10
#define tDH     5
#define tWC     25
#define tWH     10
#define tADL    70

#define tAR     10
#define tCLR    10
#define tRR     20
#define tRP     12
#define tRC     25
#define tCSD    0
#define tRHOH   15
#define tCOH    15
#define tREH    10
#define tIR     0
#define tRHW    100
#define tWHR    60
#define tWB     100         /* WE high to busy */
#define tPROG   (900 * 1000)  /* (900 us to ns) page program time */
#define tBERS   (4.5 * 1e6)   /* (4.5 ms to ns) block erase time */


/* chip specs */
#define PAGESIZE       2112
#define TOTALPAGES     65536
#define PAGES_PER_BLOCK 64
#define TOTALSIZE     (PAGESIZE * TOTALPAGES)
#define BLOCKSIZE      (PAGESIZE * PAGES_PER_BLOCK)
#define READSIZE       PAGESIZE
#define WRITESIZE      PAGESIZE
#define ERASESIZE      BLOCKSIZE

