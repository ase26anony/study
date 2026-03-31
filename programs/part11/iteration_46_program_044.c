/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
static void test_bitfield_ops(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    /* Variables to force register usage */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) ^ (b & 0xF);          /* 4-bit extract and store */
    bf.field8 = ((a >> 4) & 0xFF) + ((b >> 4) & 0xFF); /* 8-bit extract with arithmetic */
    bf.field12 = (c & 0xFFF) | ((a & 0xF) << 8); /* 12-bit with composition */
    
    /* Use __builtin_popcount on sub-word data - may involve bit extraction */
    unsigned char byte_val = (a & 0xFF);
    int popcnt = __builtin_popcount(byte_val);
    bf.field4 = popcnt & 0xF;  /* Store result back to bitfield */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(bf.field4), "r"(bf.field8), "r"(bf.field12));
}

/* Test 2: SUBREG operations through type narrowing */
static void test_subreg_ops(void) {
    /* Volatile short destination for SUBREG store */
    volatile short vs;
    volatile char vc;
    
    /* Register variables to ensure they stay in registers */
    register int reg_int = 0x12345678;
    register long reg_long = 0x9ABCDEF0;
    
    /* Explicit narrowing casts that may generate SUBREG */
    vs = (short)reg_int;                     /* int -> short SUBREG */
    vc = (char)(reg_int >> 16);              /* int -> char SUBREG */
    
    /* Arithmetic with implicit narrowing */
    unsigned char uc1 = 200;
    unsigned char uc2 = 100;
    vc = uc1 + uc2;                          /* char + char -> char with potential overflow */
    
    /* More complex narrowing */
    vs = (short)((reg_int * 3) / 7);         /* Arithmetic then narrowing */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(vs), "r"(vc));
}

/* Test 3: Complex memory addressing for MEM_P(x) */
static void test_complex_mem_ops(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr2d[16][16];
    int * restrict ptr = &arr2d[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Complex addressing: i*stride + j with non-linear terms */
            int idx = i * (16 + (i % 3)) + (j * 2) + (i & j);
            arr2d[i][j] = idx * 3;           /* Store with complex address */
            
            /* Pointer arithmetic with multiple offsets */
            *(ptr + idx + (i * j)) = idx * 5;
        }
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s;
    
    /* Access struct member through pointer with offset */
    int *data_ptr = s.data;
    for (int i = 0; i < 16; i++) {
        /* Complex index calculation */
        int offset = (i * 3 + 7) & 31;
        data_ptr[offset] = i * i;            /* Store with computed offset */
    }
    
    /* Prevent optimization */
    asm volatile("" : : "m"(arr2d), "m"(s));
}

/* Test 4: Combined patterns */
static void test_combined_ops(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        short values[16];
        unsigned int status : 4;
    } combined = {0};
    
    /* Register variables */
    register int r1 = 0x1234;
    register int r2 = 0x5678;
    
    /* Combined assignment: bitfield store (ZERO_EXTRACT) */
    combined.flags = (r1 & 0xFF) | ((r2 >> 4) & 0xFF);
    
    /* Array store with narrowing (SUBREG + complex addressing) */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 5 + 3) & 15;
        
        /* Narrowing store: int -> short */
        combined.values[idx] = (short)((r1 * i) + (r2 / (i + 1)));
        
        /* Update bitfield in same loop */
        combined.status = (combined.status + i) & 0xF;
    }
    
    /* Inline assembly to directly influence RTL generation */
    short temp;
    asm volatile(
        "movw %1, %0\n\t"          /* Force a move with potential SUBREG */
        : "=r"(temp)
        : "r"((short)r1)
    );
    combined.values[0] = temp;
    
    /* Prevent optimization */
    asm volatile("" : : "m"(combined));
}

/* Test 5: Direct inline assembly for RTL patterns */
static void test_asm_ops(void) {
    int array[32];
    int index;
    
    /* Complex addressing in asm output */
    for (int i = 0; i < 16; i++) {
        index = (i * 7 + 11) & 31;
        
        /* Inline asm with memory output and complex addressing */
        asm volatile(
            "# Force memory store with complex address\n\t"
            : "=m"(array[index])   /* Complex addressing in constraint */
            : 
            : "memory"
        );
    }
    
    /* Bitfield-like operation through asm */
    struct {
        volatile unsigned int low : 16;
        volatile unsigned int high : 16;
    } split;
    
    unsigned int value = 0x12345678;
    
    /* Simulate STRICT_LOW_PART-like behavior */
    asm volatile(
        "mov {%1, %0 | %0, %1}\n\t"
        : "=r"(split.low), "=r"(split.high)
        : "r"(value)
    );
    
    /* Prevent optimization */
    asm volatile("" : : "m"(array), "m"(split));
}

int main(void) {
    int checksum = 0;
    
    /* Execute all tests */
    test_bitfield_ops();
    checksum += 1;
    
    test_subreg_ops();
    checksum += 2;
    
    test_complex_mem_ops();
    checksum += 3;
    
    test_combined_ops();
    checksum += 4;
    
    test_asm_ops();
    checksum += 5;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum == 15 ? 0 : 1;
}
