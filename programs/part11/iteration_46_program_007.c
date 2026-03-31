/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource marking:
 * 1. ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * 2. SUBREG in SET_DEST
 * 3. MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
KEEP_REGISTER static void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Register variables to force register operations */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* Complex bitfield store */
    bf.field12 = (c & 0xFFF) | ((a & 0xF) << 8); /* Mixed operations */
    
    /* Use __builtin_popcount on sub-word data */
    unsigned char byte_val = (unsigned char)(a & 0xFF);
    int popcnt = __builtin_popcount(byte_val);  /* May involve bit extraction */
    (void)popcnt;  /* Prevent unused variable warning */
    
    /* Read back to prevent elimination */
    volatile unsigned int read_back = bf.field4 + bf.field8 + bf.field12;
    (void)read_back;
}

/* Test 2: SUBREG operations in assignments */
KEEP_REGISTER static void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri = 0x12345678;
    register short rs = 0xABCD;
    register char rc = 0x42;
    
    /* Explicit narrowing casts - should generate SUBREG */
    vs1 = (short)ri;                    /* int -> short with SUBREG */
    vs2 = (short)(ri + 0x1000);         /* Arithmetic then narrowing */
    vc1 = (char)rs;                     /* short -> char */
    vc2 = (char)(rc * 2);               /* char operation with truncation */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 50;
    char sum = c1 + c2;                 /* May generate SUBREG for truncation */
    vs1 = sum;                          /* Store truncated result */
    
    /* Mixed-size operations */
    short result = (short)((ri & 0xFFFF) + (rs & 0xFFFF));
    vs2 = result;                       /* Another SUBREG store */
    
    /* Read back */
    volatile int sum_check = vs1 + vs2 + vc1 + vc2;
    (void)sum_check;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
KEEP_REGISTER static void test_complex_addressing(void) {
    /* Multi-dimensional array */
    int arr2d[16][8] = {{0}};
    
    /* Struct with array member */
    struct data {
        int values[32];
        short shorts[64];
    } data_obj = {{0}};
    
    /* Pointer with restrict to prevent aliasing assumptions */
    int* restrict ptr = &arr2d[0][0];
    
    register int r1 = 0x1111;
    register int r2 = 0x2222;
    register int r3 = 0x3333;
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index: i*3 + 7 */
        int idx = i * 3 + 7;
        
        /* Store with complex addressing */
        arr2d[idx % 16][i] = r1 + i;           /* 2D array with modulo */
        ptr[idx * 2] = r2 - i;                 /* Pointer arithmetic */
        
        /* Struct member access with computed index */
        data_obj.values[(i * 5) % 32] = r3 * i;
        data_obj.shorts[(i * 7) % 64] = (short)(r1 + r2 + i);
    }
    
    /* Pointer chain */
    int* p1 = &arr2d[0][0];
    int* p2 = p1 + 16;
    int* p3 = p2 + 8;
    
    *p3 = r1 + r2;                            /* Multiple offset addition */
    
    /* Compute checksum to prevent elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += arr2d[i][j];
        }
    }
    (void)checksum;
}

/* Test 4: Combined patterns */
KEEP_REGISTER static void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned int status : 4;
        short data[32];
    } comb = {0};
    
    /* Array with restrict for complex addressing */
    short* restrict sptr = comb.data;
    
    register int reg_val = 0x89ABCDEF;
    register short reg_short = 0x1234;
    register unsigned int reg_bits = 0x5A;
    
    /* Combined assignment 1: Bitfield with complex expression */
    comb.flags = (reg_bits & 0xF) | ((reg_val & 0xF0) >> 4);
    
    /* Combined assignment 2: Array with complex index and narrowing */
    for (int i = 0; i < 16; i++) {
        int idx = (i * 13 + 7) % 32;          /* Complex index */
        sptr[idx] = (short)(reg_val + i * 256); /* Narrowing store */
        
        /* Alternate: direct struct access */
        comb.data[(i * 17) % 32] = (short)(reg_short - i);
    }
    
    /* Combined assignment 3: Bitfield from narrowed value */
    comb.status = (unsigned int)(reg_short & 0xF);
    
    /* Inline assembly to force specific RTL patterns */
    asm volatile (
        "# Force memory store with complex addressing\n"
        : "=m" (comb.data[reg_val & 0x1F])  /* Complex address constraint */
        :
        : "memory"
    );
    
    /* Compute verification sum */
    volatile int verify = comb.flags + comb.status;
    for (int i = 0; i < 32; i++) {
        verify += comb.data[i];
    }
    (void)verify;
}

/* Test 5: Additional patterns for coverage */
KEEP_REGISTER static void test_additional_patterns(void) {
    /* Use __builtin_parity on sub-word data */
    unsigned short us = 0xBEEF;
    int parity = __builtin_parity(us);        /* May use ZERO_EXTRACT */
    (void)parity;
    
    /* Bitfield in union for different access patterns */
    union {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } bits;
        unsigned int full;
    } u = {0};
    
    u.bits.low = 0x1234;                      /* Bitfield store */
    u.bits.high = 0x5678;                     /* Another bitfield store */
    
    /* Volatile pointer to volatile bitfield */
    volatile union* vu = &u;
    vu->bits.low = 0x9ABC;                    /* Volatile bitfield access */
    
    /* Array of bitfields */
    struct {
        unsigned int val : 3;
    } bitarray[8];
    
    for (int i = 0; i < 8; i++) {
        bitarray[i].val = (i & 0x7);          /* Array of bitfield stores */
    }
    
    /* Read back everything */
    volatile unsigned int total = u.full;
    for (int i = 0; i < 8; i++) {
        total += bitarray[i].val;
    }
    (void)total;
}

int main(void) {
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all test patterns */
    test_bitfield_operations();
    test_subreg_operations();
    test_complex_addressing();
    test_combined_patterns();
    test_additional_patterns();
    
    printf("All tests completed.\n");
    
    /* Return non-zero to ensure all code paths are considered */
    return 0;
}
