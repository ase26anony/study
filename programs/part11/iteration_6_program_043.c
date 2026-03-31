/* Compile with: gcc -O2 -fdump-rtl-reload -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs = {0};

/* Force memory addressing modes with noinline function */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFF;
    
    /* Additional assignment to force different pattern */
    s->a = (s->b + s->c) & 0xF;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(short *shorts, char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* int -> char truncation */
        
        /* More complex mixed-width expression */
        ints[i] = (shorts[i] * chars[i]) + (ints[i] & 0xFF);
    }
}

/* Complex addressing mode generator */
__attribute__((noinline))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Complex addressing with volatile indices */
        int val = arr[*idx1 + i][*idx2 + i];
        
        /* Combine with bitwise operation that may generate ZERO_EXTRACT */
        sum += (val & 0x00FF00FF) + ((val >> 8) & 0x00FF00FF);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Force non-constant loop bounds */
    int iterations = argc > 1 ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* 1. Trigger bit-field operations */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, 0x12345678, 0x9ABCDEF0);
    
    /* 2. Mixed-width operations for SUBREG generation */
    volatile short short_arr[100];
    volatile char char_arr[100];
    volatile int int_arr[100];
    
    /* Initialize with volatile to prevent optimization */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * 0x01010101;
    }
    
    mixed_width_ops((short*)short_arr, (char*)char_arr, (int*)int_arr, 
                   iterations < 100 ? iterations : 100);
    
    /* 3. Complex addressing modes */
    int matrix[100][100];
    volatile int idx1 = 5, idx2 = 10;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    int addr_sum = complex_addressing(matrix, &idx1, &idx2, iterations % 50);
    
    /* 4. Increase register pressure with inline assembly */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 5. Additional volatile bit-field manipulation in main */
    volatile struct BitFieldStruct local_bfs = {0};
    for (int i = 0; i < iterations; i++) {
        /* Multiple assignments to same bit-field with different values */
        local_bfs.a = i & 0xF;
        local_bfs.b = (i >> 4) & 0xFF;
        local_bfs.c = (i * 3) & 0xFFF;
        local_bfs.d = (i * 5) & 0xFF;
        
        /* Mixed-width store that could generate SUBREG */
        *((volatile short*)&local_bfs.d) = (short)(i * 7);
    }
    
    /* 6. More SUBREG patterns through pointer casting */
    volatile int big_var = 0x12345678;
    volatile short *short_ptr = (volatile short*)&big_var;
    volatile char *char_ptr = (volatile char*)&big_var;
    
    for (int i = 0; i < iterations; i++) {
        *short_ptr = (*short_ptr + i) & 0xFFFF;
        *char_ptr = (*char_ptr * 3) & 0xFF;
        
        /* Force memory access with different widths */
        big_var = (*short_ptr << 16) | (*char_ptr << 8) | (big_var & 0xFF);
    }
    
    /* 7. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    
    for (int i = 0; i < (iterations < 100 ? iterations : 100); i++) {
        checksum += short_arr[i] + char_arr[i] + int_arr[i];
    }
    
    checksum += addr_sum;
    checksum += local_bfs.a + local_bfs.b + local_bfs.c + local_bfs.d;
    checksum += big_var;
    
    printf("Checksum: %u\n", checksum);
    
    /* 8. Final inline assembly to ensure resource tracking */
    asm volatile(
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        : 
        : "r" (checksum), "r" (iterations)
        : "r0", "r1", "memory"
    );
    
    return (checksum > 0) ? 0 : 1;
}
