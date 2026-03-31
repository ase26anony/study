/* Compile with: gcc -O2 -fdump-rtl-reload -fno-strict-aliasing -o coverage_test coverage_test.c */

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
} g_bfs = {0};

/* Non-inline function to force memory addressing modes */
__attribute__((noinline)) 
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
    s->e = (x ^ y) & 0x7;
    s->f = (x + y) & 0x1F;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile char *chars, int *ints, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        shorts[i] = ints[i] & 0xFFFF;  /* Truncation to 16-bit */
        chars[i] = ints[i] & 0xFF;     /* Truncation to 8-bit */
        
        /* Sign extension operations that may generate SUBREG */
        ints[i] = (int)shorts[i] * 2;  /* short to int promotion */
        ints[i] += (signed char)chars[i]; /* char to int with sign extension */
    }
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    int sum = 0;
    /* Complex addressing with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Generate MEM with complex XEXP */
            int val = arr[*idx1 + i][*idx2 + j];
            
            /* Bitwise operation that might be represented as ZERO_EXTRACT */
            sum += val & 0x00FF00FF;  /* Mask alternating bytes */
            
            /* Another bit-field like operation */
            sum ^= (val >> 8) & 0xFF;
        }
    }
    return sum;
}

/* Function to create register pressure */
__attribute__((noinline, __optimize__("O0")))
void create_register_pressure(void) {
    /* Many local variables to force register allocation/spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed-width operations generating SUBREG */
    s1 = v1; s2 = v2; s3 = v3; s4 = v4; s5 = v5;
    c1 = v1; c2 = v2; c3 = v3; c4 = v4; c5 = v5;
    
    /* Inline assembly to clobber registers and force resource tracking */
    asm volatile("" ::: 
        "r0", "r1", "r2", "r3", "r4", "r5", 
        "r6", "r7", "r8", "r9", "r10", "memory");
    
    /* Use all variables to prevent optimization */
    v1 = s1 + c1;
    v2 = s2 + c2;
    v3 = s3 + c3;
    v4 = s4 + c4;
    v5 = s5 + c5;
}

int main(int argc, char *argv[]) {
    /* Force argc to be used to prevent constant propagation */
    if (argc < 2) return 1;
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_arr[50];
    volatile char char_arr[50];
    int int_arr[50];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 50; i++) {
        int_arr[i] = argc + i;
    }
    
    mixed_width_ops(short_arr, char_arr, int_arr, 50);
    
    /* 3. Complex array addressing */
    int matrix[100][100];
    volatile int idx1 = argc % 50;
    volatile int idx2 = (argc * 3) % 50;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    int addr_result = complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. Create register pressure */
    create_register_pressure();
    
    /* 5. Additional volatile bit-field operations in main */
    volatile struct {
        unsigned int low : 10;
        unsigned int mid : 10;
        unsigned int high : 12;
    } local_bf = {0};
    
    /* Multiple assignments to same bit-field structure */
    for (int i = 0; i < 10; i++) {
        local_bf.low = i & 0x3FF;
        local_bf.mid = (i * 2) & 0x3FF;
        local_bf.high = (i * 3) & 0xFFF;
        
        /* Take address and dereference to force MEM patterns */
        unsigned int *ptr = (unsigned int*)&local_bf;
        *ptr ^= 0xAAAAAAAA;
    }
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d + g_bfs.e + g_bfs.f;
    
    for (int i = 0; i < 50; i++) {
        checksum += short_arr[i];
        checksum += char_arr[i];
        checksum += int_arr[i];
    }
    
    checksum += addr_result;
    checksum += local_bf.low + local_bf.mid + local_bf.high;
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
