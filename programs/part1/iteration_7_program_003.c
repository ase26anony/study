#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr) {
    uint32_t temp;
    
    /* Attempt to generate ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        "mov %[out], %[in]\n\t"
        : [out] "=r" (temp)
        : [in] "r" (*ptr)
        : "memory"
    );
    
    /* Try to extract specific bits using bitfield operations */
    uint32_t mask = 0x00000FF0;  /* Extract bits 4-11 */
    __asm__ volatile (
        "and %0, %1, %2\n\t"
        : "=r" (temp)
        : "r" (*ptr), "r" (mask)
    );
    
    /* Complex pattern that might generate ZERO_EXTRACT */
    uint32_t result;
    __asm__ volatile (
        "bfi %0, %1, #4, #8\n\t"  /* Bit field insert - might generate ZERO_EXTRACT */
        : "=r" (result)
        : "r" (temp)
    );
    
    global_int = result;
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr) {
    uint32_t low_part;
    
    /* Operation that only affects low part of register */
    __asm__ volatile (
        "add %0, %1, #1\n\t"
        : "=r" (low_part)
        : "r" ((uint32_t)*ptr)
        : "cc"
    );
    
    /* Another attempt with explicit low part modification */
    uint16_t short_val;
    __asm__ volatile (
        "inc %0\n\t"
        : "=r" (short_val)
        : "0" ((uint16_t)*ptr)
        : "cc"
    );
    
    /* Complex pattern with early clobber */
    uint32_t a, b;
    a = (uint32_t)*ptr;
    b = a + 1;
    __asm__ volatile (
        "add %0, %1, %2\n\t"
        : "=&r" (low_part)  /* Early clobber */
        : "r" (a), "r" (b)
        : "cc"
    );
    
    global_int = low_part;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *ptr) {
    /* Access different sized subregions of memory */
    
    /* Write to short within int */
    __asm__ volatile (
        "movw %0, #0xABCD\n\t"
        : "=m" (*(volatile uint16_t *)ptr)
        :
        : "memory"
    );
    
    /* Write to byte within long */
    __asm__ volatile (
        "movb %0, #0x42\n\t"
        : "=m" (*(volatile uint8_t *)ptr)
        :
        : "memory"
    );
    
    /* Complex memory access with offset */
    uint32_t offset = 2;
    __asm__ volatile (
        "strh %1, [%0, %2]\n\t"
        :
        : "r" (ptr), "r" (0x1234), "r" (offset)
        : "memory"
    );
}

/* Additional test with array access */
static void test_array_subreg(volatile uint16_t *arr) {
    /* Access array elements with type punning */
    volatile uint32_t *as_int = (volatile uint32_t *)arr;
    
    /* Write to first two array elements as a single int */
    __asm__ volatile (
        "mov %0, #0xDEADBEEF\n\t"
        : "=m" (*as_int)
        :
        : "memory"
    );
    
    /* Access misaligned subreg */
    volatile uint8_t *as_byte = (volatile uint8_t *)arr;
    __asm__ volatile (
        "mov %0, #0x55\n\t"
        : "=m" (as_byte[1])  /* Access byte within short */
        :
        : "memory"
    );
}

/* Test with bitfield structures */
struct bitfield_struct {
    uint32_t a : 8;
    uint32_t b : 16;
    uint32_t c : 8;
};

static void test_bitfield_struct(volatile struct bitfield_struct *bf) {
    /* Operations on bitfields might generate ZERO_EXTRACT */
    uint32_t temp;
    
    __asm__ volatile (
        "ldrb %0, [%1]\n\t"
        : "=r" (temp)
        : "r" (&bf->a)
        : "memory"
    );
    
    /* Modify bitfield through pointer */
    volatile uint32_t *whole = (volatile uint32_t *)bf;
    __asm__ volatile (
        "orr %0, %0, #0x0000FF00\n\t"
        : "+m" (*whole)
        :
        : "memory"
    );
}

int main() {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_long);
    checksum += (uint32_t)global_long;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    checksum += global_int;
    
    /* Test array subreg access */
    test_array_subreg(global_short_array);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Test bitfield structure */
    volatile struct bitfield_struct bf = {0};
    test_bitfield_struct(&bf);
    checksum += *(volatile uint32_t *)&bf;
    
    /* Additional complex pattern with mixed types */
    volatile uint32_t mixed_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    volatile uint16_t *as_short = (volatile uint16_t *)mixed_array;
    
    /* Force SUBREG MEM access */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=m" (as_short[1])
        : "r" (0x5555)
        : "memory"
    );
    
    for (int i = 0; i < 4; i++) {
        checksum += mixed_array[i];
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
