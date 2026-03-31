/* Target: resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fdump-rtl-reload -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 8;    /* Another 8-bit field */
} g_bitfield = {0};

/* Force memory addressing with complex expressions */
volatile int g_index1 = 0;
volatile int g_index2 = 0;

/* Noinline function to prevent optimization of bit-field operations */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int iterations) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    for (int i = 0; i < iterations; i++) {
        s->a = (i & 0xF);          /* 4-bit assignment */
        s->b = (i & 0xFF);         /* 8-bit assignment */
        s->c = (i & 0xFFF);        /* 12-bit assignment */
        s->d = (i & 0xFF) ^ 0x55;  /* 8-bit assignment with operation */
        
        /* Mix with memory access to create addressing modes */
        volatile int temp = s->a + s->b;
        (void)temp;
    }
}

/* Another noinline function for SUBREG operations */
__attribute__((noinline))
void mixed_width_operations(short *short_arr, char *char_arr, int *int_arr, int n) {
    /* Operations that will generate SUBREG RTL patterns */
    for (int i = 0; i < n; i++) {
        /* int -> short assignment generates SUBREG */
        short_arr[i] = (short)(int_arr[i] & 0xFFFF);
        
        /* char -> int with sign extension */
        int_arr[i] = (int)char_arr[i] * 2;
        
        /* Mixed-width arithmetic */
        int temp = (int)short_arr[i] + (char_arr[i] << 4);
        int_arr[i] = temp;
    }
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int idx1, volatile int idx2) {
    /* Complex addressing mode that will be processed by XEXP(x, 0) */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Non-constant indices force complex address calculation */
            int idx_i = (idx1 + i) % 100;
            int idx_j = (idx2 + j) % 100;
            
            /* Access with bitwise operation that may involve ZERO_EXTRACT */
            sum += arr[idx_i][idx_j] & 0xFF;  /* Lower 8 bits only */
            
            /* Another access with different mask */
            sum -= (arr[idx_j][idx_i] >> 8) & 0xF;  /* Bits 8-11 */
        }
    }
    return sum;
}

/* Function to increase register pressure */
__attribute__((noinline))
void register_pressure(int iterations) {
    /* Many local variables to force register spilling */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    short s1, s2, s3, s4, s5;
    char c1, c2, c3, c4, c5;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed type operations generating SUBREG */
        s1 = (short)v1;
        s2 = (short)v2;
        s3 = (short)(v3 & 0xFFFF);
        s4 = (short)v4;
        s5 = (short)v5;
        
        c1 = (char)v6;
        c2 = (char)v7;
        c3 = (char)v8;
        c4 = (char)v9;
        c5 = (char)v10;
        
        /* Arithmetic mixing types */
        v1 = (int)s1 + c1;
        v2 = (int)s2 + c2;
        v3 = (int)s3 + c3;
        v4 = (int)s4 + c4;
        v5 = (int)s5 + c5;
        
        v6 = v1 * 2;
        v7 = v2 * 3;
        v8 = v3 * 4;
        v9 = v4 * 5;
        v10 = v5 * 6;
        
        /* Inline assembly to clobber registers and force reload */
        asm volatile("" 
                     : 
                     : 
                     : "r0", "r1", "r2", "r3", "r4", "r5", 
                       "r6", "r7", "r8", "r9", "r10", "memory");
    }
    
    /* Use variables to prevent elimination */
    volatile int result = v1 + v2 + v3 + v4 + v5 + 
                         v6 + v7 + v8 + v9 + v10 +
                         s1 + s2 + s3 + s4 + s5 +
                         c1 + c2 + c3 + c4 + c5;
    (void)result;
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
    
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * 3;
        char_arr[i] = (char)(i & 0x7F);
    }
    
    mixed_width_operations(short_arr, char_arr, int_arr, 50);
    
    /* 3. Complex array addressing */
    int arr_2d[100][100];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    volatile int idx1 = g_index1;
    volatile int idx2 = g_index2;
    int sum = complex_addressing(arr_2d, idx1, idx2);
    
    /* 4. Register pressure to force reload pass */
    register_pressure(iterations / 10);
    
    /* Additional volatile operations to prevent optimization */
    volatile int checksum = 0;
    checksum += g_bitfield.a;
    checksum += g_bitfield.b;
    checksum += g_bitfield.c;
    checksum += g_bitfield.d;
    
    for (int i = 0; i < 50; i++) {
        checksum += short_arr[i];
        checksum += char_arr[i];
        checksum += int_arr[i];
    }
    
    checksum += sum;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed. Check RTL dumps for coverage.\n");
    
    return 0;
}
