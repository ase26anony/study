/* Target: resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate specific RTL patterns */
#define NOINLINE __attribute__((noinline))
#define NOPTIMIZE __attribute__((optimize("O0")))

/* Global volatile struct with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs = {0};

/* NOINLINE function to force memory addressing modes */
NOINLINE void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFF;
    
    /* Additional assignment to ensure SET_DEST processing */
    s->a = (s->b + s->c) & 0xF;
}

/* Function with mixed-width operations to generate SUBREG patterns */
NOPTIMIZE void mixed_width_operations(int argc, char **argv) {
    volatile short vs;
    volatile char vc;
    volatile int vi;
    
    /* Mixed type operations to generate SUBREG RTL */
    for (int i = 0; i < argc; i++) {
        /* int -> short assignment generates SUBREG */
        vs = (short)(vi + i);
        
        /* char -> int with sign extension */
        vc = (char)(i * 3);
        vi = vc;  /* This may generate SUBREG or ZERO_EXTEND */
        
        /* Complex expression with mixed types */
        vi = (vs << 8) | (vc & 0xFF);
    }
    
    /* Array with complex addressing */
    volatile int arr[100][100];
    volatile int idx_i = argc % 50;
    volatile int idx_j = (argc * 3) % 50;
    
    /* Complex memory access pattern */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* This generates complex addressing modes */
            arr[idx_i + i][idx_j + j] = (vi + vs + vc) & 0xFFFF;
            
            /* Bit-field like operation on array element */
            arr[idx_i + i][idx_j + j] &= 0xFF;  /* May generate ZERO_EXTRACT */
        }
    }
    
    /* Force register pressure with inline assembly */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
}

/* Another function to create more RTL patterns */
NOINLINE void create_subreg_patterns(void) {
    volatile struct {
        int full;
        short half;
        char quarter;
    } data;
    
    /* Generate SUBREG patterns through type punning */
    int *p_int = &data.full;
    short *p_short = &data.half;
    char *p_char = &data.quarter;
    
    /* Cross-type assignments */
    *p_short = (short)(*p_int >> 16);
    *p_char = (char)(*p_int & 0xFF);
    
    /* Pointer arithmetic with different types */
    p_char[0] = p_short[0] & 0xFF;
    p_short[0] = p_int[0] >> 8;
    
    /* Compound assignment with mixed types */
    data.half = data.quarter + (data.full & 0xFF);
}

/* Function to use STRICT_LOW_PART patterns */
NOINLINE void strict_low_part_pattern(void) {
    volatile struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } pair = {0};
    
    /* These assignments may generate STRICT_LOW_PART */
    pair.low = 0xABCD;
    pair.high = 0x1234;
    
    /* Operation that uses both parts */
    unsigned int combined = (pair.high << 16) | pair.low;
    
    /* Assignment back to bit-field */
    pair.low = combined & 0xFFFF;
    pair.high = (combined >> 16) & 0xFFFF;
}

int main(int argc, char **argv) {
    /* Initialize with non-zero values */
    if (argc < 2) argc = 5;  /* Ensure loops execute */
    
    printf("Starting coverage test for resource.cc lines 282-290\n");
    
    /* 1. Process bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct *)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations for SUBREG generation */
    mixed_width_operations(argc, argv);
    
    /* 3. Additional SUBREG patterns */
    create_subreg_patterns();
    
    /* 4. STRICT_LOW_PART patterns */
    strict_low_part_pattern();
    
    /* Create memory pressure with many variables */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v1 = argc; v2 = argc * 2; v3 = argc * 3; v4 = argc * 4; v5 = argc * 5;
    v6 = argc * 6; v7 = argc * 7; v8 = argc * 8; v9 = argc * 9; v10 = argc * 10;
    
    /* Complex expression to prevent optimization */
    volatile int checksum = 
        g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d +
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* More inline assembly to force resource tracking */
    asm volatile("" ::: 
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12", "memory");
    
    printf("Checksum: %d\n", checksum);
    printf("Bit-field values: a=%u, b=%u, c=%u, d=%u\n", 
           g_bfs.a, g_bfs.b, g_bfs.c, g_bfs.d);
    
    return checksum != 0 ? 0 : 1;
}
