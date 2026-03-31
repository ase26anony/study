/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */
/* Target: Trigger mark_referenced_resources for SET_DEST with ZERO_EXTRACT/STRICT_LOW_PART/SUBREG -> MEM patterns */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
    unsigned int e : 3;
    unsigned int f : 5;
} g_bfs;

/* Non-inline function to force memory addressing modes */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (x * y) & 0xFFFFF;
    s->e = (x ^ y) & 0x7;
    s->f = (x + y) & 0x1F;
    
    /* Mix with regular memory access */
    volatile int *p = (volatile int*)s;
    *p ^= 0x12345678;
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (ints[i] >> 16) & 0xFF;      /* int -> char truncation */
        
        /* More SUBREG patterns with arithmetic */
        ints[i] = (int)shorts[i] * (int)chars[i];  /* short/char -> int promotion */
        
        /* Complex expression with SUBREG */
        shorts[i] = (shorts[i] + (short)(ints[i] & 0xFF)) & 0x7FFF;
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    int sum = 0;
    /* Complex addressing with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Generate MEM with complex address expression */
            sum += arr[*idx1 + i][*idx2 + j] & 0x3FF;  /* ZERO_EXTRACT-like pattern */
            
            /* More complex addressing with bit operations */
            arr[*idx1 + i][*idx2 + j] ^= (sum << 4) | (sum >> 28);
        }
    }
    return sum;
}

/* Function with high register pressure */
__attribute__((noinline, optimize("O0")))
void high_register_pressure(void) {
    /* Many local variables to force register spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed-width operations generating SUBREG */
    s1 = v1 & 0xFFFF;
    s2 = v2 & 0xFFFF;
    s3 = v3 & 0xFFFF;
    s4 = v4 & 0xFFFF;
    s5 = v5 & 0xFFFF;
    
    c1 = v1 & 0xFF;
    c2 = v2 & 0xFF;
    c3 = v3 & 0xFF;
    c4 = v4 & 0xFF;
    c5 = v5 & 0xFF;
    
    /* Complex expressions */
    v1 = (int)s1 * (int)c1 + (int)s2 * (int)c2;
    v2 = (int)s3 * (int)c3 + (int)s4 * (int)c4;
    v3 = v1 ^ v2;
    
    /* Inline assembly to clobber registers and increase pressure */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory");
    
    /* Bit-field like operations on integers */
    v4 = (v3 & 0xF) | ((v2 & 0xFF) << 4) | ((v1 & 0xFFF) << 12);
    
    /* Force memory access with complex addressing */
    volatile int *ptr = &v5;
    *ptr = v4;
}

int main(int argc, char **argv) {
    /* Initialize with non-constant values */
    int init_val = argc * 12345;
    
    /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART) */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, init_val, init_val + 1);
    
    /* 2. Mixed-width operations (SUBREG patterns) */
    short shorts[100];
    int ints[100];
    char chars[100];
    
    for (int i = 0; i < 100; i++) {
        ints[i] = init_val + i * 3;
    }
    
    mixed_width_ops(shorts, ints, chars, argc % 50 + 10);
    
    /* 3. Complex addressing modes */
    int arr[100][100];
    volatile int idx1 = argc % 50;
    volatile int idx2 = (argc * 17) % 50;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    int addr_sum = complex_addressing(arr, &idx1, &idx2);
    
    /* 4. High register pressure section */
    high_register_pressure();
    
    /* 5. More bit-field operations with volatile */
    volatile struct BitFieldStruct local_bfs;
    local_bfs.a = (init_val >> 0) & 0xF;
    local_bfs.b = (init_val >> 4) & 0xFF;
    local_bfs.c = (init_val >> 12) & 0xFFF;
    local_bfs.d = (init_val >> 24) & 0xFFFFF;
    local_bfs.e = (addr_sum) & 0x7;
    local_bfs.f = (addr_sum >> 3) & 0x1F;
    
    /* 6. Additional SUBREG patterns through pointer casting */
    volatile int vi = init_val;
    volatile short *vp = (volatile short*)&vi;
    vp[0] = vi & 0xFFFF;      /* Generates SUBREG store */
    vp[1] = (vi >> 16) & 0xFFFF;
    
    /* 7. Compute checksum to prevent elimination */
    int checksum = 0;
    checksum ^= g_bfs.a + g_bfs.b + g_bfs.c;
    checksum ^= local_bfs.d + local_bfs.e + local_bfs.f;
    
    for (int i = 0; i < 10; i++) {
        checksum ^= shorts[i] + ints[i] + chars[i];
    }
    
    checksum ^= addr_sum;
    checksum ^= vi;
    checksum ^= vp[0] + vp[1];
    
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
