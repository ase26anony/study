/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */

/* Global volatile structure with bit-fields */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 1;
    unsigned int e : 3;
    unsigned int f : 16;
} g_bfs;

/* 2D array for complex addressing */
static int arr[100][100];

/* Non-inline function to force memory accesses */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    s->d = (x ^ y) & 1;
    s->e = (y >> 3) & 0x7;
    s->f = (x + y) & 0xFFFF;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations generating SUBREG patterns */
        int temp = chars[i];          /* Load char, sign-extend to int */
        shorts[i] = temp & 0x7FFF;    /* Store as short (SUBREG destination) */
        
        /* More complex expression with SUBREG */
        int wide = temp * 3;
        shorts[i] = (wide >> 1) & 0x7FFF;  /* Another SUBREG store */
    }
}

/* Function with complex addressing modes */
__attribute__((noinline))
int complex_addressing(int i, int j, int mask) {
    /* Complex array access with bitwise operation */
    int val = arr[i][j];
    /* Bitwise operation that might generate ZERO_EXTRACT */
    return (val & mask) | ((val >> 16) & 0xFFFF);
}

/* Function to increase register pressure */
__attribute__((noinline, optimize("O0")))
void force_register_pressure(int iterations) {
    volatile int v1, v2, v3, v4, v5, v6, v7, v8;
    volatile short s1, s2, s3, s4;
    volatile char c1, c2, c3, c4;
    
    for (int i = 0; i < iterations; i++) {
        /* Many local variables to force spilling */
        v1 = i * 2;
        v2 = v1 + 1;
        v3 = v2 * 3;
        v4 = v3 - v1;
        v5 = v4 & 0xFF;
        v6 = v5 | 0x100;
        v7 = v6 << 2;
        v8 = v7 >> 1;
        
        /* Mixed-width operations */
        s1 = v1 & 0xFFFF;
        s2 = v2 & 0xFFFF;
        s3 = v3 & 0xFFFF;
        s4 = v4 & 0xFFFF;
        
        c1 = v5 & 0xFF;
        c2 = v6 & 0xFF;
        c3 = v7 & 0xFF;
        c4 = v8 & 0xFF;
        
        /* Inline assembly to clobber registers and force reload */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    }
}

int main(int argc, char **argv) {
    /* Use argc to prevent constant propagation */
    int iterations = argc > 1 ? 10 : 5;
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_arr[50];
    volatile char char_arr[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 50; i++) {
        char_arr[i] = (i * 3) & 0xFF;
    }
    
    mixed_width_ops(short_arr, char_arr, iterations * 3);
    
    /* 3. Complex array addressing with volatile indices */
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 7) % 50;
    
    int result = complex_addressing(idx_i, idx_j, 0x00FF00FF);
    
    /* 4. Force register pressure and resource tracking */
    force_register_pressure(iterations);
    
    /* 5. More bit-field operations combined with array access */
    g_bfs.a = result & 0xF;
    g_bfs.b = (result >> 8) & 0xFF;
    
    /* Complex expression combining bit-fields and memory */
    int temp = arr[idx_i][idx_j];
    g_bfs.f = (temp + result) & 0xFFFF;
    
    /* 6. Additional SUBREG patterns through type punning */
    {
        volatile int wide = argc * 1000;
        volatile short narrow;
        
        /* Direct assignment generating SUBREG */
        narrow = wide;  /* int to short conversion */
        
        /* Pointer casting for SUBREG patterns */
        volatile char *byte_ptr = (volatile char*)&wide;
        for (int i = 0; i < 4; i++) {
            char_arr[i] = byte_ptr[i];  /* Memory access with different widths */
        }
    }
    
    /* 7. Nested bit-field operations in loop */
    for (int i = 0; i < iterations; i++) {
        /* Multiple bit-field assignments in loop */
        g_bfs.c = (g_bfs.c + i) & 0xFFFFF;
        g_bfs.e = (g_bfs.e ^ i) & 0x7;
        
        /* Mixed with array access */
        short_arr[i % 50] = (g_bfs.c >> 4) & 0x7FFF;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    checksum += g_bfs.a;
    checksum += g_bfs.b;
    checksum += g_bfs.c & 0xFF;
    checksum += g_bfs.d;
    checksum += g_bfs.e;
    checksum += g_bfs.f;
    
    for (int i = 0; i < 50; i++) {
        checksum += short_arr[i];
        checksum += char_arr[i];
    }
    
    checksum += result;
    
    /* Use checksum to prevent optimization */
    if (checksum > 1000000) {
        return 1;
    }
    
    return 0;
}
