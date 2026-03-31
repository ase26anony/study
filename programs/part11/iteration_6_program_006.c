/* Compile with: gcc -O2 -fdump-rtl-reload -fno-strict-aliasing coverage.c -o coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bitfield = {0};

/* Non-inline function to force memory addressing modes */
__attribute__((noinline)) 
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 8) & 0xFF;
    
    /* Additional assignment to ensure multiple patterns */
    s->a = (s->b + s->c) & 0xF;
}

/* Another noinline function to create SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns by mixing widths */
        shorts[i] = (i * 37) & 0xFFFF;           /* int to short -> SUBREG */
        chars[i] = (shorts[i] + i) & 0xFF;       /* short to char -> SUBREG */
        
        /* Complex expression with SUBREG destination */
        shorts[i] = (chars[i] * 3) + (shorts[i] >> 2);
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[100][100], int idx1, int idx2) {
    volatile int vi = idx1;
    volatile int vj = idx2;
    
    /* Complex memory access with non-constant indices */
    int val = arr[vi % 100][vj % 100];
    
    /* Combine with bitwise operation that may generate ZERO_EXTRACT */
    return val & 0x3FF;  /* 10-bit mask */
}

int main(int argc, char *argv[]) {
    /* Force register pressure with many local variables */
    volatile int v1 = argc;
    volatile short v2 = argc * 2;
    volatile char v3 = argc * 3;
    volatile int v4 = argc + 1;
    volatile short v5 = argc + 2;
    volatile char v6 = argc + 3;
    volatile int v7 = argc * 4;
    volatile short v8 = argc * 5;
    volatile char v9 = argc * 6;
    
    /* 1. Trigger bit-field assignments for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct*)&g_bitfield, argc, argc * 17);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_array[50];
    volatile char char_array[50];
    mixed_width_operations(short_array, char_array, argc % 50);
    
    /* 3. Complex addressing with 2D array */
    int arr_2d[100][100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Access with volatile indices */
    volatile int idx_i = argc;
    volatile int idx_j = argc * 7;
    int addr_result = complex_addressing(arr_2d, idx_i, idx_j);
    
    /* 4. Additional mixed-width operations in main */
    /* int to short assignment generating SUBREG */
    v2 = v1 & 0x7FFF;
    
    /* short to char assignment */
    v3 = v2 & 0x7F;
    
    /* char to int with sign extension */
    v4 = v3 * 2;
    
    /* 5. More bit-field manipulation */
    g_bitfield.b = v4 & 0xFF;
    g_bitfield.c = (v4 >> 8) & 0xFFF;
    
    /* 6. Inline assembly to clobber registers and increase pressure */
    asm volatile ("" 
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory");
    
    /* 7. Additional complex pattern: memory access through pointer with offset */
    volatile int *ptr = (volatile int*)&g_bitfield;
    for (int i = 0; i < 4; i++) {
        /* Generate MEM with complex address */
        int val = ptr[i];
        
        /* Use in expression that might create SUBREG */
        short temp = val & 0xFFFF;
        ptr[i] = (temp * 3) & 0xFF;
    }
    
    /* 8. Create a ZERO_EXTRACT pattern from memory */
    volatile unsigned int mem_word = 0x12345678;
    /* This should generate ZERO_EXTRACT when assigning to bit-field */
    g_bitfield.a = (mem_word >> 4) & 0xF;
    g_bitfield.b = (mem_word >> 8) & 0xFF;
    
    /* 9. STRICT_LOW_PART pattern through pointer casting */
    volatile int *int_ptr = (volatile int*)short_array;
    /* Cast and assign partial word */
    short_array[0] = (*int_ptr) & 0xFFFF;
    
    /* 10. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bitfield.a;
    checksum += g_bitfield.b;
    checksum += g_bitfield.c;
    checksum += g_bitfield.d;
    
    for (int i = 0; i < (argc % 50); i++) {
        checksum += short_array[i];
        checksum += char_array[i];
    }
    
    checksum += addr_result;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    
    printf("Checksum: %u\n", checksum);
    
    return (checksum > 1000) ? 0 : 1;
}
