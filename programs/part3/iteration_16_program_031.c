#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Struct with multiple fields to create complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int mod1 = 5;
volatile int mod2 = 11;

/* Force no optimization on the complex access function */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod1;
    int v5 = mod2;
    int v6 = n * 2;
    int v7 = n / 3;
    int v8 = n + 100;
    int v9 = n - 50;
    int v10 = v1 * v2;
    int v11 = v3 + v4;
    int v12 = v5 - v6;
    int v13 = v7 * v8;
    int v14 = v9 / 2;
    int v15 = v10 + v11;
    int v16 = v12 * v13;
    int v17 = v14 + v15;
    int v18 = v16 - v17;
    int v19 = v18 * 3;
    int v20 = v19 / 7;
    
    int sum = 0;
    
    /* Loop with complex addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % v6;
        idx = (idx + v7) & (v8 - 1);
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(i * v9) % n].z;
        val += v10 + v11 - v12;
        
        /* More complex calculations keeping variables live */
        int tmp1 = v13 * v14 + v15;
        int tmp2 = v16 / (v17 + 1) + v18;
        int tmp3 = v19 - v20 * 2;
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val * tmp1 + tmp2;
        arr[i].x[1] = tmp3 * v1 + v2;
        arr[i].x[2] = (tmp1 + tmp2) * v3;
        arr[i].x[3] = tmp3 / (v4 + 1);
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address operand %0" : : "m" (arr[idx].y));
        asm volatile("# Another address %0" : : "m" (arr[i].x[(v5 + i) % 4]));
        
        /* More complex addressing in inline asm */
        asm volatile("# Third address %0" : : "m" (arr[(idx + v6) % n].z));
        
        /* Keep all variables live by using them */
        v1 = (v1 + 1) % 17;
        v2 = (v2 + v3) % 23;
        v3 = (v3 + v4) % 29;
        v4 = (v4 + v5) % 31;
        v5 = (v5 + v6) % 37;
        v6 = (v6 + 1) % 41;
        v7 = (v7 * 3) % 43;
        v8 = (v8 + v9) % 47;
        v9 = (v9 * 5) % 53;
        v10 = (v10 + v11) % 59;
        v11 = (v11 * 7) % 61;
        v12 = (v12 + v13) % 67;
        v13 = (v13 * 11) % 71;
        v14 = (v14 + v15) % 73;
        v15 = (v15 * 13) % 79;
        v16 = (v16 + v17) % 83;
        v17 = (v17 * 17) % 89;
        v18 = (v18 + v19) % 97;
        v19 = (v19 * 19) % 101;
        v20 = (v20 + 1) % 103;
        
        sum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3];
    }
    
    /* Use all variables one more time to ensure they stay live */
    sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 200;
        arr[i].z = (i * 17) % 300;
    }
}

int main() {
    /* Non-constant size to prevent optimization */
    volatile int N = SIZE;
    int actual_n = N;
    
    /* Allocate array dynamically to avoid static addressing */
    struct Data *arr = (struct Data*)malloc(actual_n * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize array */
    init_array(arr, actual_n);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, actual_n);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional complex operation to trigger more reloads */
    int checksum = 0;
    for (int i = 0; i < actual_n; i += 2) {
        /* Complex addressing in both load and store */
        int idx1 = (i * stride + offset) / scale;
        int idx2 = ((i + 1) * mod1 + mod2) % actual_n;
        
        if (idx1 >= 0 && idx1 < actual_n && idx2 >= 0 && idx2 < actual_n) {
            /* Mixed addressing modes */
            arr[i].y = arr[idx1].x[0] + arr[idx2].z;
            
            /* Inline asm with memory operand */
            asm volatile("# Final address check %0" : : "m" (arr[idx1].x[(i % 4)]));
        }
        checksum += arr[i].y;
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
