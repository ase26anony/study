/* Target: Generate RTL patterns to cover lines 282-290 in resource.cc
 * These lines handle SET_DEST processing for ZERO_EXTRACT, STRICT_LOW_PART,
 * SUBREG, and MEM patterns in mark_referenced_resources()
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 8;    /* Another 8-bit field */
} g_bitfield = {0};

/* 2D array for complex addressing modes */
static int g_array[100][100];

/* Non-inline function to force memory addressing and prevent optimization */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(volatile struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART patterns */
    s->a = x & 0xF;        /* 4-bit assignment */
    s->b = (x >> 4) & 0xFF; /* 8-bit assignment */
    s->c = y & 0xFFF;      /* 12-bit assignment */
    s->d = (y >> 12) & 0xFF; /* 8-bit assignment */
    
    /* Mix with memory access using complex addressing */
    g_array[x % 100][y % 100] = s->a + s->b;
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns by mixing widths */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        ints[i] = chars[i] * 2;                 /* char -> int promotion */
        
        /* Complex expression with mixed types */
        shorts[i] = (shorts[i] + (short)chars[i]) & 0x7FFF;
    }
}

/* Function to create register pressure */
__attribute__((noinline, optimize("O0")))
void create_register_pressure(void) {
    /* Many local variables to force register allocation/spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed-width operations generating SUBREG */
    s1 = v1 & 0xFF;
    s2 = v2 & 0xFF;
    s3 = v3 & 0xFF;
    s4 = v4 & 0xFF;
    s5 = v5 & 0xFF;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* Use the variables to prevent dead code elimination */
    c1 = s1 & 0xF;
    c2 = s2 & 0xF;
    c3 = s3 & 0xF;
    c4 = s4 & 0xF;
    c5 = s5 & 0xF;
    
    /* More mixed operations */
    v1 = c1 * 256 + s1;
    v2 = c2 * 256 + s2;
    v3 = c3 * 256 + s3;
}

int main(int argc, char **argv) {
    /* Use argc to create non-constant loop bounds */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields(&g_bitfield, argc, iterations);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_array[100];
    volatile int int_array[100];
    volatile char char_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100 && i < iterations * 10; i++) {
        int_array[i] = i * 3;
        char_array[i] = i & 0x7F;
    }
    
    mixed_width_operations(short_array, int_array, char_array, 
                          (iterations < 100) ? iterations : 100);
    
    /* 3. Complex addressing with 2D array */
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 3) % 50;
    
    /* Complex memory access pattern */
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < iterations && j < 50; j++) {
            /* Access with volatile indices to prevent constant propagation */
            int val = g_array[idx_i + i][idx_j + j];
            
            /* Bitwise operation that might generate ZERO_EXTRACT */
            g_array[idx_i + i][idx_j + j] = (val & 0x00FF00FF) | 
                                           ((val & 0xFF00FF00) >> 8);
            
            /* Mixed-width store */
            short_array[(i * j) % 100] = g_array[idx_i + i][idx_j + j] & 0xFFFF;
        }
    }
    
    /* 4. Create register pressure to trigger resource tracking */
    create_register_pressure();
    
    /* 5. Additional bit-field manipulation */
    volatile struct BitFieldStruct local_bitfield;
    
    /* Direct bit-field assignments on local volatile struct */
    local_bitfield.a = argc & 0xF;
    local_bitfield.b = (argc >> 4) & 0xFF;
    local_bitfield.c = iterations & 0xFFF;
    local_bitfield.d = (iterations >> 4) & 0xFF;
    
    /* Combine with pointer arithmetic */
    unsigned char *byte_ptr = (unsigned char*)&local_bitfield;
    for (int i = 0; i < sizeof(local_bitfield); i++) {
        byte_ptr[i] ^= 0x55;  /* Modify individual bytes */
    }
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    for (int i = 0; i < 100 && i < iterations * 2; i++) {
        checksum += short_array[i];
        checksum += int_array[i % 100];
        checksum += char_array[i % 100];
    }
    
    checksum += g_bitfield.a + g_bitfield.b + g_bitfield.c + g_bitfield.d;
    checksum += local_bitfield.a + local_bitfield.b + 
                local_bitfield.c + local_bitfield.d;
    
    /* Access 2D array with complex indices for final checksum */
    for (int i = 0; i < 10 && i < iterations; i++) {
        volatile int idx1 = (argc + i) % 50;
        volatile int idx2 = (argc * i) % 50;
        checksum += g_array[idx1][idx2];
    }
    
    printf("Checksum: %u\n", checksum);
    
    /* One more inline assembly to clobber registers */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", 
                 "r6", "r7", "r8", "r9", "r10", "memory");
    
    return (checksum > 0) ? 0 : 1;
}
