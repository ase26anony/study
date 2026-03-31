/* test_resource_marking.c - Generate RTL patterns for ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P in SET_DEST */

#include <stdio.h>
#include <stdint.h>

/* Helper to prevent optimization */
static int checksum = 0;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Source values with bitwise operations */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex RHS */
    bf.field4 = (a & 0xF) + (b & 0xF);          /* Could generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF);  /* Complex expression */
    bf.field12 = __builtin_popcount(c) & 0xFFF; /* Builtin with masking */
    
    /* Read back to prevent elimination */
    checksum += bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG patterns through type narrowing */
void test_subreg_patterns(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register sources (hint to compiler) */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register long r3 = 0x1122334455667788ULL;
    
    /* Explicit narrowing casts - potential SUBREG in SET_DEST */
    vs1 = (short)r1;                    /* int -> short */
    vs2 = (short)(r1 + r2);             /* expression then narrowing */
    vc1 = (char)(r1 & 0xFF);            /* int -> char with mask */
    vc2 = (char)((r1 + r2) >> 8);       /* complex expression to char */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 200;
    vc1 = c1 + c2;                      /* char + char -> char (overflow truncation) */
    
    /* Read back */
    checksum += vs1 + vs2 + vc1 + vc2;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr2d[16][16];
    int arr1d[256];
    
    /* Struct with array member */
    struct {
        int data[32];
        int offset;
    } s = { .offset = 8 };
    
    /* Pointer with restrict to avoid aliasing assumptions */
    int *restrict ptr = arr1d;
    
    /* Complex address computations */
    for (int i = 0; i < 8; i++) {
        /* Multi-dimensional access with stride */
        arr2d[i][i * 2 + 3] = i * 100;          /* i*stride + constant offset */
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + i * 3 + 7) = i * 200;           /* base + i*3 + 7 */
        
        /* Struct member through pointer with index arithmetic */
        s.data[s.offset + i] = i * 300;         /* struct.base + offset + i */
    }
    
    /* Compute checksum from memory */
    for (int i = 0; i < 8; i++) {
        checksum += arr2d[i][i * 2 + 3];
        checksum += *(ptr + i * 3 + 7);
        checksum += s.data[s.offset + i];
    }
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        short values[16];
    } combined = {0};
    
    /* Register source */
    register int reg_val = 0x87654321;
    
    /* Combined assignment: bitfield + array with complex index */
    combined.flags = (reg_val & 0xFF) | ((reg_val >> 16) & 0xFF);
    
    /* Array assignment with narrowing cast and complex index */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 5 + 3) & 0xF;            /* Non-linear index computation */
        combined.values[idx] = (short)(reg_val + i * 1000);
    }
    
    /* Read back */
    checksum += combined.flags;
    for (int i = 0; i < 16; i++) {
        checksum += combined.values[i];
    }
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int array[32] = {0};
    int index = 7;
    
    /* Complex addressing in asm output constraint */
    asm volatile (
        "# Force MEM with complex address\n"
        : "=m" (array[index * 2 + 3])  /* MEM with computed address */
        :
        : "memory"
    );
    
    /* Another asm with potential SUBREG */
    short short_var;
    asm volatile (
        "# Potential SUBREG pattern\n"
        : "=r" (short_var)              /* Output in register, then store */
        : 
        : 
    );
    
    checksum += array[17] + short_var;
}

int main(void) {
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests sequentially */
    test_bitfield_operations();
    printf("  Bitfield test complete\n");
    
    test_subreg_patterns();
    printf("  SUBREG test complete\n");
    
    test_complex_memory_addressing();
    printf("  Complex memory addressing test complete\n");
    
    test_combined_patterns();
    printf("  Combined patterns test complete\n");
    
    test_inline_asm();
    printf("  Inline assembly test complete\n");
    
    /* Final checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
