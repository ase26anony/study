/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent constant propagation */
static int __attribute__((noinline)) get_value(void) {
    static int counter = 0;
    return ++counter;
}

/* Volatile read to prevent optimization */
static int volatile_read(void) {
    volatile int v = 42;
    return v;
}

/* Target function with complex loop carrying multiple dependency types */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with opaque values */
    int init_val = get_value();
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with carefully crafted dependencies */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + volatile_read();  /* Read a[i-1] */
        b[i] = temp * 2;                      /* Write b[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) within iteration */
        int read_before_write = b[i];         /* Read b[i] */
        b[i] = read_before_write + c[i];      /* Write b[i] again */
        
        /* 3. OUTPUT DEPENDENCY (WAW) within iteration */
        a[i] = temp + d[i];                   /* Write a[i] first */
        int volatile_val = volatile_read();   /* Opaque operation */
        a[i] = a[i] + volatile_val;           /* Write a[i] again */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance=1) */
        c[i] = c[i-1] + b[i];                 /* Flow from iteration i-1 to i */
        
        /* 5. LOOP-CARRIED ANTI DEPENDENCY */
        d[i] = d[i-1] * 2;                    /* Anti: read d[i-1], will be overwritten */
        if (i < n-1) {
            d[i] = get_value();               /* Overwrite d[i] in next iteration? */
        }
        
        /* 6. MEMORY DEPENDENCY with variant index */
        int idx = i % 10;
        a[idx] = b[idx] + 1;                  /* Potential aliasing creates memory dep */
    }
    
    /* Final output dependency to prevent dead code elimination */
    a[n-1] = b[n-1] + c[n-1];
}

/* Wrapper function to ensure loop is not unrolled completely */
void __attribute__((noinline))
process_data(int size) {
    /* Dynamically allocate to avoid stack overflow for large sizes */
    int *a = (int*)malloc(size * sizeof(int));
    int *b = (int*)malloc(size * sizeof(int));
    int *c = (int*)malloc(size * sizeof(int));
    int *d = (int*)malloc(size * sizeof(int));
    
    if (!a || !b || !c || !d) {
        free(a); free(b); free(c); free(d);
        return;
    }
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < size; i++) {
        a[i] = i * 3;
        b[i] = i * 5;
        c[i] = i * 7;
        d[i] = i * 11;
    }
    
    /* Call the loop processing function */
    process_loop(a, b, c, d, size);
    
    /* Use results to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Print something to ensure execution */
    if (checksum != 0) {
        printf("Processed %d elements, checksum influenced\n", size);
    }
    
    free(a); free(b); free(c); free(d);
}

int main(int argc, char **argv) {
    /* Use command line argument for loop bound to prevent constant folding */
    int loop_size = 1000;  /* Default size */
    
    if (argc > 1) {
        loop_size = atoi(argv[1]);
        if (loop_size <= 0) loop_size = 1000;
        if (loop_size > 100000) loop_size = 100000; /* Reasonable limit */
    }
    
    /* Also use volatile to hide trip count from compiler */
    volatile int hidden_size = loop_size;
    
    /* Process multiple times with different sizes to increase coverage */
    process_data(hidden_size);
    process_data(hidden_size / 2);
    process_data(hidden_size / 4);
    
    return 0;
}
