/* gcc -O2 -m32 -fno-strict-aliasing -funroll-loops -fdump-rtl-all -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_a(volatile int *arr, int idx1, int idx2) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bit_struct = {0};
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bit_struct.field1 = 1;
    bit_struct.field2 = idx1 & 0x7;
    bit_struct.field3 = idx2 & 0xFF;
    
    /* MEM pattern with complex addressing */
    volatile int val = arr[(idx1 * 7 + idx2 * 3) % 64];
    
    /* Use both to prevent elimination */
    bit_struct.field1 = val & 0x1F;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_b(int base) {
    volatile char c = base & 0xFF;
    volatile short s = base & 0xFFFF;
    int combined = (s << 16) | c;
    
    /* STRICT_LOW_PART pattern via inline asm */
    /* Modify only low byte of a register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)  /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG pattern via type punning */
    int *pi = &combined;
    short *ps = (short *)pi;  /* Cast to smaller type */
    *ps = (*ps + 1) & 0xFFFF;  /* Access via SUBREG */
    
    /* Another SUBREG pattern with different sizes */
    char *pc = (char *)&combined;
    pc[2] = pc[1] + pc[0];
    
    /* Use results to prevent elimination */
    volatile int dummy = combined + c;
}

/* Function C: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_c(volatile int *mem_base, int selector) {
    /* Struct with bit-fields at different positions */
    struct mixed {
        volatile unsigned int low : 4;
        volatile unsigned int mid : 12;
        volatile unsigned int high : 16;
    } m = {0};
    
    /* Complex expression with ternary selecting address */
    volatile int *addr = selector ? 
        (volatile int *)((char *)mem_base + m.low * 4) : 
        (volatile int *)((char *)mem_base + m.mid * 2);
    
    /* ZERO_EXTRACT assignment based on selected address */
    m.low = (*addr) & 0xF;
    m.mid = ((*addr) >> 4) & 0xFFF;
    
    /* MEM access with the computed address */
    volatile int loaded = addr[selector];
    
    /* Use in bit-field assignment (potential ZERO_EXTRACT) */
    m.high = loaded & 0xFFFF;
}

/* Helper with array indexing for MEM patterns */
static void __attribute__((noinline))
mem_intensive(int size, volatile int *arr) {
    int i, j;
    /* 2D array access pattern generating complex MEM RTL */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            /* Complex addressing with multiple indices */
            volatile int v = arr[i * size + j];
            arr[j * size + i] = v + 1;  /* Transpose with update */
        }
    }
}

int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* Volatile data to prevent optimization */
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Arrays for MEM patterns */
    volatile int array1[64];
    volatile int array2[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        array1[i] = i * 3;
    }
    for (int i = 0; i < 100; i++) {
        array2[i] = i * 7;
    }
    
    /* Main loop to trigger resource tracking in multiple passes */
    for (volatile int iter = 0; iter < iterations; iter++) {
        int idx1 = (iter * 17) % 64;
        int idx2 = (iter * 23) % 64;
        
        /* Call pattern functions with volatile-derived values */
        pattern_a(array1, idx1, idx2);
        pattern_b(iter + counter);
        pattern_c(array2, iter & 1);
        
        /* MEM-intensive operations */
        mem_intensive(8, array1);
        
        /* Update volatile state to prevent dead code elimination */
        counter++;
        sum += array1[idx1] + array2[idx2];
    }
    
    /* Final dummy use to prevent elimination */
    volatile int result = sum + counter;
    
    /* The program would crash if actually run due to type punning,
       but it's valid for compilation */
    return 0;
}
