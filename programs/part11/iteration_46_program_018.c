/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource marking:
 * 1. ZERO_EXTRACT/STRICT_LOW_PART in SET_DEST
 * 2. SUBREG in SET_DEST  
 * 3. MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Source variables with bitwise operations */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* Complex bitfield store */
    bf.field12 = (a + b + c) & 0xFFF;          /* Another ZERO_EXTRACT candidate */
    
    /* Use builtins that might extract bit ranges */
    unsigned char small = (a & 0xFF);
    bf.field8 = __builtin_popcount(small);     /* Bit manipulation builtin */
    
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations for sub-word type assignments */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    volatile unsigned char vuc;
    
    /* Register sources to encourage register operations */
    register int reg_int = 0x12345678;
    register unsigned long reg_long = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - potential SUBREG in SET_DEST */
    vs = (short)reg_int;                       /* int -> short SUBREG */
    vc = (signed char)(reg_int + 0x100);       /* With arithmetic */
    vuc = (unsigned char)(reg_long >> 16);     /* long -> char */
    
    /* Implicit narrowing with arithmetic */
    char c1 = 100, c2 = 50;
    volatile char vc2;
    vc2 = c1 + c2;                             /* char + char -> char (truncation) */
    
    /* Mixed operations that might create SUBREG */
    short s1 = 1000;
    int i1 = 50000;
    vs = s1 + (short)i1;                       /* Mixed with explicit cast */
    
    sink = vs + vc + vuc + vc2;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    int * restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index: i*3 + 7 encourages address computation */
        int idx = i * 3 + 7;
        
        /* Store with complex addressing - should create MEM with address expr */
        arr[idx % 64][(i * 5) % 8] = i * 100 + 42;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx + (i & 3)) = i * 200 + 17;
    }
    
    /* Struct with array member */
    struct {
        int data[32];
        int extra;
    } s = {0};
    
    /* Access through pointer with offset */
    int *data_ptr = s.data;
    for (int i = 0; i < 8; i++) {
        /* Complex addressing: base + scaled index + constant */
        data_ptr[i * 4 + 2] = i * 300 + 99;
    }
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sink = sum + s.data[10];
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        short values[16];
        unsigned int status : 4;
    } comb = {0};
    
    /* Register source for potential SUBREG */
    register int reg_val = 0x87654321;
    
    /* Combined assignment: bitfield + array element with complex index */
    comb.flags = (reg_val & 0xFF) | 0x1;       /* ZERO_EXTRACT */
    
    /* Array store with narrowed value and complex index */
    int idx = (reg_val >> 8) & 0xF;
    comb.values[idx * 2 + 1] = (short)(reg_val >> 16);  /* SUBREG + complex MEM */
    
    /* Another bitfield with computation */
    comb.status = ((reg_val & 0xF) + (reg_val >> 28)) & 0xF;
    
    /* Additional complex memory store */
    int temp_array[32] = {0};
    int complex_idx = ((reg_val & 0xFF) * 3 + 11) % 32;
    temp_array[complex_idx] = (short)reg_val;  /* SUBREG in store */
    
    sink = comb.flags + comb.values[3] + comb.status + temp_array[10];
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[32] = {0};
    int index = 7;
    
    /* Inline asm with memory output and complex addressing */
    asm volatile (
        "# Force complex memory store\n"
        : "=m" (array[index * 2 + 3])  /* Complex addressing in constraint */
        : 
        : "memory"
    );
    
    /* Another asm with potential bitfield reference */
    volatile unsigned int bitfield = 0;
    asm volatile (
        "# Reference bitfield-like location\n"
        : "=m" (bitfield)
        : 
        : "memory"
    );
    
    sink = array[17] + bitfield;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total_checksum += sink;
    
    test_subreg_operations();
    total_checksum += sink;
    
    test_complex_addressing();
    total_checksum += sink;
    
    test_combined_patterns();
    total_checksum += sink;
    
    test_inline_asm();
    total_checksum += sink;
    
    /* Final output to prevent dead code elimination */
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
