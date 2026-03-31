/* Target: resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -c coverage.c */

#include <stdio.h>
#include <stdlib.h>

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

/* Global volatile structure with bit-fields */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 17;
} g_bfs = {0};

/* Helper function to modify bit-fields - forces ZERO_EXTRACT/STRICT_LOW_PART */
NOINLINE void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    s->d = (y >> 20) & 0x7;
    s->e = (x ^ y) & 0x1FFFF;
}

/* Another helper with mixed-width operations */
NOINLINE void mixed_width_ops(volatile short *arr, int count) {
    for (int i = 0; i < count; i++) {
        /* SUBREG patterns from mixed-width operations */
        volatile int temp = i * 256;
        /* Casting int to short may generate SUBREG */
        arr[i] = (short)(temp + i);
        
        /* More complex expression with SUBREG */
        volatile char c = (char)(temp & 0xFF);
        arr[i] += (short)c;  /* char to short to int conversions */
    }
}

/* Complex addressing with 2D array */
NOINLINE int complex_addressing(int idx1, int idx2) {
    static int arr[100][100];
    volatile int vi = idx1;
    volatile int vj = idx2;
    
    /* Complex memory addressing that may generate MEM with XEXP */
    int val = arr[vi % 100][vj % 100];
    
    /* Bitwise operation that could be ZERO_EXTRACT */
    return (val & 0xFF00) >> 8;
}

/* Function with register pressure */
NOINLINE void create_register_pressure(void) {
    /* Many local variables to force register allocation */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed operations causing SUBREG patterns */
    s1 = (short)v1;
    s2 = (short)(v2 + v3);
    s3 = (short)(v4 * v5);
    s4 = (short)(v6 ^ v7);
    s5 = (short)(v8 | v9);
    
    /* More conversions */
    c1 = (char)v1;
    c2 = (char)s2;
    c3 = (char)(v3 + v4);
    c4 = (char)(v5 * v6);
    c5 = (char)(v7 ^ v8);
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* Use variables to prevent elimination */
    v10 += s1 + s2 + s3 + s4 + s5;
    v10 += c1 + c2 + c3 + c4 + c5;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_arr[50];
    mixed_width_ops(short_arr, argc % 50);
    
    /* 3. Complex addressing modes */
    result += complex_addressing(argc, argc + 1);
    
    /* 4. Create register pressure */
    create_register_pressure();
    
    /* 5. More bit-field operations with volatile */
    volatile struct BitFieldStruct local_bfs;
    local_bfs.a = argc & 0xF;
    local_bfs.b = (argc >> 4) & 0xFF;
    local_bfs.c = (argc * 3) & 0xFFFFF;
    local_bfs.d = (argc >> 2) & 0x7;
    local_bfs.e = (argc ^ (argc >> 1)) & 0x1FFFF;
    
    /* 6. Additional SUBREG patterns through pointer casting */
    volatile int *int_ptr = &argc;
    volatile short *short_ptr = (volatile short *)int_ptr;
    volatile char *char_ptr = (volatile char *)int_ptr;
    
    *short_ptr = (short)(*int_ptr + 1);      /* int to short store */
    *char_ptr = (char)(*int_ptr + 2);        /* int to char store */
    
    /* 7. Nested complex expressions */
    {
        volatile int arr2d[10][10];
        volatile int i = argc % 10;
        volatile int j = (argc + 1) % 10;
        
        /* Complex memory access pattern */
        arr2d[i][j] = (*int_ptr + *short_ptr + *char_ptr);
        
        /* Bit-field extraction from memory */
        result += (arr2d[i][j] & 0x0F00) >> 8;  /* Potential ZERO_EXTRACT */
    }
    
    /* 8. Final register pressure with inline assembly */
    asm volatile("" : : : "r6", "r7", "r8", "r9", "r10", "memory");
    
    /* Compute checksum to prevent dead code elimination */
    result += g_bfs.a + g_bfs.b + g_bfs.c;
    result += local_bfs.d + local_bfs.e;
    
    for (int i = 0; i < (argc % 50); i++) {
        result += short_arr[i];
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
