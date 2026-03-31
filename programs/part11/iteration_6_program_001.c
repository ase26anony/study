/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c this_file.c */

/* Global volatile structure with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 17;
} g_bfs;

/* Non-inline function to prevent optimization of bit-field operations */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to create SET_DEST with ZERO_EXTRACT */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    s->d = (y >> 20) & 0x7;
    s->e = (x ^ y) & 0x1FFFF;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(short *shorts, char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* Operations that create SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* int -> char truncation */
        /* Sign extension creating SUBREG patterns */
        ints[i] = (int)shorts[i] + (int)chars[i] * 256;
    }
}

/* Function with complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[][100], int idx1, int idx2) {
    /* Complex addressing with bit-field-like extraction */
    int val = arr[idx1][idx2];
    /* Create ZERO_EXTRACT pattern through bitwise operations */
    return (val & 0xFF) | ((val >> 8) & 0xFF00);
}

int main(int argc, char **argv) {
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields(&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations creating SUBREG patterns */
    volatile short short_arr[100];
    volatile char char_arr[100];
    volatile int int_arr[100];
    
    /* Initialize with argc-dependent values */
    for (int i = 0; i < argc % 50 + 10; i++) {
        int_arr[i] = argc + i * 3;
    }
    
    mixed_width_ops((short *)short_arr, (char *)char_arr, (int *)int_arr, 
                    argc % 50 + 10);
    
    /* 3. Complex 2D array access with volatile indices */
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 3) % 50;
    int matrix[100][100];
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Complex access pattern that may generate ZERO_EXTRACT */
    int matrix_val = complex_addressing(matrix, idx_i, idx_j);
    
    /* 4. Additional mixed operations to increase register pressure */
    volatile int temp = 0;
    for (int i = 0; i < argc % 20 + 5; i++) {
        /* Mix types to create SUBREG patterns */
        char c = (char)(int_arr[i] & 0xFF);
        short s = (short)(int_arr[i] >> 8);
        int x = (int)c + (int)s * 2;
        
        /* Bit-field like operation on memory */
        temp = (temp & ~0xFF) | (x & 0xFF);
        
        /* STRICT_LOW_PART pattern through pointer casting */
        *(volatile char *)(&int_arr[i]) = (char)x;
    }
    
    /* 5. Inline assembly to clobber registers and force resource tracking */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 6. Compute checksum to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c;
    for (int i = 0; i < argc % 50 + 10; i++) {
        checksum += short_arr[i] + char_arr[i] + int_arr[i];
    }
    checksum += matrix_val + temp;
    
    /* Use checksum to prevent optimization */
    volatile int result = checksum;
    
    return result % 256;
}

/* Additional function to create more opportunities for resource tracking */
__attribute__((noinline, optimize("O0")))
void extra_pressure() {
    /* Many local variables to force register spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed operations creating SUBREG patterns */
    s1 = v1 & 0xFFFF;
    c1 = v1 >> 16;
    s2 = v2 & 0xFFFF;
    c2 = v2 >> 16;
    
    /* Bit-field operations on local struct */
    struct {
        unsigned int f1 : 5;
        unsigned int f2 : 11;
        unsigned int f3 : 16;
    } local_bf;
    
    local_bf.f1 = v1 & 0x1F;
    local_bf.f2 = v2 & 0x7FF;
    local_bf.f3 = v3 & 0xFFFF;
    
    /* Complex memory addressing */
    int *ptr = &v1;
    for (int i = 0; i < 5; i++) {
        *(volatile char *)(ptr + i) = (char)(v1 + i);
    }
}
