/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 */

#include <stddef.h>

/* Force functions to not be inlined to ensure RTL generation */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct S {
        volatile unsigned int f1:5;
        volatile unsigned int f2:7;
        volatile unsigned int f3:10;
    } s;
    
    /* Array with complex addressing for MEM patterns */
    int arr[10][10];
    volatile int idx1 = *counter % 10;
    volatile int idx2 = (*counter + 1) % 10;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = idx1 & 0x1F;
    s.f2 = idx2 & 0x7F;
    
    /* MEM pattern with complex addressing */
    volatile int v = arr[idx1][idx2];
    
    /* Combine both: MEM addressing with bit-field result */
    arr[idx1][idx2] = s.f1 + s.f2;
    
    /* Prevent dead code elimination */
    *counter += v;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART pattern using inline assembly */
    /* Modify only low byte of a register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)    /* =q constraint for byte-addressable register */
        : "0"(c)     /* Matching input constraint */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "addw $1, %0\n\t"
        : "=r"(s)    /* Register constraint */
        : "0"(s)     /* Matching input */
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    /* Cast int to short pointer for SUBREG access */
    short *ps = (short*)&i;
    *ps = s;  /* This generates SUBREG RTL */
    
    /* More SUBREG: access char through int pointer */
    char *pc = (char*)&i;
    pc[1] = c;
    
    /* Prevent dead code elimination */
    *counter = i + s + c;
}

/* Function C: Complex expression mixing patterns */
NOINLINE static void func_c(volatile int *counter) {
    /* Struct with bit-fields at different offsets */
    struct Mixed {
        volatile unsigned int a:3;
        volatile unsigned int b:9;
        volatile unsigned int c:20;
    } m;
    
    /* Array for MEM patterns */
    volatile int array[5][5][5];
    volatile int idx = *counter;
    
    /* Complex addressing with ternary operator */
    int *ptr = (idx & 1) ? 
               (int*)&m.a :  /* Bit-field address */
               (int*)&array[idx % 5][(idx + 1) % 5][(idx + 2) % 5]; /* Array element */
    
    /* ZERO_EXTRACT assignment */
    m.a = idx & 0x07;
    m.b = (idx >> 3) & 0x1FF;
    m.c = (idx >> 12) & 0xFFFFF;
    
    /* MEM access through computed pointer */
    volatile int val = *ptr;
    
    /* Assignment that could involve multiple RTL transformations */
    *ptr = m.b + (val & 0xFF);
    
    /* Prevent dead code elimination */
    *counter += m.a + m.c;
}

/* Function D: Additional patterns with loops */
NOINLINE static void func_d(volatile int *counter) {
    /* Mixed size accesses for SUBREG */
    long long ll = *counter * 100LL;
    int *p_int = (int*)&ll;
    short *p_short = (short*)&ll;
    char *p_char = (char*)&ll;
    
    /* SUBREG patterns through pointer casting */
    p_int[0] = *counter;
    p_short[2] = (*counter >> 8) & 0xFFFF;
    p_char[5] = (*counter >> 16) & 0xFF;
    
    /* Complex MEM addressing in loop-like pattern */
    volatile int matrix[8][8];
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 4; j++) {
            /* MEM with complex index calculation */
            matrix[i * 2][j * 2] = p_int[i & 1] + p_short[j & 3];
        }
    }
    
    /* Final MEM access */
    volatile int last = matrix[0][0];
    
    /* Prevent dead code elimination */
    *counter += ll + last;
}

/* Main function that drives all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int iterations = 10;
    
    /* Use argc to bound iterations if provided */
    if (argc > 1) {
        iterations = 5; /* Small number for compilation testing */
    }
    
    /* Loop to generate repeated RTL patterns */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern-generating function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter);
        func_d(&counter);
        
        /* Additional volatile operations to prevent optimization */
        counter += i;
    }
    
    /* Final dummy result to prevent dead code elimination */
    volatile int result = counter;
    
    /* The program doesn't need to run correctly, just compile */
    /* Return 0 to satisfy compiler */
    return 0;
}
