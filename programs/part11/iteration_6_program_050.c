/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all coverage.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 17;
} g_bfs = {0};

/* Force memory addressing modes with noinline function */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    s->d = (y >> 16) & 0x7;
    s->e = (x ^ y) & 0x1FFFF;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* int -> char truncation */
        
        /* More SUBREG patterns through arithmetic */
        int val = shorts[i] * chars[i];         /* short/char -> int promotion */
        ints[i] = (val << 8) | (chars[i] & 0xFF);
    }
}

/* Complex addressing with 2D array */
__attribute__((noinline))
int complex_addressing(int arr[100][100], int idx1, int idx2) {
    volatile int vi = idx1;
    volatile int vj = idx2;
    
    /* Complex memory addressing that will generate XEXP patterns */
    int val = arr[vi % 100][vj % 100];
    
    /* Combine with bit-field like operation */
    return (val & 0xFFF) | ((val >> 12) & 0xFFF00);
}

/* Function with register pressure to trigger reload pass */
__attribute__((optimize("O0")))  /* Prevent optimizations that might simplify RTL */
void high_register_pressure(void) {
    /* Many local variables to force register spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed-width operations generating SUBREG */
    s1 = v1; s2 = v2; s3 = v3; s4 = v4; s5 = v5;
    c1 = v1; c2 = v2; c3 = v3; c4 = v4; c5 = v5;
    
    /* Inline assembly to clobber registers and increase pressure */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* More operations to use the clobbered registers */
    v1 = s1 + c1;
    v2 = s2 * c2;
    v3 = s3 - c3;
    v4 = s4 / (c4 ? c4 : 1);
    v5 = s5 | c5;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Trigger ZERO_EXTRACT/STRICT_LOW_PART patterns */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Trigger SUBREG patterns with mixed-width operations */
    volatile short short_arr[100];
    volatile char char_arr[100];
    int int_arr[100];
    
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * argc;
    }
    
    mixed_width_ops(short_arr, char_arr, int_arr, argc % 50 + 10);
    
    /* 3. Complex addressing modes */
    int matrix[100][100];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    volatile int idx1 = argc;
    volatile int idx2 = argc * 3;
    result += complex_addressing(matrix, idx1, idx2);
    
    /* 4. High register pressure to trigger reload pass */
    high_register_pressure();
    
    /* 5. Combine results to prevent dead code elimination */
    result += g_bfs.a + g_bfs.b + g_bfs.c;
    for (int i = 0; i < 10; i++) {
        result += short_arr[i] + char_arr[i] + int_arr[i];
    }
    
    /* Additional bit-field operations on local volatile struct */
    volatile struct {
        unsigned int f1 : 5;
        unsigned int f2 : 11;
        unsigned int f3 : 16;
    } local_bf = {0};
    
    local_bf.f1 = result & 0x1F;
    local_bf.f2 = (result >> 5) & 0x7FF;
    local_bf.f3 = (result >> 16) & 0xFFFF;
    
    /* More SUBREG patterns through pointer casting */
    int *int_ptr = &result;
    short *short_ptr = (short*)int_ptr;
    char *char_ptr = (char*)int_ptr;
    
    *short_ptr = (*int_ptr >> 8) & 0xFFFF;
    *char_ptr = (*int_ptr >> 16) & 0xFF;
    
    /* Final computation to use all values */
    result = local_bf.f1 + local_bf.f2 + local_bf.f3 + 
             *short_ptr + *char_ptr + g_bfs.d + g_bfs.e;
    
    printf("Result: %d\n", result);
    return result != 0;
}
