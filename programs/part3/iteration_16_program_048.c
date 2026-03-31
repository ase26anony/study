#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int mod_val = 5;

/* Helper function to prevent optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod_val;
    int v5 = n % 7;
    int v6 = n % 11;
    int v7 = n % 13;
    int v8 = n % 17;
    int v9 = n % 19;
    int v10 = n % 23;
    int v11 = n % 29;
    int v12 = n % 31;
    int v13 = n % 37;
    int v14 = n % 41;
    int v15 = n % 43;
    int v16 = n % 47;
    int v17 = n % 53;
    int v18 = n % 59;
    int v19 = n % 61;
    int v20 = n % 67;
    
    int sum = 0;
    
    /* Loop with complex addressing in both loads and stores */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + v5 - v6 + v7 * v8) / v3;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + 
                  arr[i-1].y * v4 + 
                  arr[(i + v9) % n].z;
        
        /* More complex calculations keeping variables live */
        val = val * v10 + v11 - v12 * v13 + v14 / (v15 + 1);
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val + v16 * v17 - v18;
        arr[i].x[1] = val * v19 + v20;
        
        /* Different complex store */
        int idx2 = (i * v2 + v1) / (v3 + 1);
        if (idx2 < 0) idx2 = -idx2;
        idx2 = idx2 % n;
        
        /* Store with complex addressing */
        arr[idx2].y = val + i;
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and related types */
        asm volatile("# Complex address 1: %0" 
                     : 
                     : "m" (arr[(i * v1 + v2) / v3].x[0]));
        
        asm volatile("# Complex address 2: %0" 
                     : 
                     : "m" (arr[idx].y));
        
        asm volatile("# Complex address 3: %0" 
                     : 
                     : "m" (arr[i].x[(v4 + i) % 4]));
        
        /* Keep sum for checksum */
        sum += arr[i].x[0] + arr[i].x[1] + arr[idx].y;
        
        /* Update some variables to prevent dead code elimination */
        v5 = (v5 + i) % 13;
        v6 = (v6 * 3 + 1) % 17;
        v7 = (v7 + v8) % 19;
        v8 = (v8 * 5 + 7) % 23;
    }
    
    return sum;
}

/* Another function with different addressing patterns */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z = (i * 29 + 11) % 100;
    }
}

int main() {
    /* Non-constant loop bound */
    volatile int N = SIZE;
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    init_array(arr, N);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, N);
    
    /* Additional complex operations to increase pressure */
    {
        /* More variables to consume registers */
        int t1 = result % 101;
        int t2 = result % 103;
        int t3 = result % 107;
        int t4 = result % 109;
        int t5 = result % 113;
        
        /* Complex addressing in output context */
        for (int i = 0; i < 10; ++i) {
            int idx = (i * t1 + t2) / (t3 + 1);
            if (idx < 0 || idx >= N) idx = 0;
            
            /* Output address reload */
            arr[idx].z = t4 * t5 + i;
            
            /* Input address reload with different pattern */
            t4 += arr[(i * t2 + t3) % N].x[i % 4];
        }
    }
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3];
        checksum += arr[i].y + arr[i].z;
    }
    
    checksum += result;
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
