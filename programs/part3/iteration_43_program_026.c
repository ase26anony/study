/* resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources function).
 * The target patterns are: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM.
 * 
 * Compilation recommendations:
 *   gcc -O2 -m32 -fno-strict-aliasing -c resource_coverage.c -fdump-rtl-all
 *   gcc -O3 -funroll-loops -march=i686 -fno-strict-aliasing -c resource_coverage.c
 *   gcc -Os -m32 -fno-strict-aliasing -c resource_coverage.c -dP -da
 */

#include <stddef.h>

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *counter) 
{
    /* Struct with volatile bit-fields to generate ZERO_EXTRACT */
    struct S {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 3;
        volatile unsigned int f3 : 8;
    } s;
    
    /* Array with complex addressing for MEM patterns */
    volatile int arr[10][10];
    volatile int *ptr;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    s.f1 = (*counter) & 0x1F;
    s.f2 = ((*counter) >> 5) & 0x07;
    s.f3 = ((*counter) >> 8) & 0xFF;
    
    /* MEM: Complex addressing with pointer arithmetic */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Multiple indices create complex MEM addressing */
            ptr = &arr[i][j];
            *ptr = i * j + (*counter);
            
            /* More pointer arithmetic */
            ptr = ptr + (i - j);
            if (ptr >= &arr[0][0] && ptr < &arr[9][9]) {
                volatile int v = *ptr;  /* MEM access */
            }
        }
    }
    
    /* Mix bit-field with MEM addressing */
    struct S *sp = &s;
    volatile int *intp = (volatile int *)sp;
    *intp += (*counter);  /* This may generate both patterns */
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int *counter) 
{
    volatile char c = (*counter) & 0xFF;
    volatile short s = (*counter) & 0xFFFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART: Inline assembly modifying byte-sized part */
    asm volatile (
        "addb $1, %0\n\t"
        "subb $1, %0"
        : "=q"(c)   /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "incw %0\n\t"
        "decw %0"
        : "=r"(s)   /* May use 'r' but with appropriate mode */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG: Type punning with different-sized accesses */
    int *pi = &i;
    short *ps = (short *)pi;  /* Type punning - disable strict aliasing */
    char *pc = (char *)pi;
    
    /* Mixed-size accesses generating SUBREG */
    *ps = (*counter) & 0xFFFF;      /* SUBREG for 16-bit access to 32-bit reg */
    pc[1] = ((*counter) >> 8) & 0xFF; /* SUBREG for 8-bit access */
    
    /* More SUBREG: Access parts of larger type */
    volatile long long ll = (*counter) * 100LL;
    volatile int *ll_low = (volatile int *)&ll;
    volatile int *ll_high = ll_low + 1;
    
    *ll_low = (*counter);    /* SUBREG access to 64-bit value */
    *ll_high = (*counter) + 1;
}

/* Function C: Complex expression mixing patterns */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int *counter, volatile int selector) 
{
    /* Array for MEM patterns */
    static volatile int matrix[5][5];
    
    /* Struct with bit-fields for ZERO_EXTRACT */
    struct BF {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 10;
    } bf;
    
    /* Initialize */
    bf.a = 1;
    bf.b = 2;
    bf.c = 3;
    
    /* Complex addressing with ternary operator */
    volatile int *addr = (selector & 1) ? 
                         (volatile int *)&bf : 
                         (volatile int *)&matrix[0][0];
    
    /* MEM access with complex address calculation */
    int idx1 = (*counter) % 5;
    int idx2 = ((*counter) >> 3) % 5;
    
    /* This complex expression may generate multiple RTL patterns */
    volatile int val = matrix[idx1][idx2] + 
                      ((selector & 2) ? bf.a : bf.b);
    
    /* Assignment that could involve transformations */
    if (selector & 4) {
        /* ZERO_EXTRACT pattern */
        bf.c = val & 0x3FF;
    } else {
        /* MEM pattern with pointer arithmetic */
        volatile int *p = &matrix[idx1][0];
        p[idx2] = val;
    }
    
    /* SUBREG through type punning in the same expression */
    union {
        int i;
        short s[2];
    } u;
    u.i = val;
    u.s[0] = u.s[1] + 1;  /* SUBREG accesses */
}

/* Helper to force different addressing modes */
static void __attribute__((noinline))
force_complex_addressing(volatile int *arr, int size, volatile int *counter)
{
    /* Multiple indices create complex MEM addressing */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                /* 3D-like addressing */
                volatile int idx = (i * size * size + j * size + k) % (size * size * size);
                if (idx >= 0 && idx < size * size * size) {
                    volatile int v = arr[idx];  /* MEM with complex address */
                    arr[idx] = v + (*counter);
                }
            }
        }
    }
}

/* Main function that drives all patterns */
int main(int argc, char **argv) 
{
    volatile int iterations = 10;
    volatile int selector = 0;
    volatile int sum = 0;
    
    /* Use argc to bound iterations if provided */
    if (argc > 1) {
        /* Dummy use of argv to prevent optimization */
        selector = argv[0][0];
    }
    
    /* Array for complex addressing */
    volatile int big_arr[100];
    for (int i = 0; i < 100; i++) {
        big_arr[i] = i;
    }
    
    /* Main loop to trigger resource tracking across passes */
    for (volatile int i = 0; i < iterations; i++) {
        /* Update selector to vary paths */
        selector = (selector + i) & 7;
        
        /* Call pattern functions */
        pattern_zero_extract_mem(&i);
        pattern_strict_low_part_subreg(&i);
        pattern_mixed_complex(&i, selector);
        
        /* Force complex addressing patterns */
        force_complex_addressing(big_arr, 5, &i);
        
        /* Accumulate to prevent dead code elimination */
        sum += i;
        
        /* Volatile memory barrier to prevent reordering */
        asm volatile ("" : : : "memory");
    }
    
    /* Final dummy use of results */
    volatile int result = sum + selector;
    
    /* The program doesn't need to run correctly, just compile */
    /* Return 0 to satisfy compiler */
    return 0;
}
