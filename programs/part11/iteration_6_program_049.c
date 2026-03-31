/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -c this_file.c */
/* For debugging: gcc -O0 -fdump-rtl-expand -c this_file.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;   /* Will generate ZERO_EXTRACT */
    unsigned int b : 8;   /* Different widths increase pattern variety */
    unsigned int c : 20;  /* Larger bit-field */
    unsigned int d : 1;   /* Single bit - may generate STRICT_LOW_PART */
} g_bfs = {0};

/* 2D array for complex addressing modes */
static int arr_2d[100][100];

/* NOINLINE helper to force memory accesses and bit-field operations */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int iterations) {
    /* Multiple bit-field assignments to same structure */
    for (int i = 0; i < iterations; i++) {
        s->a = (i & 0xF);          /* 4-bit assignment */
        s->b = (i & 0xFF);         /* 8-bit assignment */
        s->c = (i & 0xFFFFF);      /* 20-bit assignment */
        s->d = (i & 0x1);          /* 1-bit assignment - may be STRICT_LOW_PART */
        
        /* Force memory barrier between assignments */
        asm volatile("" ::: "memory");
    }
}

/* Another NOINLINE function to create SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(volatile short *shorts, volatile char *chars, 
                           volatile int *ints, int count) {
    /* Mixed-width operations that should generate SUBREG RTL */
    for (int i = 0; i < count; i++) {
        /* int -> short assignment (truncation) */
        shorts[i] = (short)(ints[i] & 0xFFFF);
        
        /* char -> int promotion with sign extension */
        ints[i] = (int)chars[i] * 2;
        
        /* Mixed-width arithmetic */
        shorts[i] = (short)(ints[i] + chars[i]);
    }
}

/* Complex addressing with bit operations */
__attribute__((noinline))
int complex_addressing(volatile int idx_i, volatile int idx_j) {
    /* Volatile indices prevent constant propagation */
    int result = arr_2d[idx_i][idx_j];
    
    /* Bitwise operation that might generate ZERO_EXTRACT */
    result = result & 0x0000FFFF;  /* Mask lower 16 bits */
    
    /* Additional operation to use result */
    return result * 2;
}

int main(int argc, char **argv) {
    /* Use argc to create non-constant loop bounds */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* 1. Bit-field operations on global volatile structure */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, iterations);
    
    /* 2. Mixed-width local arrays for SUBREG generation */
    volatile short short_arr[50];
    volatile char char_arr[50];
    volatile int int_arr[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 50; i++) {
        int_arr[i] = i * 3;
        char_arr[i] = (char)(i & 0x7F);
    }
    
    /* Perform mixed-width operations */
    mixed_width_operations(short_arr, char_arr, int_arr, 
                          (iterations < 50) ? iterations : 50);
    
    /* 3. Complex 2D array addressing with volatile indices */
    volatile int idx_i = iterations % 100;
    volatile int idx_j = (iterations * 2) % 100;
    
    /* Initialize some array elements */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    int complex_result = complex_addressing(idx_i, idx_j);
    
    /* 4. Inline assembly to increase register pressure and force reload */
    asm volatile(
        "/* Clobber multiple registers to force register allocation */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        : 
        : "r" (complex_result), "r" (iterations)
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* 5. Additional mixed operations to create more RTL patterns */
    {
        volatile struct {
            unsigned int x : 10;
            unsigned int y : 6;
            unsigned int z : 16;
        } local_bf = {0};
        
        /* Direct bit-field assignments in main */
        local_bf.x = complex_result & 0x3FF;
        local_bf.y = (complex_result >> 10) & 0x3F;
        local_bf.z = iterations & 0xFFFF;
        
        /* Pointer casting for SUBREG patterns */
        volatile int *int_ptr = (volatile int*)&local_bf;
        volatile short *short_ptr = (volatile short*)int_ptr;
        
        /* Cross-type assignment through pointers */
        *short_ptr = (short)(*int_ptr & 0xFFFF);
    }
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    
    /* Use all modified data in checksum */
    checksum ^= g_bfs.a;
    checksum ^= g_bfs.b << 4;
    checksum ^= g_bfs.c << 8;
    checksum ^= complex_result;
    
    for (int i = 0; i < ((iterations < 50) ? iterations : 50); i++) {
        checksum ^= short_arr[i];
        checksum ^= int_arr[i] << 16;
    }
    
    /* Final volatile store to ensure all operations complete */
    volatile unsigned int final_result = checksum;
    
    printf("Result: %u (argc=%d)\n", final_result, argc);
    
    return (final_result > 0) ? 0 : 1;
}

/* Additional global to increase memory pressure */
volatile int extra_global[1000];

/* NOINLINE function that uses the global */
__attribute__((noinline))
void use_extra_global(int idx) {
    /* Complex pattern: bit-field in a loop with global */
    volatile struct {
        unsigned int f1 : 3;
        unsigned int f2 : 5;
        unsigned int f3 : 24;
    } local = {0};
    
    for (int i = 0; i < idx; i++) {
        local.f1 = (extra_global[i] & 0x7);
        local.f2 = (extra_global[i] >> 3) & 0x1F;
        local.f3 = (extra_global[i] >> 8) & 0xFFFFFF;
        
        /* Modify global based on local bit-fields */
        extra_global[i] = local.f1 | (local.f2 << 3) | (local.f3 << 8);
    }
}
