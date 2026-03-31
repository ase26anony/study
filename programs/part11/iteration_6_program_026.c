/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c this_file.c */

/* Global volatile structure with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
    unsigned int e : 3;
    unsigned int f : 5;
} g_bfs;

/* Non-inline function to prevent optimization of bit-field operations */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to create ZERO_EXTRACT patterns */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
    s->e = (x ^ y) & 0x7;
    s->f = (x + y) & 0x1F;
    
    /* Additional volatile access to prevent reordering */
    volatile int dummy = s->a;
    (void)dummy;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* SUBREG patterns from mixed-width assignments */
        shorts[i] = ints[i] & 0xFFFF;           /* 32-bit to 16-bit: potential SUBREG */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* 32-bit to 8-bit: potential SUBREG */
        
        /* Mixed-width arithmetic */
        int temp = shorts[i] * chars[i];        /* Promotions and subregs */
        ints[(i + 1) % n] = temp + (int)chars[i];
    }
}

/* Complex addressing mode generator */
__attribute__((noinline))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    int sum = 0;
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex addressing with multiple uses */
    sum += arr[i][j];
    sum += arr[j][i];
    sum += arr[i][i] & 0xFF;      /* Bitwise op that might become ZERO_EXTRACT */
    sum += arr[j][j] | 0xFFFF00;
    
    /* More complex nested addressing */
    sum += arr[arr[i][j] % 100][arr[j][i] % 100];
    
    return sum;
}

/* Function with register pressure */
__attribute__((optimize("O0")))  /* Minimal optimization to preserve complex RTL */
void high_register_pressure(void) {
    /* Many local variables to force register spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed operations creating SUBREG patterns */
    s1 = v1; s2 = v2; s3 = v3; s4 = v4; s5 = v5;
    c1 = v1; c2 = v2; c3 = v3; c4 = v4; c5 = v5;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* More mixed-width operations */
    v1 = s1 + c1;
    v2 = s2 * c2;
    v3 = s3 - c3;
    v4 = s4 / (c4 ? c4 : 1);
    v5 = s5 % (c5 ? c5 : 1);
    
    /* Use all variables to prevent elimination */
    g_bfs.a = (v1 + v2 + v3 + v4 + v5) & 0xF;
}

int main(int argc, char *argv[]) {
    /* Force argc to be used to prevent optimization */
    volatile int arg = argc;
    
    /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, arg, arg * 2);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_arr[100];
    volatile char char_arr[100];
    int int_arr[100];
    
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * arg;
    }
    
    mixed_width_ops(short_arr, char_arr, int_arr, 100);
    
    /* 3. Complex addressing modes */
    int matrix[100][100];
    volatile int idx1 = arg, idx2 = arg * 3;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    int addr_sum = complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. High register pressure section */
    high_register_pressure();
    
    /* 5. Additional volatile bit-field with complex expression */
    struct BitFieldStruct local_bfs;
    volatile struct BitFieldStruct *p_bfs = &local_bfs;
    
    /* Complex bit-field assignment chain */
    int x = arg;
    for (int i = 0; i < 10; i++) {
        p_bfs->a = (x >> i) & 0xF;
        p_bfs->b = (x << i) & 0xFF;
        p_bfs->c = (x + i) & 0xFFF;
        x = p_bfs->a + p_bfs->b + p_bfs->c;
    }
    
    /* 6. Final computation to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c;
    checksum += short_arr[arg % 100];
    checksum += char_arr[arg % 100];
    checksum += int_arr[arg % 100];
    checksum += addr_sum;
    checksum += local_bfs.a + local_bfs.b + local_bfs.c;
    
    /* Use checksum so it can't be optimized away */
    if (checksum > 0) {
        return checksum % 256;
    }
    
    return 0;
}
