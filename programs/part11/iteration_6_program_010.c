/* Target coverage for resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate specific RTL patterns */

/* Global volatile struct with bit-fields - will generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 8;    /* Another 8-bit field */
} g_bitfield = {0};

/* Force memory addressing with complex modes */
volatile int g_indices[4] = {10, 20, 30, 40};

/* NOINLINE function to prevent optimization of bit-field operations */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int iterations) {
    /* Multiple assignments to different bit-fields */
    for (int i = 0; i < iterations; i++) {
        s->a = (i & 0xF);           /* 4-bit assignment - ZERO_EXTRACT */
        s->b = ((i * 3) & 0xFF);    /* 8-bit assignment */
        s->c = ((i * 5) & 0xFFF);   /* 12-bit assignment */
        s->d = ((i * 7) & 0xFF);    /* Another 8-bit assignment */
        
        /* Mix with memory operations to force addressing modes */
        volatile int temp = g_indices[i & 3];
        s->b = temp & 0xFF;         /* Another bit-field store */
    }
}

/* Another NOINLINE function for SUBREG operations */
__attribute__((noinline, optimize("O0")))
void mixed_width_operations(short *short_arr, char *char_arr, int *int_arr, int n) {
    /* Operations that will generate SUBREG RTL */
    for (int i = 0; i < n; i++) {
        /* int to short assignment - generates SUBREG */
        short_arr[i] = (short)(int_arr[i] & 0xFFFF);
        
        /* char to int with sign extension - may generate SUBREG */
        int val = (int)char_arr[i];  /* Sign extension */
        
        /* Mixed-width arithmetic */
        int_arr[i] = val * 2 + (short_arr[i] & 0xFF);
        
        /* Another SUBREG pattern: store int to char */
        char_arr[i] = (char)(int_arr[i] & 0xFF);
    }
}

/* Complex addressing with 2D array */
__attribute__((noinline))
int complex_addressing(int size) {
    /* Large 2D array to force memory pressure */
    int arr[100][100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Volatile indices prevent constant propagation */
    volatile int idx1 = g_indices[0] % 100;
    volatile int idx2 = g_indices[1] % 100;
    volatile int idx3 = g_indices[2] % 100;
    volatile int idx4 = g_indices[3] % 100;
    
    /* Complex addressing expressions */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Multiple array accesses with volatile indices */
        sum += arr[idx1][idx2];
        sum += arr[idx3][idx4];
        
        /* Mix with bit-field like operation */
        sum = (sum & 0xFFFF) + (arr[i % 100][(i * 7) % 100] & 0xFF);
        
        /* Rotate indices */
        idx1 = (idx1 + 1) % 100;
        idx2 = (idx2 * 3) % 100;
    }
    
    return sum;
}

/* Function with high register pressure */
__attribute__((noinline, optimize("O3")))
void high_register_pressure(int iterations) {
    /* Many local variables to force register spilling */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    short s1 = 1, s2 = 2, s3 = 3, s4 = 4;
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    
    /* Complex operations mixing types */
    for (int i = 0; i < iterations; i++) {
        /* Operations that generate SUBREG */
        s1 = (short)(v1 + v2);
        s2 = (short)(v3 * v4);
        c1 = (char)(v5 & 0xFF);
        
        /* Mix widths in arithmetic */
        v1 = v1 + (int)s1;
        v2 = v2 + (int)c1;
        v3 = v3 * (int)s2;
        
        /* Bit-field like operations on integers */
        v4 = (v4 & 0xF) | ((v5 & 0xF) << 4);  /* Like packing bit-fields */
        v5 = (v5 & 0xFF00) | (v6 & 0xFF);     /* High and low byte */
        
        /* Inline assembly to clobber registers and force reload */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "memory");
    }
    
    /* Use all variables to prevent optimization */
    g_indices[0] = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    g_indices[1] = s1 + s2 + s3 + s4;
    g_indices[2] = c1 + c2 + c3 + c4;
}

int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    printf("Starting coverage test for resource.cc lines 282-290\n");
    
    /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct*)&g_bitfield, iterations);
    
    /* 2. Mixed-width operations for SUBREG generation */
    short short_arr[100];
    char char_arr[100];
    int int_arr[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * 3;
        char_arr[i] = (char)(i & 0x7F);
    }
    
    mixed_width_operations(short_arr, char_arr, int_arr, 50);
    
    /* 3. Complex addressing modes */
    int sum1 = complex_addressing(iterations % 50 + 10);
    
    /* 4. High register pressure to force reload pass */
    high_register_pressure(iterations % 20 + 5);
    
    /* 5. Additional mixed operations in main */
    volatile int mix_var = 0;
    for (int i = 0; i < iterations; i++) {
        /* int to short store - SUBREG */
        volatile short vs = (short)(mix_var & 0xFFFF);
        
        /* Bit-field simulation */
        mix_var = (mix_var & ~0xF) | (i & 0xF);  /* Like bit-field assignment */
        
        /* Memory access with complex address */
        int *ptr = &int_arr[i % 100];
        *ptr = (*ptr & 0xFF00) | (mix_var & 0xFF);
        
        /* Another inline asm for register pressure */
        asm volatile("" ::: "r5", "r6", "r7", "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = g_bitfield.a + g_bitfield.b + g_bitfield.c + g_bitfield.d;
    checksum += sum1;
    checksum += mix_var;
    
    for (int i = 0; i < 50; i++) {
        checksum += short_arr[i];
        checksum += char_arr[i];
        checksum += int_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed - check coverage with:\n");
    printf("  gcc -O2 -fdump-rtl-reload -fno-strict-aliasing this_file.c\n");
    printf("  gcc -O3 -fschedule-insns -fno-strict-aliasing this_file.c\n");
    
    return checksum != 0 ? 0 : 1;
}
