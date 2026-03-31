/* test_sel_sched_dump.c
 * Program designed to trigger selective scheduling RTL dumps in GCC
 * Specifically targets sel_print_insn_rtl function in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold)) 
int compute_checksum(int *a, int *b, int *c, volatile int *trigger) {
    int s = 0;
    int checksum = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operation in this path */
            *trigger = i;
        } else if (s < -LIMIT) {
            s = 0;
            /* Different operation in else-if path */
            *trigger = -i;
        } else {
            /* Third path with switch statement */
            switch (i % 4) {
                case 0:
                    s += *trigger;
                    break;
                case 1:
                    s -= *trigger;
                    break;
                case 2:
                    s *= 2;
                    break;
                default:
                    s /= 2;
                    break;
            }
        }
        
        /* Memory barrier to prevent optimization */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with more complex operations */
        for (int j = 0; j < M; ++j) {
            /* Variable index using volatile */
            int idx = (j + *trigger) % M;
            
            /* Complex update with multiple dependencies */
            c[idx] += s * j + a[i] - b[i];
            
            /* Another conditional inside inner loop */
            if (c[idx] > 1000) {
                c[idx] = c[idx] % 1000;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Accumulate checksum */
            checksum += c[idx];
        }
        
        /* More arithmetic to increase register pressure */
        s = (s * 3 + 7) % 100;
        __asm__ volatile("" : : : "memory");
    }
    
    return checksum;
}

/* Another complex function to ensure multiple scheduling opportunities */
__attribute__((noinline))
void process_array(int *arr, int size, volatile int *flag) {
    int temp = 0;
    for (int i = 0; i < size; ++i) {
        /* Complex indexing pattern */
        int idx = (i * 13 + 7) % size;
        
        /* Multiple operations with dependencies */
        temp = arr[idx] * 3 + temp;
        arr[idx] = temp;
        
        /* Conditional with side effect */
        if (temp % 5 == 0) {
            *flag = idx;
            temp = 0;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Nested loop with variable bound */
        for (int k = 0; k < (i % 8 + 1); ++k) {
            arr[(idx + k) % size] += k * temp;
        }
    }
}

int main(void) {
    /* Initialize with deterministic pseudo-random values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    
    /* Additional array for second function */
    int *arr = (int*)malloc(M * sizeof(int));
    
    /* Volatile variable to prevent optimization */
    volatile int trigger = 0;
    volatile int flag = 0;
    
    if (!a || !b || !c || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = 0;
        arr[i] = rand() % 500;
    }
    
    /* Call the complex function to trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, &trigger);
    
    /* Call another complex function */
    process_array(arr, M, &flag);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = checksum1;
    for (int i = 0; i < M; ++i) {
        final_checksum += arr[i];
    }
    
    /* Use volatile variables in computation */
    final_checksum += trigger + flag;
    
    printf("Result: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
