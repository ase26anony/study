/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 3;
    } bf = {0};
    
    /* Variables to force register use */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field1 = (a & 0xF) + (b & 0x7);          /* 4-bit field */
    bf.field2 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit field */
    bf.field3 = (b >> 8) | (c >> 4);            /* 12-bit field */
    bf.field4 = __builtin_parity(a) & 0x7;      /* 3-bit field with builtin */
    
    /* Read back to prevent elimination */
    sink = bf.field1 + bf.field2 + bf.field3 + bf.field4;
}

/* Test 2: SUBREG operations through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri1 = 0x12345678;
    register int ri2 = 0x9ABCDEF0;
    register long rl1 = 0x1122334455667788ULL;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs1 = (short)ri1;                     /* int -> short */
    vs2 = (short)(ri1 + ri2);             /* arithmetic then narrowing */
    vs3 = (short)((ri1 * 3) / 7);         /* complex computation then narrowing */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 50;
    vc1 = c1 + c2;                        /* char + char -> char (implicit truncation) */
    vc2 = (c1 * c2) / 25;                 /* more complex narrowing */
    
    /* Use builtins that return int but store to smaller types */
    vc1 = __builtin_popcount(ri1) & 0xFF; /* int result narrowed to char */
    
    /* Read back */
    sink = vs1 + vs2 + vs3 + vc1 + vc2;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int arr1[256] __attribute__((aligned(16)));
    short arr2[512] __attribute__((aligned(8)));
    int arr3[128][4] __attribute__((aligned(32)));
    
    /* Register values for sources */
    register int rval1 = 0x11111111;
    register int rval2 = 0x22222222;
    register int rval3 = 0x33333333;
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        /* Multi-dimensional with non-linear index */
        int idx1 = (i * 17 + 23) % 256;
        int idx2 = (i * 13 + 7) % 512;
        int idx3 = (i * 3 + 11) % 128;
        int idx4 = (i * 5 + 19) % 4;
        
        /* Complex addressing patterns */
        arr1[idx1] = rval1 + i;                     /* 1D array with computed index */
        arr2[idx2] = (short)(rval2 ^ i);            /* 1D array with narrowing */
        arr3[idx3][idx4] = rval3 - i;               /* 2D array access */
        
        /* Pointer arithmetic with multiple offsets */
        int *ptr1 = arr1 + idx1 + (i & 3);
        *ptr1 = rval1 * i;
        
        /* Struct-like access through pointer */
        short *ptr2 = &arr2[idx2] + (i % 8);
        *ptr2 = (short)(rval2 >> (i & 0xF));
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr1[i] + arr2[i] + arr3[i][0];
    }
    sink = sum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct with mixed members */
    struct combined {
        volatile unsigned int flags : 16;
        volatile short data[8];
        volatile int counter;
    } cmb __attribute__((aligned(16)));
    
    /* Initialize */
    cmb.flags = 0;
    cmb.counter = 0;
    for (int i = 0; i < 8; i++) cmb.data[i] = 0;
    
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = 0x11223344;
    
    /* Combined assignment: bitfield + array with complex index */
    int idx = (r1 & 0x7);  /* 0-7 */
    
    /* May generate both ZERO_EXTRACT (bitfield) and SUBREG (short store) */
    cmb.flags = (r1 & 0xFFFF) | ((r2 >> 16) & 0xFFFF);
    cmb.data[idx] = (short)(r3 + idx);  /* Narrowing store with computed index */
    cmb.counter = r1 + r2 + r3;
    
    /* Inline assembly to directly influence RTL generation */
    /* Memory output with complex addressing */
    asm volatile (
        "# Force complex memory operand\n"
        : "=m" (cmb.data[(r2 & 0x3) + 2])  /* Complex addressing */
        : 
        : "memory"
    );
    
    sink = cmb.flags + cmb.data[0] + cmb.counter;
}

/* Test 5: Additional patterns for specific architectures */
void test_architecture_specific(void) {
    /* Operations that may generate STRICT_LOW_PART on some architectures */
    volatile struct {
        unsigned char low : 4;
        unsigned char high : 4;
    } packed __attribute__((packed));
    
    register unsigned int val = 0xDEADBEEF;
    
    /* Multiple bitfield assignments */
    packed.low = val & 0xF;
    packed.high = (val >> 4) & 0xF;
    
    /* Sub-word memory operations with shifting */
    volatile uint16_t mem16[4];
    volatile uint8_t mem8[8];
    
    for (int i = 0; i < 4; i++) {
        /* May generate different RTL patterns based on architecture */
        mem16[i] = (uint16_t)(val >> (i * 4));
        mem8[i * 2] = (uint8_t)(val >> (i * 8));
        mem8[i * 2 + 1] = (uint8_t)(val >> (i * 8 + 4));
    }
    
    /* Compute sum */
    unsigned int sum = packed.low + packed.high;
    for (int i = 0; i < 4; i++) {
        sum += mem16[i] + mem8[i * 2] + mem8[i * 2 + 1];
    }
    sink = sum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    total_checksum += sink;
    
    test_subreg_operations();
    total_checksum += sink;
    
    test_complex_addressing();
    total_checksum += sink;
    
    test_combined_patterns();
    total_checksum += sink;
    
    test_architecture_specific();
    total_checksum += sink;
    
    /* Final output to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
