/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload coverage.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 17;
} S;

/* Force memory addressing and prevent optimization */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    s->d = (x ^ y) & 0x7;
    s->e = (x + y) & 0x1FFFF;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(volatile short *shorts, volatile char *chars, 
                           volatile int *ints, int count) {
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* int -> char truncation */
        
        /* More complex mixed-width expressions */
        int temp = shorts[i] * chars[i];        /* short/char -> int promotion */
        ints[i] = (temp << 8) | (chars[i] & 0xFF);
    }
}

/* Complex addressing mode generator */
__attribute__((noinline))
int complex_addressing(int arr[][100], int idx1, int idx2, int mask) {
    /* Generate complex memory addressing with bit operations */
    int val = arr[idx1][idx2];
    /* Bit operation that might generate ZERO_EXTRACT */
    return (val & mask) | ((val >> 16) & 0xFFFF);
}

int main(int argc, char *argv[]) {
    /* Ensure non-constant loop bounds */
    int iterations = argc > 1 ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* 1. Trigger bit-field patterns (ZERO_EXTRACT/STRICT_LOW_PART) */
    modify_bitfields((struct BitFieldStruct *)&S, argc, iterations);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_arr[100];
    volatile char char_arr[100];
    volatile int int_arr[100];
    
    /* Initialize with varying values */
    for (int i = 0; i < 100 && i < iterations * 10; i++) {
        int_arr[i] = (i * 37) ^ argc;
    }
    
    mixed_width_operations(short_arr, char_arr, int_arr, 
                          iterations < 100 ? iterations : 100);
    
    /* 3. Complex addressing modes with 2D array */
    int arr_2d[100][100];
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = (i * 100 + j) ^ argc;
        }
    }
    
    /* Use volatile indices to prevent constant propagation */
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 17) % 50;
    
    /* Complex addressing with bit operations */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Vary indices slightly each iteration */
        int access_i = (idx_i + i) % 50;
        int access_j = (idx_j + i * 3) % 50;
        
        /* This should generate MEM with complex address */
        result ^= complex_addressing(arr_2d, access_i, access_j, 0x00FF00FF);
        
        /* Additional mixed-width store to generate SUBREG as SET_DEST */
        short_arr[access_i] = arr_2d[access_i][access_j] & 0x7FFF;
    }
    
    /* 4. Increase register pressure with inline assembly */
    asm volatile ("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 5. More bit-field operations combined with memory accesses */
    struct BitFieldStruct local_s;
    volatile struct BitFieldStruct *volatile ptr = &local_s;
    
    /* Chain of bit-field assignments */
    ptr->a = result & 0xF;
    ptr->b = (result >> 4) & 0xFF;
    ptr->c = (result >> 12) & 0xFFFFF;
    
    /* Mixed operation that might generate SUBREG for memory */
    int temp = ptr->b;
    short_arr[0] = temp;  /* int -> short store */
    
    /* 6. Complex expression combining everything */
    int final_result = 
        (S.a + S.b) * (short_arr[idx_i % 10] - char_arr[idx_j % 10]) +
        complex_addressing(arr_2d, idx_i % 50, idx_j % 50, result);
    
    /* Prevent dead code elimination */
    printf("Result: %d (argc=%d)\n", final_result, argc);
    
    /* Additional memory pressure */
    volatile int pressure[100];
    for (int i = 0; i < 100; i++) {
        pressure[i] = final_result + i;
        asm volatile ("" : "+r" (pressure[i]) : : "memory");
    }
    
    return final_result != 0 ? 0 : 1;
}
