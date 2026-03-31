/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -c this_file.c */
/* For debugging: gcc -O0 -fdump-rtl-expand -c this_file.c */

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
    unsigned int g : 16;
} g_bfs;

/* Helper function to force bit-field assignments */
__attribute__((noinline, noipa))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
    s->e = (x + y) & 0x7;
    s->f = ((x ^ y) >> 3) & 0x1F;
    s->g = (x * y) & 0xFFFF;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline, noipa))
void mixed_width_ops(short *shorts, char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* Operations that generate SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;  /* Truncation to 16-bit */
        chars[i] = (ints[i] >> 16) & 0xFF;  /* Truncation to 8-bit */
        
        /* Sign extension operations that may generate SUBREG */
        int val = (int)((short)shorts[i]) * (int)chars[i];
        ints[i] = val + (int)((signed char)chars[i]);
    }
}

/* Function with complex addressing modes */
__attribute__((noinline, noipa))
int complex_addressing(int arr[][100], volatile int *idx1, volatile int *idx2) {
    int sum = 0;
    /* Complex array access with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Access with volatile indices prevents constant propagation */
            int val = arr[*idx1 + i][*idx2 + j];
            
            /* Bit-field like operation on the loaded value */
            unsigned int masked = val & 0x00000FFF;  /* Could generate ZERO_EXTRACT */
            sum += masked;
            
            /* Another potential ZERO_EXTRACT pattern */
            unsigned int high_bits = (val >> 20) & 0x3FF;
            sum -= high_bits;
        }
    }
    return sum;
}

/* Function to create register pressure */
__attribute__((noinline, noipa, optimize("O0")))
void create_register_pressure(void) {
    /* Many local variables to force register allocation */
    register int r0 asm("r0") = 1;
    register int r1 asm("r1") = 2;
    register int r2 asm("r2") = 3;
    register int r3 asm("r3") = 4;
    register int r4 asm("r4") = 5;
    register int r5 asm("r5") = 6;
    register int r6 asm("r6") = 7;
    register int r7 asm("r7") = 8;
    
    /* Volatile variables to prevent optimization */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    volatile int v5 = r5;
    volatile int v6 = r6;
    volatile int v7 = r7;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" 
                 : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3),
                   "=r"(r4), "=r"(r5), "=r"(r6), "=r"(r7)
                 : "0"(v0), "1"(v1), "2"(v2), "3"(v3),
                   "4"(v4), "5"(v5), "6"(v6), "7"(v7)
                 : "memory", "cc");
}

int main(int argc, char *argv[]) {
    /* Force argc to be used to prevent optimization */
    if (argc < 2) return 1;
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* Take address and modify again */
    struct BitFieldStruct *p_bfs = (struct BitFieldStruct*)&g_bfs;
    p_bfs->a = (argc + 1) & 0xF;
    p_bfs->b = (argc + 2) & 0xFF;
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_arr[100];
    volatile char char_arr[100];
    volatile int int_arr[100];
    
    /* Initialize with argc-dependent values */
    for (int i = 0; i < 100 && i < argc * 10; i++) {
        int_arr[i] = argc + i * 3;
    }
    
    mixed_width_ops((short*)short_arr, (char*)char_arr, (int*)int_arr, 
                   (argc > 50 ? 50 : argc * 2) % 100);
    
    /* 3. Complex addressing with 2D array */
    int big_arr[100][100];
    volatile int idx1 = argc % 50;
    volatile int idx2 = (argc * 3) % 50;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            big_arr[i][j] = i * 100 + j + argc;
        }
    }
    
    int addr_sum = complex_addressing(big_arr, &idx1, &idx2);
    
    /* 4. Create register pressure to force reload pass */
    create_register_pressure();
    
    /* Additional inline assembly to clobber more registers */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", 
                 "r9", "r10", "r11", "r12", "memory");
    
    /* 5. More bit-field operations combined with memory accesses */
    {
        volatile struct NestedBitFields {
            struct {
                unsigned int x : 8;
                unsigned int y : 8;
                unsigned int z : 16;
            } inner;
            unsigned int full : 32;
        } nested;
        
        /* Combined operations that may generate complex SET_DEST patterns */
        nested.inner.x = argc & 0xFF;
        nested.inner.y = (argc >> 8) & 0xFF;
        nested.inner.z = addr_sum & 0xFFFF;
        nested.full = nested.inner.x | (nested.inner.y << 8) | (nested.inner.z << 16);
        
        /* Access through pointer with offset */
        unsigned int *ptr = (unsigned int*)&nested;
        ptr[0] = ptr[0] ^ 0xAAAAAAAA;
        ptr[1] = ptr[1] ^ 0x55555555;
    }
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c;
    checksum += short_arr[0] + char_arr[0] + int_arr[0];
    checksum += addr_sum;
    
    /* Use checksum so it can't be optimized away */
    if (checksum == 0xDEADBEEF) {
        printf("Impossible!\n");
    }
    
    printf("Checksum: %u (argc=%d)\n", checksum, argc);
    
    return checksum & 0xFF;
}
