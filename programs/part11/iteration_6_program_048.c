/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Prevent function inlining to force memory operations */
#define NOINLINE __attribute__((noinline))

/* Global volatile structure with bit-fields */
volatile struct BitFieldStruct {
    int a:4;
    int b:8;
    int c:20;
    int padding:32;
} g_bfs = {0};

/* NOINLINE function to modify bit-fields */
NOINLINE void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;           /* Likely generates ZERO_EXTRACT for 4-bit field */
    s->b = 0xFF;        /* 8-bit field assignment */
    s->c = 0x12345;     /* 20-bit field assignment */
    s->padding = 0xDEADBEEF;
}

/* Another NOINLINE function for mixed-width operations */
NOINLINE void mixed_width_ops(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that may generate SUBREG patterns */
        int temp = (int)chars[i];      /* Load char, sign-extend to int */
        temp = temp * 2 + i;           /* Arithmetic mixing */
        shorts[i] = (short)(temp & 0xFFFF);  /* Truncate to short - may generate SUBREG */
        
        /* More complex pattern: char -> int -> short chain */
        chars[i] = (char)((shorts[i] >> 4) & 0xFF);
    }
}

/* Function with complex array addressing */
NOINLINE int complex_addressing(int idx1, int idx2) {
    static int arr[100][100];
    volatile int *volatile_ptr = (volatile int *)arr;
    
    /* Complex addressing with bitwise operation */
    int val = arr[idx1][idx2];
    /* Bitwise operation that might be represented as ZERO_EXTRACT */
    val = (val & 0xFF00FF00) | 0x00FF00FF;
    arr[idx1][idx2] = val;
    
    return val;
}

int main(int argc, char **argv) {
    int i, j;
    
    /* 1. Bit-field operations on global volatile struct */
    modify_bitfields((struct BitFieldStruct *)&g_bfs);
    
    /* 2. Mixed-width local arrays */
    volatile short short_arr[100];
    volatile char char_arr[100];
    
    /* Initialize with non-constant values */
    for (i = 0; i < 100; i++) {
        short_arr[i] = (short)(i * 3);
        char_arr[i] = (char)(i * 5);
    }
    
    /* Perform mixed-width operations */
    mixed_width_ops(short_arr, char_arr, argc > 1 ? atoi(argv[1]) % 50 + 50 : 50);
    
    /* 3. Complex array addressing with volatile indices */
    volatile int idx_i = argc > 2 ? atoi(argv[2]) % 50 : 10;
    volatile int idx_j = argc > 3 ? atoi(argv[3]) % 50 : 20;
    
    int arr_result = complex_addressing(idx_i, idx_j);
    
    /* 4. Additional mixed-type operations to increase SUBREG generation */
    volatile int int_var = 0x12345678;
    volatile short short_var;
    volatile char char_var;
    
    /* Multiple width conversions */
    short_var = (short)int_var;                    /* int -> short, may generate SUBREG */
    char_var = (char)(short_var >> 4);             /* short -> char */
    int_var = (int)char_var * 256 + (int)short_var; /* Mixed-width arithmetic */
    
    /* 5. Inline assembly to increase register pressure */
    asm volatile ("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 6. More bit-field manipulation with address taken */
    struct BitFieldStruct local_bfs;
    volatile struct BitFieldStruct *bfs_ptr = &local_bfs;
    
    bfs_ptr->a = 0xF;
    bfs_ptr->b = 0xAA;
    bfs_ptr->c = 0xCCCCC;
    
    /* 7. Complex expression combining multiple patterns */
    int complex_result = 0;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            /* Mix array access with bit-field like operation */
            complex_result += (arr_result >> (i * 4)) & 0xF;
            complex_result += (int_var >> (j * 2)) & 0x3;
            
            /* Additional SUBREG generation through type punning */
            union {
                int full;
                struct {
                    short low;
                    short high;
                } parts;
            } u;
            
            u.full = complex_result;
            u.parts.low = (short)(u.parts.high + i);  /* May generate SUBREG as SET_DEST */
            complex_result = u.full;
        }
    }
    
    /* 8. Final computation to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c 
                   + short_arr[0] + char_arr[0] 
                   + arr_result + int_var 
                   + complex_result;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
