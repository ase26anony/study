/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

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

/* Target function with carefully constructed data dependencies */
void __attribute__((noinline)) 
process_loop(int *restrict a, int *restrict b, int *restrict c, 
             int *restrict d, int n) {
    int i;
    
    /* Initialize with volatile to prevent dead code elimination */
    int init_val = volatile_read();
    
    /* Pre-loop setup to create initial values */
    a[0] = init_val;
    b[0] = init_val + 1;
    
    /* Main loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* 1. FLOW DEPENDENCY (RAW) within iteration */
        int temp = a[i-1] + get_value();  /* Read a[i-1] */
        b[i] = temp * 2;                  /* Write b[i] */
        
        /* 2. ANTI DEPENDENCY (WAR) */
        int read_before = b[i];           /* Read b[i] just written */
        a[i] = read_before + c[i];        /* Write a[i] - WAR with next iteration's read of a[i-1] */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        d[i] = temp + i;                  /* First write to d[i] */
        d[i] = d[i] * 3;                  /* Second write to d[i] - WAW */
        
        /* 4. LOOP-CARRIED FLOW DEPENDENCY (distance > 0) */
        c[i] = c[i-1] + b[i];             /* Flow from iteration i-1 to i */
        
        /* 5. MEMORY DEPENDENCY with variant index */
        int idx = i % 10;
        a[idx] = b[idx] + get_value();    /* May create dependencies across iterations */
    }
    
    /* Cross-iteration anti dependency */
    for (i = 0; i < n-1; i++) {
        /* Read after write from previous loop */
        int val = a[i];
        /* Write that will be read in next iteration */
        b[i+1] = val + d[i];
    }
}

/* Another function with different patterns */
void __attribute__((noinline))
complex_dependencies(int *arr, int n) {
    int i;
    
    /* Multiple interleaved dependencies */
    for (i = 2; i < n; i++) {
        /* Chain of flow dependencies */
        int x = arr[i-2];
        int y = x + arr[i-1];
        int z = y * arr[i];
        
        /* Anti-dependency pattern */
        arr[i-1] = z + i;      /* Write arr[i-1] */
        int read_again = arr[i-1] + x;  /* Read arr[i-1] */
        
        /* Output dependency */
        arr[i] = read_again;
        arr[i] = arr[i] * 2;   /* Overwrite arr[i] */
        
        /* Another flow with distance 2 */
        arr[i] = arr[i] + arr[i-2];
    }
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use command line argument for variable loop bound */
    int n = 1000;
    if (argc > 1) {
        volatile int v = 0;
        for (int i = 0; argv[1][i]; i++) {
            v += argv[1][i];
        }
        n = 100 + (v % 900);  /* Non-constant trip count */
    }
    
    /* Allocate arrays with restrict to help alias analysis */
    int *a = __builtin_malloc(sizeof(int) * n);
    int *b = __builtin_malloc(sizeof(int) * n);
    int *c = __builtin_malloc(sizeof(int) * n);
    int *d = __builtin_malloc(sizeof(int) * n);
    
    if (!a || !b || !c || !d) return 1;
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
    }
    
    /* Call functions with dependency patterns */
    process_loop(a, b, c, d, n);
    complex_dependencies(a, n);
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    __builtin_free(a);
    __builtin_free(b);
    __builtin_free(c);
    __builtin_free(d);
    
    return checksum != 0;
}
