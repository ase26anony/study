/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ 0x55AA;
}

/* Volatile read to prevent dead code elimination */
static volatile int volatile_source = 42;

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, int n) {
    int i;
    
    /* Initialize with volatile to prevent constant folding */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW (RAW) dependency within iteration */
        int temp = a[i] + init_val;      /* Read a[i] */
        b[i] = temp * 2;                 /* Write b[i] */
        
        /* 2. FLOW (RAW) dependency across iterations (distance=1) */
        /* Loop-carried: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] + get_value(i);    /* Read a[i-1], Write a[i] */
        
        /* 3. ANTI (WAR) dependency */
        int read_before_write = b[i-1];  /* Read b[i-1] */
        b[i-1] = read_before_write + c[i]; /* Write b[i-1] after read */
        
        /* 4. OUTPUT (WAW) dependency on 'c' */
        c[i] = temp + 1;                 /* Write c[i] - first write */
        c[i] = c[i] * 3;                 /* Write c[i] - second write (WAW) */
        
        /* 5. Additional FLOW dependency with memory aliasing */
        /* Use different indices to create complex memory dependencies */
        if (i > 2) {
            /* Flow dependency with distance=2 */
            a[i] += c[i-2];              /* Read c[i-2], Write a[i] */
        }
    }
}

/* Alternative: Loop with array of structures for more complex dependencies */
struct data_item {
    int value;
    int processed;
    int result;
};

void __attribute__((noinline))
process_struct_loop(struct data_item *items, int n) {
    int i;
    
    /* Initialize with volatile */
    int base = volatile_source & 0xFF;
    
    for (i = 1; i < n; i++) {
        /* Chain of dependencies */
        items[i].processed = items[i-1].value + base;  /* Flow, distance=1 */
        items[i].value = items[i].processed * 2;       /* Flow within iteration */
        items[i-1].result = items[i].value;            /* Anti dependency */
        items[i].result = items[i].value;              /* Output dependency */
        items[i].result = items[i].processed;          /* Second write (WAW) */
    }
}

/* Main function with runtime-determined loop bounds */
int main(int argc, char **argv) {
    int n = 1000;
    
    /* Use command line or environment for variable loop count */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
        if (n > 100000) n = 100000;  /* Reasonable limit */
    }
    
    /* Allocate arrays with alignment hint */
    int *a = (int*)__builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *b = (int*)__builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    int *c = (int*)__builtin_assume_aligned(malloc(n * sizeof(int)), 16);
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Call the target function multiple times to give optimizer more chances */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(a, b, c, n);
        
        /* Also call struct version */
        struct data_item *items = (struct data_item*)malloc(n * sizeof(struct data_item));
        if (items) {
            for (int i = 0; i < n; i++) {
                items[i].value = i + iter;
                items[i].processed = 0;
                items[i].result = 0;
            }
            process_struct_loop(items, n);
            
            /* Use results to prevent dead code elimination */
            volatile int checksum = 0;
            for (int i = 0; i < n; i++) {
                checksum += items[i].result;
            }
            free(items);
        }
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    volatile int final_checksum = 0;
    for (int i = 0; i < n; i++) {
        final_checksum += a[i] + b[i] + c[i];
    }
    
    /* Print something to ensure side effects */
    printf("Processed %d elements, checksum: %d\n", n, final_checksum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
