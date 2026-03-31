/* test_resource_marking.c
 * Designed to trigger uncovered lines in resource.cc (lines 282-290)
 * Specifically targets: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG in SET_DEST,
 * and MEM_P with complex addressing.
 */

#include <stdio.h>
#include <stdint.h>

/* Force noinline to prevent optimization of test patterns */
#define NOINLINE __attribute__((noinline))

/* Test 1: Bitfield operations to generate ZERO_EXTRACT */
NOINLINE static unsigned int test_bitfield_extract(void) {
    volatile struct {
        unsigned int field4 : 4;    /* Likely ZERO_EXTRACT for 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex RHS */
    bf.field4 = (a & 0xF) + (b & 0x7);      /* Should generate ZERO_EXTRACT */
    bf.field8 = (b >> 4) & 0xFF;            /* Another ZERO_EXTRACT candidate */
    bf.field12 = (a ^ b ^ c) & 0xFFF;       /* Complex expression to bitfield */
    
    /* Use __builtin_parity on sub-word data */
    unsigned int parity_val = __builtin_parity(bf.field8);
    bf.field4 = parity_val & 0xF;           /* Another bitfield store */
    
    return bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG generation through type narrowing */
NOINLINE static unsigned int test_subreg_narrowing(void) {
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register variables to encourage SUBREG in SET_DEST */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = r1 + r2;
    
    /* Explicit narrowing casts - potential SUBREG in destination */
    vs1 = (short)r1;                     /* int -> short */
    vs2 = (short)(r1 * r2);              /* Complex expression narrowed */
    vs3 = (short)(r1 & 0xFFFF);          /* Masked then narrowed */
    
    /* char operations with implicit truncation */
    char c1 = 100, c2 = 200;
    vc1 = c1 + c2;                       /* Overflow truncation to char */
    vc2 = (c1 * c2) >> 4;                /* Complex char operation */
    
    /* Mixed-type arithmetic with narrowing store */
    short s1 = 1000, s2 = 2000;
    vc1 = (char)(s1 + s2);               /* short -> char narrowing */
    
    return vs1 + vs2 + vs3 + vc1 + vc2;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
NOINLINE static unsigned int test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    unsigned int sum = 0;
    
    /* Complex address calculation */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-linear index calculation */
            int idx = i * 13 + j * 7 + 3;
            idx = idx & 63;  /* Keep within bounds */
            
            /* Store with complex addressing */
            register int val = i * 100 + j;
            arr[idx][j] = val;           /* 2D array with computed index */
            
            /* Pointer arithmetic with multiple offsets */
            int *ptr = &arr[0][0];
            ptr += idx * 8 + j;          /* Manual 2D to 1D conversion */
            *ptr = *ptr + val;           /* Complex memory store */
        }
    }
    
    /* Struct with array member accessed via pointer */
    struct {
        int data[32];
        int extra;
    } s = {0};
    
    int *restrict p = s.data;  /* Use restrict to help optimizer */
    for (int i = 0; i < 16; i++) {
        /* Complex addressing within struct array */
        p[i * 2 + 1] = i * i;            /* Non-contiguous access pattern */
    }
    
    /* Compute checksum to prevent elimination */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    for (int i = 0; i < 32; i++) {
        sum += s.data[i];
    }
    
    return sum;
}

/* Test 4: Combined patterns in single assignments */
NOINLINE static unsigned int test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        short values[16];
        unsigned int status : 4;
    } combined = {0};
    
    unsigned int checksum = 0;
    
    /* Combined: bitfield store with complex expression */
    register int r = 0x89ABCDEF;
    combined.flags = (r >> 16) & 0xFF;   /* ZERO_EXTRACT candidate */
    
    /* Combined: array store with narrowing and complex index */
    for (int i = 0; i < 8; i++) {
        register int src = r + i * 0x1111;
        /* Narrowing store to array with computed index */
        combined.values[i * 2] = (short)src;  /* SUBREG candidate */
        
        /* More complex index calculation */
        int idx = (i * 3 + 7) & 15;
        combined.values[idx] = (short)(src ^ 0xAAAA);  /* Complex addressing */
    }
    
    /* Another bitfield store */
    combined.status = __builtin_popcount(r) & 0xF;
    
    /* Compute checksum */
    for (int i = 0; i < 16; i++) {
        checksum += combined.values[i];
    }
    checksum += combined.flags + combined.status;
    
    return checksum;
}

/* Test 5: Inline assembly for direct RTL influence */
NOINLINE static unsigned int test_inline_asm(void) {
    int array[32] = {0};
    unsigned int sum = 0;
    
    /* Use inline assembly to create memory stores with complex addressing */
    for (int i = 0; i < 8; i++) {
        int idx = i * 5 + 3;
        
        /* Assembly with memory output and complex addressing */
        asm volatile (
            "# Force memory store with complex address\n"
            : "=m" (array[idx])   /* Complex addressing in constraint */
            : 
            : "memory"
        );
        
        /* Another with more complex computation */
        idx = (i * 7 + 11) & 31;
        int val = i * 100;
        asm volatile (
            "# Store with value\n"
            : "=m" (array[idx])
            : "r" (val)
            : "memory"
        );
    }
    
    /* Compute sum to prevent elimination */
    for (int i = 0; i < 32; i++) {
        sum += array[i];
    }
    
    return sum;
}

int main(void) {
    unsigned int total = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Run all tests sequentially */
    total += test_bitfield_extract();
    printf("Bitfield test complete\n");
    
    total += test_subreg_narrowing();
    printf("Subreg narrowing test complete\n");
    
    total += test_complex_addressing();
    printf("Complex addressing test complete\n");
    
    total += test_combined_patterns();
    printf("Combined patterns test complete\n");
    
    total += test_inline_asm();
    printf("Inline assembly test complete\n");
    
    /* Print final checksum to ensure all code executed */
    printf("Final checksum: %u\n", total);
    
    return 0;
}
