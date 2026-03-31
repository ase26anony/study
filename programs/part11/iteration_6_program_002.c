/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
    unsigned int e : 24;
} g_bfs;

/* Non-inline function to force memory addressing */
__attribute__((noinline)) 
void modify_bitfields(volatile struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
    s->e = (x + y) & 0xFFFFFF;
}

/* Another noinline function for SUBREG operations */
__attribute__((noinline))
void mixed_width_ops(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        ints[i] = chars[i] * 2;                 /* char -> int promotion */
        /* Complex expression with SUBREG possibilities */
        shorts[i] = (shorts[i] + (ints[i] & 0xFF)) & 0x7FFF;
    }
}

/* Function with complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[][100], int idx1, int idx2, int mask) {
    /* Generate MEM with complex XEXP */
    int val = arr[idx1][idx2];
    /* Combine with bit operation that might become ZERO_EXTRACT */
    return (val & mask) | ((val >> 16) & 0xFFFF);
}

int main(int argc, char **argv) {
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields(&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations with local volatile arrays */
    volatile short v_shorts[100];
    volatile int v_ints[100];
    volatile char v_chars[100];
    
    /* Initialize with argc-dependent values */
    for (int i = 0; i < argc % 50 + 10; i++) {
        v_ints[i] = argc + i * 3;
        v_chars[i] = (argc + i) & 0xFF;
    }
    
    mixed_width_ops((short *)v_shorts, (int *)v_ints, (char *)v_chars, 
                    argc % 50 + 10);
    
    /* 3. Complex array addressing with 2D array */
    int arr[100][100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j + argc;
        }
    }
    
    /* Use volatile indices to prevent constant propagation */
    volatile int idx1 = argc % 50;
    volatile int idx2 = (argc * 3) % 50;
    
    int result = complex_addressing(arr, idx1, idx2, 0x00FF00FF);
    
    /* 4. Additional mixed-type operations to generate more SUBREG patterns */
    volatile int vi = argc;
    volatile short vs;
    volatile char vc;
    
    /* Multiple assignments with type conversions */
    vs = vi & 0x7FFF;                    /* Potential SUBREG */
    vc = (vi >> 8) & 0xFF;               /* Another conversion */
    vi = vc * vs;                        /* Mixed-width operation */
    
    /* 5. Inline assembly to increase register pressure and force reload */
    asm volatile ("" 
                  : "=r"(vi), "=r"(vs) 
                  : "0"(vi), "1"(vs) 
                  : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 6. More bit-field manipulation with address taken */
    volatile struct BitFieldStruct local_bfs;
    volatile struct BitFieldStruct *pbfs = &local_bfs;
    
    /* Chain of bit-field assignments */
    pbfs->a = argc & 0xF;
    pbfs->b = (argc >> 4) & 0xFF;
    pbfs->c = result & 0xFFF;
    pbfs->d = (result >> 12) & 0xFFFFF;
    pbfs->e = (pbfs->a + pbfs->b + pbfs->c) & 0xFFFFFF;
    
    /* 7. Pointer casting for SUBREG patterns */
    volatile int *int_ptr = &vi;
    volatile short *short_ptr = (volatile short *)int_ptr;
    volatile char *char_ptr = (volatile char *)int_ptr;
    
    /* Access through different type pointers */
    *short_ptr = (*char_ptr + argc) & 0xFF;
    *(short_ptr + 1) = (argc >> 8) & 0xFF;
    
    /* 8. Compute checksum to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d + g_bfs.e;
    checksum += v_shorts[0] + v_ints[0] + v_chars[0];
    checksum += result + vi + vs + vc;
    checksum += pbfs->a + pbfs->b + pbfs->c + pbfs->d + pbfs->e;
    checksum += *short_ptr + *(short_ptr + 1);
    
    /* Use checksum so it can't be optimized away */
    if (checksum > 0) {
        return checksum & 0xFF;
    }
    
    return 0;
}
