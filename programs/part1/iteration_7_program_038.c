#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Bit-field assignment targeting specific bits */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
}

/* Function to generate ZERO_EXTRACT with memory destination */
static void test_zero_extract_mem(volatile uint32_t *mem, uint32_t value) {
    /* Complex constraint to force ZERO_EXTRACT on memory */
    uint32_t temp;
    __asm__ volatile (
        "and %[temp], %[value], #0xFF\n\t"
        "strb %[temp], [%[mem]]\n\t"
        : [temp] "=&r" (temp), [mem] "+r" (mem)
        : [value] "r" (value)
        : "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Operation that only modifies low part of register */
    __asm__ volatile (
        "add %[dest], %[src], #1\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART with early clobber */
static void test_strict_low_part_earlyclobber(volatile uint32_t *a, volatile uint32_t *b) {
    uint32_t tmp1, tmp2;
    __asm__ volatile (
        "ldr %[tmp1], [%[a]]\n\t"
        "ldr %[tmp2], [%[b]]\n\t"
        "add %[tmp1], %[tmp2], %[tmp1]\n\t"
        "str %[tmp1], [%[a]]\n\t"
        : [tmp1] "=&r" (tmp1), [tmp2] "=&r" (tmp2)
        : [a] "r" (a), [b] "r" (b)
        : "memory", "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access sub-register of memory through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %[val], #0xABCD\n\t"
        "strh %[val], [%[ptr]]\n\t"
        : [val] "=&r" (global_int)  /* Early clobber to force SUBREG */
        : [ptr] "r" (short_ptr)
        : "memory"
    );
}

/* Function to generate complex SUBREG+MEM pattern */
static void test_subreg_complex(volatile uint64_t *long_ptr) {
    /* Access different sized subregisters of memory */
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    volatile uint16_t *short_ptr = (volatile uint16_t *)&int_ptr[1];
    volatile uint8_t *byte_ptr = (volatile uint8_t *)&short_ptr[1];
    
    __asm__ volatile (
        "mov %[tmp1], #0x11\n\t"
        "mov %[tmp2], #0x22\n\t"
        "mov %[tmp3], #0x33\n\t"
        "str %[tmp1], [%[ptr1]]\n\t"
        "strh %[tmp2], [%[ptr2]]\n\t"
        "strb %[tmp3], [%[ptr3]]\n\t"
        : [tmp1] "=&r" (global_int), 
          [tmp2] "=&r" (global_int),
          [tmp3] "=&r" (global_int)
        : [ptr1] "r" (int_ptr),
          [ptr2] "r" (short_ptr),
          [ptr3] "r" (byte_ptr)
        : "memory"
    );
}

/* Function to generate mixed patterns */
static void test_mixed_patterns(void) {
    volatile uint32_t mixed_array[4] = {0};
    volatile uint16_t *half_ptr = (volatile uint16_t *)mixed_array;
    
    /* Mix of operations that could generate various RTL patterns */
    for (int i = 0; i < 4; i++) {
        __asm__ volatile (
            "ldrh %[val], [%[ptr], %[idx], LSL #1]\n\t"
            "add %[val], %[val], #1\n\t"
            "strh %[val], [%[ptr], %[idx], LSL #1]\n\t"
            : [val] "=&r" (global_int)
            : [ptr] "r" (half_ptr), [idx] "r" (i)
            : "memory", "cc"
        );
    }
}

/* Main driver function */
int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_mem(&global_int, 0xA5A5A5A5);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part_earlyclobber(&global_int, &global_long);
    checksum += global_short_array[0] + (uint32_t)global_int;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_complex(&global_long);
    checksum += global_int + (uint32_t)(global_long & 0xFFFFFFFF);
    
    /* Test mixed patterns */
    test_mixed_patterns();
    
    /* Calculate final checksum to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("Pattern generation complete\n");
    
    return 0;
}
