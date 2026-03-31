#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *var) {
    /* Force ZERO_EXTRACT by writing to specific bit-field */
    uint32_t temp;
    __asm__ volatile (
        /* Write to bits 8-15 of the variable */
        "mov %[temp], %[val]\n\t"
        "bfi %[out], %[temp], #8, #8"
        : [out] "+r" (*var), [temp] "=&r" (temp)
        : [val] "r" (0xAAU)
        : "cc"
    );
}

/* Another ZERO_EXTRACT variant with memory destination */
static void test_zero_extract_mem(volatile uint64_t *var) {
    uint32_t mask = 0x0000FFFF;
    uint32_t value = 0xDEAD;
    
    __asm__ volatile (
        /* Extract and insert bits 16-31 */
        "bfxil %[out], %[val], #0, #16"
        : [out] "+m" (*var)
        : [val] "r" (value), "m" (mask)
        : "cc"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(void) {
    volatile uint32_t a = 0x12345678;
    volatile uint32_t b = 0x87654321;
    
    __asm__ volatile (
        /* Operation that only affects low part */
        "add %0, %1, %2"
        : "=r" (a)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Use early-clobber to force complex register allocation */
    uint32_t c, d;
    __asm__ volatile (
        "mov %[c], #0x55\n\t"
        "add %[d], %[c], #1"
        : [c] "=&r" (c), [d] "=&r" (d)
        :
        : "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_var) {
    /* Access short within int through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_var;
    
    __asm__ volatile (
        /* Write to sub-register of memory */
        "strh %[val], [%[ptr]]"
        : 
        : [ptr] "r" (short_ptr), [val] "r" ((uint16_t)0xBEEF)
        : "memory"
    );
}

/* Another SUBREG variant with array access */
static void test_subreg_mem_array(volatile uint8_t *array) {
    /* Access 32-bit value at byte offset 2 */
    volatile uint32_t *int_ptr = (volatile uint32_t *)(array + 2);
    
    __asm__ volatile (
        "str %[val], [%[ptr]]"
        :
        : [ptr] "r" (int_ptr), [val] "r" (0xCAFEBABEU)
        : "memory"
    );
}

/* Complex pattern combining multiple features */
static void test_combined_pattern(void) {
    volatile struct {
        uint32_t a;
        uint16_t b;
        uint8_t c;
    } s = {0};
    
    /* Create SUBREG access to part of structure */
    volatile uint16_t *b_ptr = &s.b;
    
    __asm__ volatile (
        /* Multiple operations to create complex RTL */
        "mov r0, #0x1234\n\t"
        "strh r0, [%0]\n\t"
        "ldrb r1, [%1]\n\t"
        "orr r1, r1, #0xF0\n\t"
        "strb r1, [%1]"
        :
        : "r" (b_ptr), "r" (&s.c)
        : "r0", "r1", "memory", "cc"
    );
}

/* Main driver function */
int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int);
    checksum += global_int;
    
    test_zero_extract_mem(&global_long);
    checksum += (global_long & 0xFFFFFFFF) + (global_long >> 32);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part();
    checksum += 1;  /* Dummy checksum update */
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    checksum += global_int;
    
    test_subreg_mem_array(global_byte_array);
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    /* Test combined pattern */
    test_combined_pattern();
    
    /* Test with local variables */
    volatile uint32_t local_var = 0xDEADBEEF;
    volatile uint16_t local_short_array[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    
    test_zero_extract(&local_var);
    checksum += local_var;
    
    test_subreg_mem(&local_var);
    checksum += local_var;
    
    /* Access array elements with SUBREG patterns */
    for (int i = 0; i < 4; i++) {
        test_subreg_mem((volatile uint32_t *)&local_short_array[i]);
        checksum += local_short_array[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    return 0;
}
