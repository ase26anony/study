/* Target: resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs;

/* Non-inline function to force memory addressing modes */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s) {
    /* Multiple bit-field assignments - should generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* Likely ZERO_EXTRACT for 4-bit field */
    s->b = 0x7F;   /* 8-bit field */
    s->c = 0xFFF;  /* 12-bit field */
    s->d = 0x55;   /* Another 8-bit field */
    
    /* Mixed assignments to same location */
    s->a = s->b & 0x3;  /* Complex expression with bit-field destination */
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(short *shorts, int *ints, char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* SUBREG patterns: mixing different widths */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (shorts[i] >> 8) & 0xFF;     /* short -> char */
        ints[i] = chars[i] * 2;                 /* char -> int promotion */
        
        /* More complex SUBREG usage */
        volatile short vshort = shorts[i];
        volatile int vint = vshort + chars[i];  /* short promoted to int */
        shorts[i] = vint;                       /* int -> short with possible SUBREG */
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access with bitwise operation */
    int val = arr[i][j];
    
    /* Operation that might generate ZERO_EXTRACT on memory */
    val = (val & 0xFF00FF00) | 0x00FF00FF;
    
    /* Store back with possible complex RTL */
    arr[i][j] = val;
    
    return val;
}

int main(int argc, char *argv[]) {
    /* Force register pressure by using many variables */
    register int r0 asm("r0");
    register int r1 asm("r1");
    register int r2 asm("r2");
    register int r3 asm("r3");
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields(&g_bfs);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_arr[100];
    volatile int int_arr[100];
    volatile char char_arr[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = argc + i * 3;
    }
    
    mixed_width_operations((short *)short_arr, (int *)int_arr, (char *)char_arr, 
                          argc % 50 + 10);
    
    /* 3. Complex addressing with 2D array */
    int matrix[100][100];
    volatile int idx1 = argc * 7;
    volatile int idx2 = argc * 13;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j + argc;
        }
    }
    
    int addr_result = complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. More mixed-width operations in main */
    volatile int wide_int = 0x12345678;
    volatile short narrow_short;
    volatile char narrow_char;
    
    /* Generate SUBREG patterns */
    narrow_short = wide_int;                    /* 32-bit to 16-bit */
    narrow_char = narrow_short >> 8;            /* 16-bit to 8-bit */
    wide_int = narrow_char * 0x101;             /* 8-bit to 32-bit with scaling */
    
    /* 5. Inline assembly to clobber registers and increase pressure */
    asm volatile (
        "mov %0, #1\n\t"
        "mov %1, #2\n\t"
        "mov %2, #3\n\t"
        "mov %3, #4\n\t"
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3)
        :
        : /* No additional clobbers needed since we're using specific registers */
    );
    
    /* Additional assembly to clobber more resources */
    asm volatile("" ::: "memory", "cc");
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    
    for (int i = 0; i < 50; i++) {
        checksum += short_arr[i];
        checksum += int_arr[i] & 0xFFFF;
        checksum += char_arr[i];
    }
    
    checksum += addr_result;
    checksum += wide_int;
    checksum += narrow_short;
    checksum += narrow_char;
    
    /* Use all register variables */
    checksum += r0 + r1 + r2 + r3;
    
    /* Complex expression that might generate interesting RTL */
    volatile int final_result = 
        (checksum & 0xF) | 
        ((checksum >> 4) & 0xF0) | 
        ((checksum >> 8) & 0xF00) |
        ((checksum >> 12) & 0xF000);
    
    /* Additional bit-field store to volatile */
    g_bfs.a = final_result & 0xF;
    g_bfs.b = (final_result >> 4) & 0xFF;
    
    /* Print to prevent optimization */
    printf("Result: %u (argc=%d)\n", final_result, argc);
    
    return final_result > 1000 ? 0 : 1;
}

/* Additional global to increase complexity */
volatile struct NestedBitFields {
    struct {
        unsigned int x : 3;
        unsigned int y : 5;
    } inner;
    unsigned int z : 24;
} g_nested;

/* Function called once to generate more patterns */
void __attribute__((constructor)) init_patterns(void) {
    g_nested.inner.x = 3;
    g_nested.inner.y = 7;
    g_nested.z = 0xFFFFFF;
    
    /* Force a memory-to-memory bit-field operation */
    volatile struct NestedBitFields local;
    local = g_nested;
    local.inner.x = local.inner.y & 0x3;
    g_nested = local;
}
