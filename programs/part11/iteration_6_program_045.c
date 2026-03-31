/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing coverage.c -o coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs;

/* Non-inline function to force memory addressing */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s) {
    /* Multiple bit-field assignments - should generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;
    s->b = 2;
    s->c = 3;
    s->d = 4;
    
    /* Additional operations to create complex patterns */
    s->a = s->b & 0x3;  /* Bit-field operation */
    s->c = s->d | 0x7;  /* Another bit-field operation */
}

/* Another non-inline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile int *ints, volatile char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Mixed-width operations that may generate SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;  /* Truncation to 16-bit */
        chars[i] = shorts[i] & 0xFF;   /* Further truncation to 8-bit */
        
        /* Sign extension operations */
        ints[i] = (int)chars[i] * 2;   /* char to int promotion */
        
        /* Complex expression with mixed types */
        shorts[i] = (shorts[i] + (short)ints[i]) & 0x7FFF;
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    int sum = 0;
    
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex array access with bitwise operation */
    sum = arr[i][j] & 0xFFF;  /* May generate ZERO_EXTRACT */
    
    /* More complex addressing with pointer arithmetic */
    sum += *(*(arr + i) + j) | 0x1000;
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Force register pressure with many local variables */
    volatile int v1 = argc;
    volatile int v2 = argc * 2;
    volatile int v3 = argc * 3;
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2, vc3;
    
    /* 1. Bit-field operations on global volatile struct */
    modify_bitfields(&g_bfs);
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_arr[50];
    volatile int int_arr[50];
    volatile char char_arr[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 50; i++) {
        int_arr[i] = i * argc;
        short_arr[i] = i * 2;
        char_arr[i] = i & 0xFF;
    }
    
    mixed_width_ops(short_arr, int_arr, char_arr, 50);
    
    /* 3. Complex array addressing with 2D array */
    int matrix[100][100];
    volatile int idx_i = argc;
    volatile int idx_j = argc + 1;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    int matrix_sum = complex_addressing(matrix, &idx_i, &idx_j);
    
    /* 4. More mixed-width operations to increase SUBREG patterns */
    vs1 = v1;           /* int to short - potential SUBREG */
    vc1 = vs1;          /* short to char - another SUBREG */
    v2 = vc1 * 100;     /* char to int promotion */
    
    /* 5. Inline assembly to clobber registers and increase pressure */
    asm volatile("" ::: "eax", "ebx", "ecx", "edx", "memory");
    
    /* Additional volatile operations to prevent optimization */
    volatile struct {
        unsigned int f1 : 3;
        unsigned int f2 : 5;
        unsigned int f3 : 10;
    } local_bf;
    
    local_bf.f1 = v1 & 0x7;
    local_bf.f2 = v2 & 0x1F;
    local_bf.f3 = v3 & 0x3FF;
    
    /* More mixed-width casts */
    int temp_int = (int)local_bf.f1 + (int)local_bf.f2;
    short temp_short = temp_int & 0xFFFF;
    char temp_char = temp_short & 0xFF;
    
    /* Complex expression combining everything */
    int result = g_bfs.c + matrix_sum + temp_int + short_arr[0] + char_arr[0];
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional bit-field manipulation in loop */
    for (int i = 0; i < 10; i++) {
        g_bfs.a = i & 0xF;
        g_bfs.b = (i * 2) & 0xFF;
        vs2 = g_bfs.b;  /* bit-field to short */
        vc2 = vs2;      /* short to char */
    }
    
    return 0;
}
