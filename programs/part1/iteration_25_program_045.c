/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static volatile int volatile_source = 42;

/* Target function with complex data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_source;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW dependency (RAW): a[i] depends on b[i-1] from previous iteration */
        int temp = b[i-1] + get_value();  /* Cross-iteration flow dep */
        
        /* 2. ANTI dependency (WAR): c[i] read before being overwritten */
        int read_c = c[i];                /* Read c[i] */
        a[i] = temp + read_c;             /* Flow: depends on temp and read_c */
        
        /* 3. OUTPUT dependency (WAW): b[i] written twice */
        b[i] = a[i-1] * 2;                /* Flow: depends on a[i-1], Output: WAW with next line */
        b[i] = b[i] + get_value();        /* Output: WAW with previous, Anti: WAR with b[i-1] next iter */
        
        /* 4. Complex memory dependencies with variant indices */
        int idx = i % 10;
        c[idx] = d[i] + b[i];             /* Flow: depends on d[i] and b[i] */
        
        /* 5. Another flow dependency chain */
        d[i] = c[idx] + a[i];             /* Flow: depends on c[idx] and a[i] */
        
        /* 6. Anti dependency through array */
        int old_d = d[i-1];               /* Read d[i-1] */
        d[i-1] = get_value();             /* Anti: WAR on d[i-1] */
        
        /* 7. Output dependency through scalar */
        static int scalar = 0;
        scalar = old_d + i;               /* Output: WAW with next iteration */
    }
}

/* Helper to prevent optimization */
static void use_result(int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    printf("Checksum: %d\n", sum);
}

int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
    
    /* Process the loop - this should trigger DDG construction */
    process_loop(a, b, c, d, n);
    
    /* Use results to prevent dead code elimination */
    use_result(a, n);
    use_result(b, n);
    use_result(c, n);
    use_result(d, n);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
