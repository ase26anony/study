#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

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

/* Helper function that won't be optimized away */
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
        int idx = (i * v1 + v2) / v3;
        idx = (idx + v5 + v6 + v7) % (n - 1);
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        /* Complex addressing for LOAD (input address reloads) */
        /* Mixing array index, struct field, and runtime calculations */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(i * v8 + v9) % n].z;
        val += arr[(val * v10 + v11) % n].x[2];
        
        /* More complex calculations keeping variables live */
        int tmp1 = val * v12 + v13;
        int tmp2 = tmp1 / (v14 + 1);
        int tmp3 = tmp2 % (v15 + 2);
        int tmp4 = tmp3 * v16 + v17;
        
        /* Complex addressing for STORE (output address reloads) */
        /* Different complex address for store operation */
        int store_idx = (i * v18 + v19) % n;
        arr[store_idx].x[i % 4] = val * v4 + v20 + tmp4;
        
        /* Force address reloads with inline assembly */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[store_idx].x[0]));
        
        /* Use all variables to keep them live */
        sum += val + tmp1 + tmp2 + tmp3 + tmp4;
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        sum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    }
    
    return sum;
}

/* Another function to trigger different reload contexts */
__attribute__((noinline, noipa))
void mixed_address_operations(struct Data *arr, int n) {
    volatile int a = 2, b = 3, c = 4, d = 5;
    
    for (int i = 0; i < n - 1; i++) {
        /* Complex addressing that might need RELOAD_FOR_INPADDR_ADDRESS */
        int idx1 = (i * a + b) / c;
        int idx2 = (i * b + c) / d;
        
        /* Operations that use addresses as part of larger expressions */
        arr[i].z = arr[idx1].x[0] * arr[idx2].y + arr[i].x[1];
        
        /* More inline assembly with memory operands */
        asm volatile("# Input address: %0" : : "m" (arr[(i * c + d) % n].x[2]));
        asm volatile("# Output address: %0" : : "m" (arr[(i * d + a) % n].y));
    }
}

int main() {
    /* Non-constant loop bound */
    volatile int N = ARRAY_SIZE;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(sizeof(struct Data) * N);
    if (!arr) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 200;
        arr[i].z = (i * 17) % 300;
    }
    
    /* Call functions that trigger complex reloads */
    int sum1 = complex_access_loop(arr, N);
    mixed_address_operations(arr, N);
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z;
        checksum %= 1000000;
    }
    checksum += sum1;
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
