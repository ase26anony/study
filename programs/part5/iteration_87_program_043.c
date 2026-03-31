/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static inline int simple_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7fffffff;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, short *c, char *d, int size, int seed) {
    int val = seed;
    for (int i = 0; i < size; i++) {
        val = simple_rand(val);
        a[i] = (val % 100) - 50;
        b[i] = (val % 200) - 100;
        c[i] = (short)(val % 256);
        d[i] = (char)(val % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int size) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod = 1;
    short max_val = 0;
    char min_val = 127;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < size; ++i) {
        /* Chain: use -> modify -> use pattern */
        sum1 = (sum1 * a[i]) + b[i];
        /* Independent operation in same iteration */
        prod = prod & (a[i] | 1);
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    int threshold = size / 2;
    for (int i = 0; i < size; i++) {
        /* Mix int and short types */
        short val = c[i];
        if (val > threshold) {
            /* Multiple uses of val in dependency chain */
            sum2 += val * (i & 0xF);
            /* Another dependent operation */
            sum2 = sum2 ^ (val >> 2);
        }
        /* Always execute path */
        sum2 = sum2 - (i % 8);
    }
    
    /* Loop 3: Search with data-dependent exit */
    int limit = size - 1;
    for (int i = 0; i < limit; i++) {
        /* Complex dependency chain */
        int diff = a[i + 1] - a[i];
        sum3 = (sum3 + diff) * 3;
        
        /* Conditional update based on computation */
        if (diff > 0) {
            max_val = (c[i] > max_val) ? c[i] : max_val;
        } else {
            min_val = (d[i] < min_val) ? d[i] : min_val;
        }
        
        /* Another dependent operation */
        sum3 = sum3 ^ (b[i] & 0xFF);
    }
    
    /* Loop 4: Reduction with alternating operations */
    int acc = 0;
    for (int i = 1; i < size; i += 2) {
        /* Alternating dependency patterns */
        if (i & 2) {
            acc = acc + a[i] * 2;
        } else {
            acc = acc | (b[i] & 0xFFFF);
        }
        /* Chain continues */
        acc = acc - (c[i] % 16);
    }
    
    /* Combine all results */
    int result = sum1 + sum2 + sum3 + prod + acc + max_val - min_val;
    return result;
}

int main(int argc, char **argv) {
    /* Use argc to prevent constant propagation */
    int size = SIZE + (argc > 1 ? 0 : 0);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    short *c = (short*)malloc(size * sizeof(short));
    char *d = (char*)malloc(size * sizeof(char));
    
    if (!a || !b || !c || !d) {
        return 1;
    }
    
    init_arrays(a, b, c, d, size, 42);
    
    /* Call work function with multiple scheduling opportunities */
    int result = work(a, b, c, d, size);
    
    /* Prevent dead code elimination */
    volatile int sink = result;
    
    /* Use result to prevent optimization */
    if (sink != 0xDEADBEEF) {  /* Arbitrary constant */
        printf("Result: %d\n", sink);
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
