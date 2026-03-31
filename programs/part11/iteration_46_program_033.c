/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
KEEP_REGISTER
static int test_bitfields(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bf = {0};
    
    /* Register variables to force register operations */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0x7);      /* 4-bit field with computation */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* 8-bit field */
    bf.field12 = (c & 0xFFF) | ((a & 0xF) << 8);       /* 12-bit field */
    
    /* Read back to prevent elimination */
    return bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations for SUBREG */
KEEP_REGISTER  
static int test_subreg(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri1 = 0x12345678;
    register int ri2 = 0x9ABCDEF0;
    register short rs1 = 0x1234;
    register char rc1 = 0x56;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs1 = (short)ri1;                     /* int -> short */
    vs2 = (short)(ri1 + ri2);             /* expression with narrowing */
    vs3 = (short)(rs1 * 2 + 0x100);       /* short computation */
    
    /* Char operations with implicit truncation */
    vc1 = (char)(ri1 >> 16);              /* int -> char */
    vc2 = rc1 + (char)(ri2 & 0xFF);       /* char expression */
    
    /* Arithmetic that forces promotion then demotion */
    char c1 = 100, c2 = 200;
    volatile char vc3 = c1 + c2;          /* overflow truncation */
    
    return vs1 + vs2 + vs3 + vc1 + vc2 + vc3;
}

/* Test 3: Complex memory addressing for MEM_P */
KEEP_REGISTER
static int test_complex_mem(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    int * restrict ptr1 = &arr[0][0];  /* restrict helps keep addressing */
    
    /* Register values to store */
    register int rval1 = 0x11111111;
    register int rval2 = 0x22222222;
    register int rval3 = 0x33333333;
    
    /* Complex address calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index computation */
        int idx = i * 7 + 3;                    /* i*stride + offset */
        arr[idx % 64][i] = rval1 + i;           /* 2D array access */
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr1 + idx + i*2) = rval2 - i;        /* base + idx + i*2 */
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[16];
        int footer;
    } s = {0};
    
    /* Access through pointer with offset */
    int *ptr2 = &s.header;
    for (int i = 0; i < 4; i++) {
        ptr2[5 + i*3] = rval3 + i;              /* ptr->array[index] pattern */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sum += s.header + s.footer;
    
    return sum;
}

/* Test 4: Combined patterns */
KEEP_REGISTER
static int test_combined(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        unsigned int count : 4;
        short data[16];
        volatile int sync;
    } combined = {0};
    
    register int source = 0x89ABCDEF;
    register short ssource = 0x1234;
    
    /* Combined assignment: bitfield + array with complex index */
    combined.flags = (source & 0xFF) ^ 0x55;          /* ZERO_EXTRACT candidate */
    
    /* Complex index calculation */
    int idx = ((source >> 8) & 0xF) * 3 + 1;          /* non-linear */
    combined.data[idx % 16] = (short)source;          /* SUBREG candidate */
    
    /* Another bitfield with computation */
    combined.count = ((source >> 16) & 0xF) + ((source >> 20) & 0x7);
    
    /* Memory access through pointer */
    short *dptr = combined.data;
    dptr[(idx + 5) % 16] = ssource + (short)(source & 0xFF);  /* complex addressing */
    
    /* Use inline assembly for direct RTL influence */
    int dummy = 0;
    asm volatile (
        "# Force memory operand with complex addressing\n"
        : "=m" (combined.data[(idx + 3) % 16])  /* memory output constraint */
        : 
        : "memory"
    );
    
    /* Compute checksum */
    int sum = combined.flags + combined.count + combined.sync;
    for (int i = 0; i < 16; i++) {
        sum += combined.data[i];
    }
    
    return sum + dummy;
}

/* Test 5: Bit manipulation builtins */
KEEP_REGISTER
static int test_builtins(void) {
    volatile struct {
        unsigned short field;
        unsigned char bits;
    } v = {0};
    
    register unsigned int x = 0x13579BDF;
    
    /* Builtins on sub-word data may involve bit extraction */
    v.bits = __builtin_parity(x & 0xFF) | 
             (__builtin_popcount(x & 0xFFF) & 0xF);
    
    /* Extract and store specific bit ranges */
    unsigned short mask = 0x3F;  /* 6 bits */
    v.field = (__builtin_popcount(x) & mask) | 
              ((__builtin_parity(x >> 16) << 6) & 0x40);
    
    return v.bits + v.field;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern coverage...\n");
    
    /* Execute all tests */
    checksum += test_bitfields();
    checksum += test_subreg();
    checksum += test_complex_mem();
    checksum += test_combined();
    checksum += test_builtins();
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Additional volatile store to force RTL generation */
    volatile int sink = checksum;
    
    return sink != 0 ? 0 : 1;
}
