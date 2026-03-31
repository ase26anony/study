/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, char *c, short *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (char)(lcg_rand() % 256);
        d[i] = (short)(lcg_rand() % 1000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int size) {
    int result = 0;
    int temp1 = 1, temp2 = 0;
    int threshold = 500;
    char char_threshold = 100;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    for (int i = 0; i < size; ++i) {
        temp1 = (temp1 * a[i]) + b[i];
        result ^= temp1;  /* Use result to create cross-iteration dependency */
    }
    
    /* Loop 2: Conditional accumulation with mixed types */
    for (int i = 0; i < size; i++) {
        if (a[i] > threshold) {
            /* Mixed-type operations create promotion/demotion instructions */
            short scaled = (short)(a[i] * 3);
            temp2 += scaled + (int)c[i];
        } else {
            temp2 -= b[i] & 0xFF;  /* Bitwise operation for varied pattern */
        }
    }
    result += temp2;
    
    /* Loop 3: Search with early exit possibility */
    int found_index = -1;
    for (int i = 0; i < size; i++) {
        /* Complex condition with multiple dependencies */
        if ((d[i] > 200) && (c[i] < char_threshold) && (a[i] % 2 == 0)) {
            found_index = i;
            break;  /* Creates control flow for scheduler */
        }
    }
    if (found_index != -1) {
        result ^= a[found_index] * b[found_index];
    }
    
    /* Loop 4: Transformation with pointer arithmetic */
    int *ptr_a = a;
    int *ptr_b = b;
    for (int i = 0; i < size; i++) {
        /* Multiple independent operations that could be scheduled in parallel */
        int val1 = *ptr_a++;
        int val2 = *ptr_b++;
        int prod = val1 * val2;
        int diff = val1 - val2;
        result += (prod & 0xFFFF) | (diff << 16);
    }
    
    /* Loop 5: Reduction with nested dependency chain */
    int chain = result;
    for (int i = 0; i < size; i++) {
        chain = (chain << 3) | (chain >> 29);  /* Rotate */
        chain += d[i] * c[i];
        chain ^= a[i] + b[i];
    }
    result = chain;
    
    return result;
}

int main(int argc, char **argv) {
    /* Use command line or fixed size to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 256;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    char *c = (char*)malloc(size * sizeof(char));
    short *d = (short*)malloc(size * sizeof(short));
    
    if (!a || !b || !c || !d) {
        return 1;
    }
    
    init_arrays(a, b, c, d, size);
    
    /* Call work function with multiple scheduling opportunities */
    int result = work(a, b, c, d, size);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
