/* Compile with: gcc -O2 -m32 -fno-strict-aliasing -c -fdump-rtl-all */
#include <stdint.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_a(volatile int *arr, int idx1, int idx2) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5;
        volatile unsigned int f2:3;
        volatile unsigned int f3:8;
    } s;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s.f1 = 1;
    s.f2 = idx1 & 0x7;
    s.f3 = idx2 & 0xFF;
    
    /* MEM pattern with complex addressing */
    volatile int *ptr = arr + idx1 * 10 + idx2;
    *ptr = s.f1 + s.f2;
    
    /* More complex MEM addressing */
    volatile int v = *(ptr + (idx1 & 3));
    (void)v;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_b(volatile short *ps, volatile char *pc) {
    /* STRICT_LOW_PART pattern using inline assembly */
    unsigned char var1 = *pc;
    /* Modify only low byte of register */
    asm volatile ("addb $1, %0" : "=q"(var1) : "0"(var1) : "cc");
    *pc = var1;
    
    /* Another STRICT_LOW_PART with short */
    unsigned short var2 = *ps;
    asm volatile ("addw $1, %0" : "=r"(var2) : "0"(var2) : "cc");
    *ps = var2;
    
    /* SUBREG pattern: type punning between different sizes */
    int i = 0x12345678;
    /* Access through smaller type pointer */
    short *psi = (short*)&i;
    *psi = 0xABCD;  /* Generates SUBREG in RTL */
    
    /* More SUBREG: mixed size access */
    long long ll = 0x1122334455667788ULL;
    int *pint = (int*)&ll;
    *pint = (*ps & 0xFF) | 0x1000;
    
    /* Prevent dead code elimination */
    *pc = (char)(i >> 16);
}

/* Function C: Mixed patterns with ternary and complex expressions */
static void __attribute__((noinline))
pattern_c(volatile int *base, int selector, int idx) {
    /* Complex addressing with ternary operator */
    volatile int *ptr = selector ? 
        (base + idx * 2) : 
        (base + (idx & 0xF) * 3);
    
    /* Struct with bit-field at computed address */
    struct BF {
        volatile unsigned int field:7;
    };
    
    /* This may generate interesting patterns after optimization */
    int temp = *ptr;
    struct BF *bf_ptr = (struct BF*)&temp;
    
    /* Force potential ZERO_EXTRACT through volatile */
    volatile unsigned int dummy = bf_ptr->field;
    (void)dummy;
    
    /* More complex MEM with multiple indices */
    int arr2d[5][5];
    volatile int v2 = arr2d[idx % 5][selector % 5];
    (void)v2;
    
    /* Pointer arithmetic that may create complex MEM addresses */
    volatile int *p = base;
    for (int i = 0; i < 3; i++) {
        p += (selector >> i) & 1;
    }
    *p = idx;
}

/* Helper to ensure all patterns are used */
static volatile int global_counter = 0;

int main(int argc, char **argv) {
    /* Use argc to bound loops (prevents infinite loops in analysis) */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Volatile counters to prevent optimization */
    volatile int i, j;
    
    /* Arrays for MEM patterns */
    volatile int array1[100];
    volatile short array2[50];
    volatile char array3[200];
    
    /* Initialize with some values */
    for (i = 0; i < 100; i++) {
        array1[i] = i;
    }
    for (i = 0; i < 50; i++) {
        array2[i] = (short)(i * 2);
    }
    for (i = 0; i < 200; i++) {
        array3[i] = (char)(i & 0xFF);
    }
    
    /* Main loop combining all patterns */
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 3; j++) {
            /* Call pattern functions with volatile-derived arguments */
            pattern_a((int*)array1, i & 0xF, j & 0xF);
            pattern_b((short*)array2 + (i % 40), 
                     (char*)array3 + (j % 190));
            pattern_c((int*)array1, i, j);
            
            /* Update global volatile to prevent dead code elimination */
            global_counter += i + j;
        }
    }
    
    /* Final dummy operation using results */
    volatile int sum = 0;
    for (i = 0; i < 50; i++) {
        sum += array1[i] + array2[i % 50];
    }
    
    /* The program doesn't need correct runtime semantics,
       but this prevents it from being optimized away entirely */
    return (sum > 0) ? 0 : 1;
}
