/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload coverage.c -o coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))

/* Global volatile structure with bit-fields */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
} g_bfs;

/* NOINLINE function to force memory addressing modes */
NOINLINE void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* Likely generates ZERO_EXTRACT for 4-bit field */
    s->b = 0xFF;   /* 8-bit field assignment */
    s->c = 0x12345; /* 20-bit field assignment */
    
    /* Additional mixed assignments to increase complexity */
    s->a = s->b & 0x0F;  /* Bit-field operation */
}

/* Another NOINLINE function for SUBREG patterns */
NOINLINE void mixed_width_ops(short *shorts, int *ints, char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that generate SUBREG RTL */
        shorts[i] = ints[i] & 0xFFFF;           /* Truncation to 16-bit */
        ints[i] = chars[i] * 2;                 /* Char to int promotion */
        chars[i] = (shorts[i] >> 8) & 0xFF;     /* Mixed-width operation */
    }
}

/* Function with complex addressing modes */
NOINLINE int complex_addressing(int arr[100][100], int idx1, int idx2) {
    /* Volatile indices to prevent constant propagation */
    volatile int vi = idx1;
    volatile int vj = idx2;
    
    /* Complex memory access with bitwise operation */
    int val = arr[vi % 100][vj % 100];
    return val & 0x3FF;  /* ZERO_EXTRACT-like operation */
}

/* Function with high register pressure */
NOINLINE void register_pressure(int iterations) {
    /* Many local variables to force register spilling */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed-width operations generating SUBREG */
        s1 = (v1 >> 16) & 0xFFFF;
        v2 = c1 * s1;
        s2 = v2 & 0xFF;
        
        /* Inline assembly to clobber registers and force reload */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
        
        /* More operations to maintain pressure */
        v3 = v4 + v5;
        s3 = v3;
        c2 = s3 >> 8;
    }
}

int main(int argc, char *argv[]) {
    int i, j;
    
    /* 1. Bit-field operations on global volatile struct */
    modify_bitfields((struct BitFieldStruct *)&g_bfs);
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_arr[50];
    volatile int int_arr[50];
    volatile char char_arr[50];
    
    /* Initialize with non-constant values */
    for (i = 0; i < 50; i++) {
        int_arr[i] = argc + i * 3;
        char_arr[i] = (argc + i) & 0xFF;
    }
    
    mixed_width_ops((short *)short_arr, (int *)int_arr, (char *)char_arr, 
                    argc % 40 + 10);
    
    /* 3. Complex array addressing with 2D array */
    int matrix[100][100];
    
    /* Initialize matrix */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Use volatile indices for complex addressing */
    volatile int idx_i = argc * 7;
    volatile int idx_j = argc * 13;
    
    int result = complex_addressing(matrix, idx_i, idx_j);
    
    /* 4. Create register pressure */
    register_pressure(argc % 10 + 5);
    
    /* 5. Additional bit-field operations combined with memory access */
    {
        volatile struct BitFieldStruct local_bfs;
        volatile int *ptr = (volatile int *)&local_bfs;
        
        /* Combined operation: bit-field store after memory load */
        local_bfs.a = (*ptr >> 4) & 0x0F;
        local_bfs.b = result & 0xFF;
        
        /* Force memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    
    checksum ^= g_bfs.a;
    checksum ^= g_bfs.b << 4;
    checksum ^= g_bfs.c << 12;
    
    for (i = 0; i < 50; i++) {
        checksum ^= short_arr[i];
        checksum ^= int_arr[i] << 16;
        checksum ^= char_arr[i] << 24;
    }
    
    checksum ^= result;
    
    /* Use checksum to affect program output */
    printf("Checksum: 0x%08X\n", checksum);
    
    return checksum & 0xFF;
}
