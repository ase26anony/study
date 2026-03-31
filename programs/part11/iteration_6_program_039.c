/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
} g_bfs;

/* Non-inline function to force memory addressing modes */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = 1;
    s->b = 2;
    s->c = 3;
    
    /* Additional mixed assignments to increase pattern variety */
    s->a = s->b & 0x3;  /* Bit-field to bit-field with mask */
}

/* Another noinline function to create SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;  /* Truncation to 16-bit */
        ints[i] = chars[i] * 2;        /* Char to int promotion */
        
        /* Complex expression with SUBREG possibilities */
        shorts[i] = (shorts[i] + (ints[i] >> 8)) & 0xFF;
    }
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access that may generate ZERO_EXTRACT */
    return arr[i][j] & 0x3FF;  /* Mask operation on memory access */
}

int main(int argc, char **argv) {
    /* Force argc to be used to prevent optimization */
    if (argc < 2) return 1;
    
    /* 1. Bit-field operations on global volatile struct */
    modify_bitfields(&g_bfs);
    
    /* 2. Mixed-width operations with local volatile arrays */
    volatile short short_arr[100];
    volatile int int_arr[100];
    volatile char char_arr[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * argc;
        char_arr[i] = i & 0x7F;
    }
    
    /* Cast away volatile for the function call */
    mixed_width_ops((short *)short_arr, (int *)int_arr, (char *)char_arr, 100);
    
    /* 3. Complex 2D array access with volatile indices */
    int matrix[100][100];
    volatile int idx1 = argc * 3;
    volatile int idx2 = argc * 7;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    int result = complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. Additional mixed operations to increase register pressure */
    volatile int temp = 0;
    for (int i = 0; i < 50; i++) {
        /* Operations that may generate SUBREG in SET_DEST */
        volatile short s = int_arr[i];
        volatile char c = int_arr[i] >> 16;
        
        /* Complex expression mixing types */
        temp += s * c + (int_arr[i] & 0xFF);
        
        /* Bit-field operation on local struct */
        struct {
            unsigned int x : 10;
            unsigned int y : 22;
        } local_bf;
        
        local_bf.x = i & 0x3FF;
        local_bf.y = int_arr[i];
        temp += local_bf.x;
    }
    
    /* 5. Inline assembly to clobber registers and increase pressure */
    asm volatile ("" 
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory");
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c;
    checksum += result;
    checksum += temp;
    
    for (int i = 0; i < 100; i++) {
        checksum += short_arr[i];
        checksum += int_arr[i];
        checksum += char_arr[i];
    }
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
