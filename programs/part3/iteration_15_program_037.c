/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* 1. TRUE (FLOW) DEPENDENCE: a[i] depends on a[i-1] from previous iteration */
        /*    Creates: RAW (Read-After-Write) dependency with distance 1 */
        int temp = a[i-1] * b[i];
        
        /* 2. OUTPUT DEPENDENCE: Multiple writes to a[i] in same iteration */
        /*    Creates: WAW (Write-After-Write) dependency */
        a[i] = temp + c[i];
        
        /* 3. ANTI-DEPENDENCE: b[i] written, then potentially read in next iteration */
        /*    Creates: WAR (Write-After-Read) dependency with distance 1 */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more complex DDG */
        /* 4. Another TRUE DEPENDENCE: c[i] modified based on previous b[i] */
        /*    Creates: RAW dependency with distance 1 */
        c[i] = b[i-1] + (i & 0xFF);
        
        /* 5. Output dependence on d[i] */
        d[i] = d[i-1] ^ (a[i] << 2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper function to initialize arrays */
void init_arrays(int *a, int *b, int *c, int *d, int n) {
    for (int i = 0; i < n; ++i) {
        /* Deterministic but non-constant initialization */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 73) % 997;
        c[i] = (i * 101) % 991;
        d[i] = (i * 137) % 983;
    }
}

int main(void) {
    /* Dynamically allocate arrays to avoid stack overflow */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    init_arrays(a, b, c, d, N);
    
    /* Execute the loop with dependencies */
    int result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum result: %d\n", result);
    
    /* Verify with simple calculation */
    int verify = a[N-1] + b[N-1] + c[N-1] + d[N-1];
    printf("Verification sum: %d\n", verify);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    
    return 0;
}
