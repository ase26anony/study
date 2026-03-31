/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Variables for bit manipulation */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF);
    bf.field12 = (a + b + c) & 0xFFF;
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (a >> 8) & 0xFF;
    bf.field4 = __builtin_popcount(byte_val) & 0xF;
    
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    
    /* Register variables to encourage register operations */
    register int reg_int = 0x12345678;
    register unsigned long reg_long = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - should generate SUBREG */
    vs = (short)reg_int;
    vc = (signed char)(reg_int >> 16);
    
    /* Arithmetic with implicit narrowing */
    char c1 = 100;
    char c2 = 50;
    volatile char vc2 = c1 + c2;  /* May generate SUBREG for truncation */
    
    /* Mixed-size operations */
    short s1 = 1000;
    short s2 = 2000;
    volatile char vc3 = (char)(s1 + s2);
    
    sink = vs + vc + vc2 + vc3;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array */
    int arr[64][8];
    int * restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-linear addressing */
            int idx = i * 13 + j * 7 + 3;  /* Prime numbers for non-simple stride */
            if (idx < 512) {  /* Ensure bounds */
                /* Store with complex address computation */
                arr[i][j] = idx * 2;
                
                /* Pointer arithmetic with multiple offsets */
                *(ptr + idx + (i * 2)) = idx * 3;
            }
        }
    }
    
    /* Struct with array member */
    struct {
        int data;
        int array[16];
        int more_data;
    } s, *sptr = &s;
    
    /* Access through pointer with offset */
    for (int i = 0; i < 8; i++) {
        sptr->array[i * 2 + 1] = i * 100;
    }
    
    sink = arr[0][0] + sptr->array[0];
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned int status : 4;
        short data[32];
        int counters[8];
    } comb;
    
    /* Initialize */
    register int temp = 0x89ABCDEF;
    
    /* Combined assignment: bitfield + narrowed store */
    comb.flags = (temp >> 16) & 0xFF;  /* ZERO_EXTRACT potential */
    
    /* Complex array access with narrowing */
    for (int i = 0; i < 16; i++) {
        int idx = (i * 7 + 3) & 0x1F;  /* Complex index */
        comb.data[idx] = (short)(temp + i * 100);  /* SUBREG potential */
        
        /* Update temp to vary the pattern */
        temp = (temp >> 1) | (temp << 31);
    }
    
    /* Nested addressing */
    int *ptr = comb.counters;
    for (int i = 0; i < 4; i++) {
        *(ptr + i * 3 + 1) = i * 1000;  /* Complex memory address */
    }
    
    sink = comb.flags + comb.data[0] + comb.counters[0];
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[32] = {0};
    int index = 7;
    int value = 0x1234;
    
    /* Memory output with complex addressing */
    asm volatile (
        "# Force complex memory store\n"
        : "=m" (array[index * 2 + 3])  /* Complex address */
        : 
        : "memory"
    );
    
    /* Bitfield-like constraint (less portable) */
    struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } packed;
    
    unsigned int source = 0xABCD1234;
    
    /* Attempt to generate subreg/zero_extract */
    asm volatile (
        "# Store low 16 bits\n"
        : "=r" (packed.low)  /* May not work as expected for bitfields */
        : "r" (source)
    );
    
    sink = array[0] + packed.low;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += sink;
    
    test_subreg_operations();
    checksum += sink;
    
    test_complex_addressing();
    checksum += sink;
    
    test_combined_patterns();
    checksum += sink;
    
    test_inline_asm();
    checksum += sink;
    
    /* Print checksum to ensure all code executes */
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
