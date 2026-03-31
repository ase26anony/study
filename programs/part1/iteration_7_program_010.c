#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Target: SET_DEST is ZERO_EXTRACT wrapping MEM
       Constraint for writing to specific bits of memory */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=m" (*dest)  /* Memory destination */
        : [src] "r" (src)      /* Register source */
        : "memory"
    );
}

/* Function to generate ZERO_EXTRACT on bit-field */
static void test_zero_extract_bitfield(volatile uint32_t *dest, uint32_t src) {
    /* Using bit-field constraints to force ZERO_EXTRACT */
    uint32_t temp;
    __asm__ volatile (
        "and %[temp], %[src], #0xFF\n\t"      /* Extract low byte */
        "bfi %[dest], %[temp], #8, #8\n\t"    /* Insert into bits 8-15 */
        : [dest] "+&r" (*dest), [temp] "=&r" (temp)
        : [src] "r" (src)
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Target: SET_DEST is STRICT_LOW_PART
       Early-clobber and specific constraints to hint at partial register */
    __asm__ volatile (
        "add %[dest], %[src], #1\n\t"
        : [dest] "=&r" (*dest)    /* Early-clobber, partial register */
        : [src] "r" (src)
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART with arithmetic */
static void test_strict_low_part_arith(volatile uint32_t *a, volatile uint32_t *b) {
    uint32_t temp_a, temp_b;
    __asm__ volatile (
        "ldr %[temp_a], [%[a]]\n\t"
        "ldr %[temp_b], [%[b]]\n\t"
        "adds %[temp_a], %[temp_a], %[temp_b]\n\t"  /* Only affects low part via flags */
        "str %[temp_a], [%[a]]\n\t"
        : [temp_a] "=&r" (temp_a), [temp_b] "=&r" (temp_b)
        : [a] "r" (a), [b] "r" (b)
        : "cc", "memory"
    );
}

/* Function to generate SUBREG wrapping MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, uint16_t value) {
    /* Target: SET_DEST is SUBREG of MEM
       Access short within int through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "strh %[val], [%[ptr]]\n\t"
        : 
        : [ptr] "r" (short_ptr), [val] "r" (value)
        : "memory"
    );
}

/* Function to generate complex SUBREG+MEM with offset */
static void test_subreg_mem_complex(volatile uint64_t *base, uint32_t offset, uint16_t value) {
    /* Access different sized subregisters within memory */
    volatile uint16_t *ptr16 = (volatile uint16_t *)((uintptr_t)base + offset);
    volatile uint8_t *ptr8 = (volatile uint8_t *)((uintptr_t)base + offset + 2);
    
    __asm__ volatile (
        "strh %[val16], [%[ptr16]]\n\t"
        "strb %[val8], [%[ptr8]]\n\t"
        : 
        : [ptr16] "r" (ptr16), [val16] "r" (value),
          [ptr8] "r" (ptr8), [val8] "r" ((uint8_t)(value >> 8))
        : "memory"
    );
}

/* Function to test all patterns with various inputs */
static void test_all_patterns(void) {
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint16_t local_short = 0xCAFE;
    volatile uint64_t local_long = 0x123456789ABCDEF0ULL;
    uint32_t temp;
    
    printf("Testing ZERO_EXTRACT patterns...\n");
    
    /* Test ZERO_EXTRACT with memory destination */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_bitfield(&global_int, 0xAA55AA55);
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part_arith(&local_int, &global_int);
    
    /* Test SUBREG of MEM patterns */
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&local_int, 0xBABE);
    test_subreg_mem_complex(&local_long, 2, 0xF00D);
    
    /* Test with array elements */
    for (int i = 0; i < 4; i++) {
        test_subreg_mem((volatile uint32_t *)&global_short_array[i], 0x1000 + i);
    }
    
    /* Mix patterns with different data types */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&global_long;
    for (int i = 0; i < 8; i++) {
        __asm__ volatile (
            "mov %[temp], #0x%c[idx]\n\t"
            "strb %[temp], [%[ptr], %[idx]]\n\t"
            : [temp] "=&r" (temp)
            : [ptr] "r" (byte_ptr), [idx] "r" (i)
            : "memory"
        );
    }
}

/* Main driver function */
int main(void) {
    uint32_t checksum = 0;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Run all pattern tests */
    test_all_patterns();
    
    /* Calculate checksum to prevent dead code elimination */
    checksum += global_int;
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    checksum += (uint32_t)global_long;
    checksum += (uint32_t)(global_long >> 32);
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed. Compile with:\n");
    printf("  gcc -O2 -dP -da -o test test.c\n");
    printf("  gcc -O1 -fno-omit-frame-pointer -fdump-rtl-all -o test test.c\n");
    printf("  gcc -O3 -funroll-loops -fno-inline -o test test.c\n");
    
    return (int)checksum;
}
