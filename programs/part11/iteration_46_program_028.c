/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations alive */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Source values in registers */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex assignments to bitfields - may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0x1);      /* 4-bit extract and store */
    bf.field8 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit with computation */
    bf.field12 = (a + b) & 0xFFF;           /* 12-bit masked store */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (a >> 8) & 0xFF;
    bf.field4 = __builtin_popcount(byte_val) & 0xF;
    
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG patterns through type narrowing */
void test_subreg_patterns(void) {
    /* Volatile sub-word destinations */
    volatile short vs;
    volatile signed char vc;
    
    /* Register sources of different sizes */
    register int ri = 0x12345678;
    register long rl = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs = (short)ri;                     /* int -> short */
    vc = (signed char)(ri >> 16);       /* int -> char */
    
    /* Arithmetic with implicit narrowing */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    vc = uc1 + uc2;                     /* char + char -> char (overflow truncation) */
    
    /* Complex expression with narrowing */
    vs = (short)((ri & 0xFFFF) + (rl & 0xFFFF));
    
    sink = vs + vc;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    int *restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        register int val = i * 0x1111;  /* Register-held source */
        
        /* Multiple addressing patterns */
        arr[i][i % 8] = val;                     /* 2D array access */
        *(ptr + i * 8 + (i % 3)) = val >> 1;     /* Pointer arithmetic */
        
        /* Struct-like access through pointer */
        int *elem = &arr[i][0];
        elem[(i * 3 + 7) % 8] = val + i;         /* Complex offset */
    }
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr[i][i % 8];
    }
    sink = sum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct with mixed members */
    struct combined {
        volatile unsigned int flags : 10;
        volatile short data[16];
        int buffer[32];
    } cmb = {0};
    
    /* Pointer to struct */
    struct combined *cmb_ptr = &cmb;
    
    register int r1 = 0x13579BDF;
    register int r2 = 0x2468ACE0;
    
    /* Combined assignment: bitfield + array with complex addressing */
    cmb.flags = (r1 & 0x3FF) | ((r2 >> 5) & 0x3FF);  /* ZERO_EXTRACT candidate */
    
    /* Array store with narrowing and complex index */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 7 + 3) % 16;                    /* Non-linear index */
        cmb.data[idx] = (short)(r1 + i * r2);          /* SUBREG candidate */
        
        /* Additional complex memory store */
        cmb.buffer[(i * 5 + 11) % 32] = (r1 >> (i * 2)) & 0xFF;
    }
    
    /* Inline assembly with memory output constraint */
    int temp = 0xDEADBEEF;
    asm volatile (
        "# Force complex memory store\n"
        : "=m" (cmb.buffer[temp % 32])  /* Complex addressing in constraint */
        : "r" (temp)
        : "memory"
    );
    
    sink = cmb.flags + cmb.data[0] + cmb.buffer[0];
}

/* Test 5: Additional patterns using builtins */
void test_builtin_patterns(void) {
    volatile struct {
        unsigned int parity_field : 3;
        unsigned int count_field : 5;
    } bits = {0};
    
    register unsigned int x = 0x89ABCDEF;
    
    /* Builtins that operate on bit ranges */
    bits.parity_field = __builtin_parity(x) & 0x7;
    bits.count_field = __builtin_popcount(x & 0x1F) & 0x1F;
    
    /* Extract and store specific bit ranges */
    unsigned int extracted = (x >> 8) & 0xFF;
    bits.count_field = __builtin_ctz(extracted) & 0x1F;
    
    sink = bits.parity_field + bits.count_field;
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests */
    test_bitfield_ops();
    test_subreg_patterns();
    test_complex_addressing();
    test_combined_patterns();
    test_builtin_patterns();
    
    /* Use results to prevent dead code elimination */
    total = sink;
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
