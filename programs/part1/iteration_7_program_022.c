#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src)
{
    /* Target: SET_DEST is ZERO_EXTRACT wrapping MEM
       Using bit-field constraints to extract specific bits */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=m" (*dest)  /* Memory destination */
        : [src] "r" (src)      /* Register source */
        : "memory"
    );
}

/* Function to generate ZERO_EXTRACT with explicit bitfield */
static void test_zero_extract_bitfield(volatile uint32_t *dest)
{
    uint32_t temp;
    /* Force ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        "btsl $8, %[dest]\n\t"      /* Set bit 8 */
        "btrl $16, %[dest]\n\t"     /* Clear bit 16 */
        : [dest] "+m" (*dest)
        :
        : "cc", "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src)
{
    /* Target: SET_DEST is STRICT_LOW_PART
       Using constraint that implies only low part is modified */
    __asm__ volatile (
        "addw %[src], %[dest]\n\t"  /* 16-bit add for low part */
        : [dest] "+r" (*dest)       /* Register operand, low part only */
        : [src] "r" (src)
        : "cc"
    );
}

/* Alternative STRICT_LOW_PART with byte operation */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src)
{
    /* Force STRICT_LOW_PART by operating on byte sub-register */
    __asm__ volatile (
        "orb %[src], %[dest]\n\t"
        : [dest] "+q" (*dest)       /* "q" constraint for byte-addressable register */
        : [src] "r" (src)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr)
{
    /* Target: SET_DEST is SUBREG wrapping MEM
       Access sub-register of memory through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw $0xABCD, %[ptr]\n\t"
        : [ptr] "=m" (*short_ptr)   /* Write to 16-bit sub-register of 32-bit memory */
        :
        : "memory"
    );
}

/* More complex SUBREG of MEM with array access */
static void test_subreg_mem_array(volatile uint64_t *array, int index)
{
    /* Access 32-bit sub-register of 64-bit memory location */
    volatile uint32_t *sub_ptr = (volatile uint32_t *)((char *)array + index * sizeof(uint64_t) + 2);
    
    __asm__ volatile (
        "movl $0xDEADBEEF, %[ptr]\n\t"
        : [ptr] "=m" (*sub_ptr)
        :
        : "memory"
    );
}

/* Function combining multiple patterns */
static void test_combined_patterns(void)
{
    volatile uint32_t combined = 0;
    volatile uint16_t *short_view = (volatile uint16_t *)&combined;
    
    /* Generate SUBREG of MEM */
    __asm__ volatile (
        "movw $0x1234, %0\n\t"
        : "=m" (short_view[0])      /* Write to low 16 bits */
        :
        : "memory"
    );
    
    /* Generate ZERO_EXTRACT for high bits */
    __asm__ volatile (
        "btsl $24, %0\n\t"          /* Set bit 24 */
        : "+m" (combined)
        :
        : "cc", "memory"
    );
}

/* Main driver function */
int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1111);
    test_strict_low_part_byte(&global_byte_array[0], 0xAA);
    checksum += global_short_array[0];
    checksum += global_byte_array[0];
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem_array(&global_long, 0);
    checksum += global_int;
    checksum += (uint32_t)(global_long & 0xFFFFFFFF);
    
    /* Test combined patterns */
    test_combined_patterns();
    
    /* Force use of all variables to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("All pattern functions executed\n");
    
    return 0;
}
