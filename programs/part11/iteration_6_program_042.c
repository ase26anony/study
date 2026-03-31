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

/* Force memory addressing modes with non-inline function */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET with ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 8) & 0xFF;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        int temp = shorts[i];
        temp = temp + (int)chars[i];  /* char to int promotion */
        shorts[i] = (short)(temp & 0xFFFF);  /* int to short truncation - SUBREG */
    }
}

/* Complex addressing mode generator */
__attribute__((noinline))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    /* Force complex XEXP addressing */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Non-constant indices from volatile variables */
            int idx_i = (*idx1 + i) % 100;
            int idx_j = (*idx2 + j) % 100;
            
            /* Complex memory access that may combine with bit operations */
            int val = arr[idx_i][idx_j];
            
            /* Bit operation that could generate ZERO_EXTRACT */
            val = val & 0x3FF;  /* Keep only lower 10 bits */
            
            /* Combine with bit-field like operation */
            val = (val << 4) | (val & 0xF);
            
            sum += val;
        }
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations for SUBREG generation */
    volatile short short_array[100];
    volatile char char_array[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        short_array[i] = (short)(i * argc);
        char_array[i] = (char)(i + argc);
    }
    
    mixed_width_ops(short_array, char_array, argc % 50 + 10);
    
    /* 3. Complex array addressing with volatile indices */
    int arr[100][100];
    volatile int idx1 = argc;
    volatile int idx2 = argc * 3;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    int complex_sum = complex_addressing(arr, &idx1, &idx2);
    
    /* 4. More bit-field manipulation with pointer casting */
    /* This may generate MEM with complex addressing */
    unsigned int *ptr = (unsigned int*)&g_bfs;
    *ptr = (*ptr & 0xFFFF0000) | (argc & 0xFFFF);
    
    /* 5. Additional SUBREG patterns through type punning */
    struct {
        volatile int full;
        volatile short half[2];
    } pun;
    
    pun.full = argc * 1000;
    pun.half[0] = (short)(pun.full & 0xFFFF);  /* SUBREG store */
    pun.half[1] = (short)((pun.full >> 16) & 0xFFFF);  /* Another SUBREG */
    
    /* 6. Inline assembly to increase register pressure and force reload */
    asm volatile("" ::: "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    
    /* 7. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    
    for (int i = 0; i < 10; i++) {
        checksum += short_array[i];
        checksum += char_array[i];
    }
    
    checksum += complex_sum;
    checksum += pun.full;
    checksum += pun.half[0] + pun.half[1];
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
