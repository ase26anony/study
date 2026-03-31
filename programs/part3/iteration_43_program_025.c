/* resource_patterns.c
 * This program is designed to generate specific RTL patterns that trigger
 * the uncovered lines in resource.cc (lines 282-290) during GCC compilation.
 * The code focuses on ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM patterns.
 */

#include <stddef.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void pattern_zero_extract_mem(volatile int iter) {
    /* Struct with volatile bit-fields to generate ZERO_EXTRACT */
    struct S {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 3;
        volatile unsigned int f3 : 8;
    } s;
    
    /* Array with complex indexing for MEM patterns */
    volatile int arr[10][10];
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    s.f1 = iter & 0x1F;
    s.f2 = (iter >> 5) & 0x7;
    s.f3 = (iter >> 8) & 0xFF;
    
    /* MEM: Complex addressing with multiple indices */
    int i = iter % 10;
    int j = (iter * 3) % 10;
    volatile int v = arr[i][j];
    
    /* More MEM patterns with pointer arithmetic */
    volatile int *ptr = &arr[0][0];
    ptr += i * 10 + j;
    volatile int v2 = *ptr;
    
    /* Prevent dead code elimination */
    (void)v2;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void pattern_strict_low_part_subreg(volatile int iter) {
    volatile char c = iter & 0xFF;
    volatile short s = iter & 0xFFFF;
    volatile int i = iter;
    
    /* STRICT_LOW_PART: Inline assembly modifying only low part of register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)      /* =q constraint for byte-addressable register */
        : "0"(c)       /* Matching input constraint */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "addw $1, %0\n\t"
        : "=r"(s)      /* Register constraint */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG: Type punning through pointer casts */
    int value = iter;
    short *ps = (short*)&value;  /* Cast to smaller type */
    *ps = (short)(iter + 1);     /* Assignment through SUBREG */
    
    /* More SUBREG patterns with mixed-size accesses */
    char *pc = (char*)&value;
    pc[1] = (char)(iter & 0xFF);
    
    /* Prevent dead code elimination */
    (void)i;
}

/* Function C: Complex expression mixing multiple patterns */
NOINLINE static void pattern_mixed(volatile int iter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct T {
        volatile unsigned int flags : 4;
        volatile unsigned int data : 12;
    } t;
    
    /* Array for MEM patterns */
    volatile int matrix[5][5];
    
    /* Complex expression with ternary operator */
    volatile int *addr = (iter & 1) ? 
        (volatile int*)&t.flags :  /* Bit-field address */
        (volatile int*)&matrix[iter % 5][0];  /* Array element address */
    
    /* Assignment that could involve multiple RTL transformations */
    t.flags = iter & 0xF;
    t.data = (iter >> 4) & 0xFFF;
    
    /* MEM with complex indexing */
    int idx1 = iter % 5;
    int idx2 = (iter * 2) % 5;
    volatile int val = matrix[idx1][idx2];
    
    /* Pointer arithmetic creating complex MEM addresses */
    volatile int *base = &matrix[0][0];
    base += idx1 * 5 + idx2;
    volatile int val2 = *base;
    
    /* Prevent dead code elimination */
    (void)addr;
    (void)val2;
}

/* Function D: Additional patterns for SUBREG and MEM */
NOINLINE static void pattern_subreg_mem(volatile int iter) {
    /* Use 64-bit types on 32-bit target to encourage SUBREG */
    volatile long long ll = iter;
    volatile int *p32 = (volatile int*)&ll;
    
    /* SUBREG: Access different parts of 64-bit value */
    p32[0] = iter;
    p32[1] = iter + 1;
    
    /* MEM: Complex addressing in loop-like pattern */
    volatile int buffer[100];
    for (int k = 0; k < 10; k++) {
        /* Complex addressing with multiple indices */
        int index = (iter + k * 7) % 100;
        volatile int x = buffer[index];
        
        /* More pointer arithmetic */
        volatile int *ptr = buffer + index;
        volatile int y = *ptr;
        
        (void)y;
    }
    
    /* Prevent dead code elimination */
    (void)ll;
}

int main(int argc, char *argv[]) {
    /* Use argc to bound loops - prevents infinite loops in analysis */
    volatile int iterations = (argc > 1) ? 10 : 5;
    volatile int sum = 0;
    
    /* Initialize some volatile data */
    volatile int counter = 0;
    
    /* Loop to generate repeated RTL patterns */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern function with arguments derived from loop */
        pattern_zero_extract_mem(i + counter);
        pattern_strict_low_part_subreg(i * 2 + counter);
        pattern_mixed(i * 3 + counter);
        pattern_subreg_mem(i * 4 + counter);
        
        /* Update volatile variables to prevent optimization */
        counter++;
        sum += i;
    }
    
    /* Final dummy operation using results */
    volatile int result = sum + counter;
    
    /* Return value based on compilation, not runtime behavior */
    return (result > 0) ? 0 : 1;
}
