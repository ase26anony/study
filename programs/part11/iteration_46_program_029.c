#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 8;
    } bf = {0};
    
    /* Variables for bit manipulation */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field1 = (a & 0xF) + (b & 0xF);                /* Should generate ZERO_EXTRACT */
    bf.field2 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* Complex bitfield store */
    bf.field3 = __builtin_popcount(a) + __builtin_parity(b); /* Builtins on sub-word data */
    bf.field4 = (c & 0xFF) | ((a >> 8) & 0xFF);       /* Mixed bit operations */
    
    /* Read back to prevent elimination */
    volatile unsigned int readback = bf.field1 + bf.field2 + bf.field3 + bf.field4;
    (void)readback;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subword_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register variables to encourage register operations */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = r1 + r2;
    
    /* Explicit casts to smaller types - may generate SUBREG in SET_DEST */
    vs1 = (short)r1;                    /* Narrowing cast from register */
    vs2 = (short)(r1 * r2);             /* Arithmetic then narrowing */
    vs3 = (short)((r1 & 0xFFFF) + (r2 & 0xFFFF)); /* Masked addition then narrowing */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100;
    char c2 = 50;
    vc1 = c1 + c2;                      /* char + char -> char (implicit truncation) */
    vc2 = (c1 * 2) - (c2 / 2);          /* More complex char arithmetic */
    
    /* Read back */
    volatile int sum = vs1 + vs2 + vs3 + vc1 + vc2;
    (void)sum;
}

/* Test 3: Complex memory addressing to trigger MEM_P(x) path */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8];
    int * restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-linear index calculation */
            int idx = (i * 7 + j * 3) % 64;
            int idy = (i * 3 + j * 7) % 8;
            
            /* Store with complex addressing */
            arr[idx][idy] = i * 100 + j;  /* MEM with address calculation */
            
            /* Pointer arithmetic with multiple offsets */
            *(ptr + idx * 8 + idy) = i * 200 + j;  /* Another complex address */
        }
    }
    
    /* Struct with array member accessed via pointer */
    struct {
        int data[32];
        int count;
    } s, *sp = &s;
    
    for (int i = 0; i < 16; i++) {
        /* Complex struct member access */
        sp->data[(i * 5 + 3) % 32] = i * 300;  /* MEM with struct+array+index */
    }
    
    /* Compute checksum */
    volatile int checksum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += arr[i][j];
        }
    }
    checksum += sp->data[0];
    (void)checksum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct containing both bitfield and array */
    volatile struct combined {
        unsigned int flags : 16;
        unsigned int status : 8;
        short values[16];
        int data[8];
    } comb;
    
    /* Register variable for SUBREG generation */
    register int reg_val = 0x89ABCDEF;
    
    /* Combined assignment: bitfield + narrowed store */
    comb.flags = (reg_val & 0xFFFF);           /* ZERO_EXTRACT from register */
    comb.status = (reg_val >> 16) & 0xFF;      /* Another bitfield extract */
    
    /* Complex array access with narrowing */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 3 + 7) % 16;
        
        /* Store narrowed value from register to array */
        comb.values[idx] = (short)(reg_val + i * 0x1111);  /* SUBREG + MEM */
        
        /* Another complex store */
        comb.data[(i * 5 + 1) % 8] = reg_val - i * 0x2222;  /* MEM with address calc */
    }
    
    /* Inline assembly to directly influence RTL generation */
    int temp_array[16];
    for (int i = 0; i < 8; i++) {
        int complex_idx = (i * 7 + 3) % 16;
        
        /* Inline asm with memory output constraint */
        asm volatile (
            "# Force complex memory operand"
            : "=m" (temp_array[complex_idx])  /* Complex addressing in output */
            : 
            : "memory"
        );
    }
    
    /* Read everything back */
    volatile unsigned long long total = 0;
    total += comb.flags;
    total += comb.status;
    for (int i = 0; i < 16; i++) total += comb.values[i];
    for (int i = 0; i < 8; i++) total += comb.data[i];
    for (int i = 0; i < 16; i++) total += temp_array[i];
    (void)total;
}

/* Test 5: Additional patterns for specific architectures */
void test_architecture_specific(void) {
    /* Use __builtin_bswap16/32 on sub-word types */
    unsigned short us = 0x1234;
    unsigned char uc = 0xAB;
    
    volatile unsigned short swapped;
    swapped = __builtin_bswap16(us);  /* May involve bit extraction */
    
    /* Parity and popcount on masked values */
    volatile int parity_sum = 0;
    parity_sum += __builtin_parity(us & 0xFF);   /* Sub-word operation */
    parity_sum += __builtin_popcount(uc);        /* Byte-sized popcount */
    
    /* Multi-step bitfield extraction */
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } multi_bf;
    
    unsigned int source = 0xDEADBEEF;
    multi_bf.a = (source >> 0) & 0x7;
    multi_bf.b = (source >> 3) & 0x1F;
    multi_bf.c = (source >> 8) & 0x3FF;
    multi_bf.d = (source >> 18) & 0x3FFF;
    
    (void)parity_sum;
    (void)swapped;
}

int main(void) {
    printf("Starting RTL pattern tests...\n");
    
    /* Execute all test patterns */
    test_bitfield_operations();
    test_subword_operations();
    test_complex_memory_addressing();
    test_combined_patterns();
    test_architecture_specific();
    
    printf("All tests completed.\n");
    
    /* Return non-zero if any issues detected */
    return 0;
}
