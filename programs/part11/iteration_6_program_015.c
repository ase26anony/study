/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
} g_bitfield = {0};

/* Non-inline function to force memory addressing modes */
__attribute__((noinline)) 
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
}

/* Another noinline function for complex addressing */
__attribute__((noinline))
int complex_array_access(int (*arr)[100], int i, int j, int mask) {
    /* Complex addressing with bitwise operation */
    return arr[i][j] & mask;
}

int main(int argc, char *argv[]) {
    /* Force argc to be used to prevent optimization */
    if (argc < 2) return 1;
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) iterations = 10;
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct *)&g_bitfield, argc, iterations);
    
    /* 2. Mixed-width operations for SUBREG generation */
    volatile short short_array[100];
    volatile char char_array[100];
    volatile int int_array[100];
    
    /* Loop with mixed-type operations */
    for (int i = 0; i < iterations && i < 100; i++) {
        /* SUBREG patterns: mixing different width operations */
        int val = argc + i * 3;
        
        /* int -> short (truncation, may generate SUBREG) */
        short_array[i] = (short)(val & 0xFFFF);
        
        /* char -> int (extension, may generate SUBREG) */
        char_array[i] = (char)(val & 0xFF);
        int_array[i] = (int)char_array[i] * 2;  /* Load char, extend, multiply */
        
        /* More mixed-width operations */
        int_array[i] += short_array[i] * 3;  /* short promoted to int */
    }
    
    /* 3. Complex 2D array access with volatile indices */
    int arr_2d[100][100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Volatile indices to prevent constant propagation */
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 3) % 50;
    
    /* Complex addressing mode */
    int result = complex_array_access(arr_2d, idx_i, idx_j, 0xFF);
    
    /* 4. More bit-field operations combined with memory access */
    struct BitFieldStruct local_bitfield;
    
    /* Take address and modify */
    struct BitFieldStruct *ptr = &local_bitfield;
    ptr->a = result & 0xF;
    ptr->b = (result >> 4) & 0xFF;
    
    /* 5. Inline assembly to increase register pressure and force reload */
    asm volatile(
        "# Force register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        : 
        : "r" (argc), "r" (iterations)
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* 6. Additional mixed operations to create more RTL patterns */
    volatile int mixed_ops = 0;
    for (int i = 0; i < iterations && i < 50; i++) {
        /* Create SUBREG patterns through type punning */
        union {
            int full;
            struct {
                short low;
                short high;
            } parts;
        } converter;
        
        converter.full = argc * i;
        /* Access partial registers */
        short_array[i] = converter.parts.low + converter.parts.high;
        
        /* More bit-field like operations using bitwise */
        mixed_ops = (mixed_ops << 3) | (argc & 0x7);
        
        /* Memory access with complex addressing */
        int_array[mixed_ops % 50] = arr_2d[i % 50][(i * 7) % 50];
    }
    
    /* 7. Final computation to prevent dead code elimination */
    int checksum = g_bitfield.a + g_bitfield.b + g_bitfield.c + g_bitfield.d;
    for (int i = 0; i < iterations && i < 100; i++) {
        checksum += short_array[i];
        checksum += int_array[i];
    }
    checksum += result;
    checksum += mixed_ops;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
