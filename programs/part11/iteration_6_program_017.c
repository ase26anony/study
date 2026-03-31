/* Target: resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate ZERO_EXTRACT/STRICT_LOW_PART patterns */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
} g_bfs;

/* Force memory addressing modes */
volatile int g_mem_pressure[256];
volatile short g_short_array[256];
volatile char g_char_array[256];

/* Prevent optimization of helper functions */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to force ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
    
    /* Mix with memory operations */
    g_mem_pressure[s->a] = s->b;
    g_mem_pressure[s->b] = s->c;
}

__attribute__((noinline, optimize("O0")))
void complex_addressing(int i, int j, int k) {
    /* Multi-dimensional array with volatile indices */
    static int arr[100][100];
    volatile int vi = i;
    volatile int vj = j;
    
    /* Complex addressing that may generate SUBREG + MEM patterns */
    arr[vi % 100][vj % 100] = k;
    
    /* Bit-field like operation using masking */
    int masked = arr[vi % 100][vj % 100] & 0xFF;
    g_short_array[vi % 256] = masked;  /* SUBREG pattern */
}

__attribute__((noinline, optimize("O0")))
void mixed_width_operations(int iterations) {
    volatile int vint;
    volatile short vshort;
    volatile char vchar;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed width operations to generate SUBREG patterns */
        vint = i * 256;
        vshort = vint;  /* Potential SUBREG in SET_DEST */
        vchar = vshort & 0xFF;
        
        /* Reverse with sign extension */
        vint = vchar;  /* Potential SUBREG in SET_DEST */
        vint = vshort; /* Another SUBREG pattern */
        
        /* Complex expression mixing types */
        g_char_array[i % 256] = (vint + vshort + vchar) & 0xFF;
    }
}

/* Force register pressure and resource tracking */
__attribute__((noinline, optimize("O0")))
void high_register_pressure(void) {
    /* Many local variables to force spilling */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    
    /* Initialize with complex expressions */
    r0 = g_bfs.a * 256;
    r1 = g_bfs.b * 128;
    r2 = g_bfs.c * 64;
    r3 = g_bfs.d * 32;
    
    /* Chain calculations to prevent optimization */
    r4 = r0 + r1;
    r5 = r2 + r3;
    r6 = r4 - r5;
    r7 = r0 * r2;
    r8 = r1 * r3;
    r9 = r7 / (r8 ? r8 : 1);
    
    r10 = r4 | r5;
    r11 = r6 & r7;
    r12 = r8 ^ r9;
    r13 = ~r10;
    r14 = r11 << 2;
    r15 = r12 >> 1;
    r16 = r13 + r14;
    r17 = r15 - r16;
    r18 = r17 * 3;
    r19 = r18 % 17;
    
    /* Force memory operations with mixed types */
    g_mem_pressure[0] = r0;
    g_mem_pressure[1] = (short)r1;  /* SUBREG pattern */
    g_mem_pressure[2] = (char)r2;   /* SUBREG pattern */
    
    /* Inline assembly to clobber registers and force resource tracking */
    asm volatile("" 
                 : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4)
                 : "0"(r0), "1"(r1), "2"(r2), "3"(r3), "4"(r4)
                 : "memory", "cc");
}

int main(int argc, char **argv) {
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    printf("Starting coverage test for resource.cc lines 282-290\n");
    
    /* 1. Trigger ZERO_EXTRACT/STRICT_LOW_PART patterns */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, 0x12345678, 0x9ABCDEF0);
    
    /* 2. Force SUBREG patterns with mixed-width operations */
    mixed_width_operations(iterations);
    
    /* 3. Complex addressing modes */
    complex_addressing(iterations, iterations * 2, iterations * 3);
    
    /* 4. High register pressure to force reload pass */
    high_register_pressure();
    
    /* 5. Additional bit-field operations in main */
    volatile struct BitFieldStruct local_bfs;
    local_bfs.a = argc & 0xF;
    local_bfs.b = (argc >> 4) & 0xFF;
    local_bfs.c = (argc >> 8) & 0xFFF;
    local_bfs.d = (argc >> 20) & 0xFFFFF;
    
    /* 6. More mixed-type operations */
    volatile int vi = argc;
    volatile short vs = vi;  /* SUBREG pattern */
    volatile char vc = vs & 0x7F;
    
    /* Array access with bit-field like masking */
    int arr2d[50][50];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex addressing */
            arr2d[i][j] = (vi * i + vs * j + vc) & 0xFF;
            
            /* Bit-field assignment pattern */
            g_short_array[(i * 10 + j) % 256] = arr2d[i][j] & 0xFFFF;
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    checksum += g_mem_pressure[0] + g_mem_pressure[100];
    checksum += g_short_array[0] + g_char_array[0];
    checksum += local_bfs.a + local_bfs.b + local_bfs.c + local_bfs.d;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed. Compile with -O2 -fschedule-insns -fno-strict-aliasing\n");
    
    return checksum != 0 ? 0 : 1;
}
