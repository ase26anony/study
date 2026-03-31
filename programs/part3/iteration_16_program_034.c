#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(int n, volatile int* params, int* checksum);

/* Struct with multiple fields to create complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Global volatile variables to prevent constant propagation */
volatile int g_stride = 3;
volatile int g_offset = 7;
volatile int g_scale = 2;
volatile int g_mod = 11;

int main(void) {
    const int N = 1024;
    struct Data arr[N];
    
    /* Initialize array with deterministic pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 17 + j * 3) % 100;
        }
        arr[i].y = (i * 23) % 100;
        arr[i].z[0] = (i * 29) % 100;
        arr[i].z[1] = (i * 31) % 100;
    }
    
    /* Volatile parameters to force runtime calculations */
    volatile int params[8];
    for (int i = 0; i < 8; i++) {
        params[i] = (i * 13 + 5) % 20 + 1;
    }
    
    int checksum = 0;
    
    /* Call the function that triggers complex reloads */
    complex_access_loop(N, (int*)params, &checksum);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

__attribute__((noinline, noipa))
void complex_access_loop(int n, volatile int* params, int* checksum) {
    struct Data* arr = malloc(n * sizeof(struct Data));
    
    /* Initialize local array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 19 + j * 7) % 100;
        }
        arr[i].y = (i * 41) % 100;
        arr[i].z[0] = (i * 43) % 100;
        arr[i].z[1] = (i * 47) % 100;
    }
    
    /* Many local variables to create register pressure */
    int v1 = params[0];
    int v2 = params[1];
    int v3 = params[2];
    int v4 = params[3];
    int v5 = params[4];
    int v6 = params[5];
    int v7 = params[6];
    int v8 = params[7];
    int v9 = v1 + v2;
    int v10 = v3 * v4;
    int v11 = v5 - v6;
    int v12 = v7 / (v8 ? v8 : 1);
    int v13 = v9 * v10;
    int v14 = v11 + v12;
    int v15 = v13 - v14;
    int v16 = v1 * v3;
    int v17 = v2 * v4;
    int v18 = v5 * v6;
    int v19 = v7 * v8;
    int v20 = v9 + v10;
    int v21 = v11 - v12;
    int v22 = v13 * v14;
    int v23 = v15 + v16;
    int v24 = v17 - v18;
    int v25 = v19 * v20;
    
    /* Use all variables in the loop to keep them live */
    for (int i = 1; i < n - 1; ++i) {
        /* Complex index calculation using many variables */
        int idx = ((i * v1 + v2) * v3 + v4) / (v5 ? v5 : 1);
        idx = (idx + v6 * v7 - v8) % (v9 ? v9 : 1);
        idx = (idx * v10 + v11) & (n - 1);
        
        /* Complex addressing for LOAD (input address reloads) */
        int field_idx = (i * v12 + v13) % 4;
        int val1 = arr[idx].x[field_idx];
        int val2 = arr[i - 1].y;
        int val3 = arr[(i * v14 + v15) % n].z[i % 2];
        
        /* Use many variables in computation */
        int compute = val1 * v16 + val2 * v17 - val3 * v18;
        compute += v19 * v20 - v21 * v22 + v23 * v24;
        
        /* Complex addressing for STORE (output address reloads) */
        int store_idx = ((i * v25 + v1) * v2 + v3) % n;
        int store_field = (i * v4 + v5) % 4;
        
        /* Store with complex addressing - triggers output address reloads */
        arr[store_idx].x[store_field] = compute;
        
        /* Different complex store */
        int store_idx2 = (i * v6 + v7 * v8) % n;
        arr[store_idx2].y = compute * v9 + v10;
        
        /* More complex stores */
        int store_idx3 = ((i * v11 + v12) * v13 + v14) % n;
        arr[store_idx3].z[0] = compute * v15 - v16;
        
        int store_idx4 = (i * v17 + v18 * v19) % n;
        arr[store_idx4].z[1] = compute * v20 + v21;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address 1: %0" 
                     : 
                     : "m" (arr[((i * v22 + v23) * v24 + v25) % n].x[(i * v1) % 4]));
        
        asm volatile("# Complex address 2: %0" 
                     : 
                     : "m" (arr[(i * v2 + v3 * v4) % n].y));
        
        asm volatile("# Complex address 3: %0" 
                     : 
                     : "m" (arr[((i * v5 + v6) * v7 + v8) % n].z[i % 2]));
        
        /* Mix in some arithmetic with addresses */
        int* addr1 = &arr[(i * v9 + v10) % n].x[0];
        int* addr2 = &arr[(i * v11 + v12) % n].y;
        int* addr3 = &arr[(i * v13 + v14) % n].z[0];
        
        /* Use addresses in computations */
        int diff = addr2 - addr1;
        int sum = (int)(addr3) + diff;
        
        /* More stores using pointer arithmetic */
        *(addr1 + (i * v15) % 3) = sum * v16 + v17;
        *(addr2 - (i * v18) % 2) = sum * v19 - v20;
        addr3[(i * v21) % 2] = sum * v22 + v23;
        
        /* Update checksum using all variables */
        *checksum += val1 + val2 + val3 + compute;
        *checksum += store_idx + store_field;
        *checksum += store_idx2 + store_idx3 + store_idx4;
        *checksum += diff + sum;
        *checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        *checksum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        *checksum += v21 + v22 + v23 + v24 + v25;
        
        /* Rotate variables to create dependencies */
        int tmp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = v16;
        v16 = v17; v17 = v18; v18 = v19; v19 = v20; v20 = v21;
        v21 = v22; v22 = v23; v23 = v24; v24 = v25; v25 = tmp;
    }
    
    free(arr);
}
