/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_trigger = 1;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold)) 
int compute_checksum(int* a, int* b, int* c, int N, int M) {
    int s = 0;
    int checksum = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Path A: More arithmetic operations */
            for (int k = 0; k < 5; ++k) {
                s += k * g_volatile_trigger;
            }
        } else if (s < -500) {
            s = 100;
            /* Path B: Different operations */
            for (int k = 0; k < 3; ++k) {
                s -= k * g_volatile_counter;
            }
        } else {
            /* Path C: Default path with switch statement */
            switch (i % 4) {
                case 0:
                    s *= 2;
                    break;
                case 1:
                    s /= 2;
                    break;
                case 2:
                    s += g_volatile_counter;
                    break;
                case 3:
                    s -= g_volatile_counter;
                    break;
            }
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with more operations */
        for (int j = 0; j < M; ++j) {
            c[j] += s * j;
            
            /* More conditional logic */
            if (c[j] > 10000) {
                c[j] = c[j] % 1000;
                g_volatile_counter++;
            }
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Update checksum */
        checksum += s;
        
        /* Use volatile variable in condition */
        if (g_volatile_trigger) {
            checksum += i;
        }
    }
    
    return checksum;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int process_arrays(int* arr1, int* arr2, int size) {
    int result = 0;
    volatile int v_mod = 7;
    
    for (int i = 0; i < size; ++i) {
        /* Complex dependency chain */
        int temp = arr1[i];
        
        /* Nested conditionals */
        if (i % 2 == 0) {
            temp = temp * temp;
            if (temp % v_mod == 0) {
                temp = temp / v_mod;
            } else {
                temp = temp % v_mod;
            }
        } else {
            temp = temp + arr2[i];
            if (temp > 1000) {
                temp = temp - 1000;
            }
        }
        
        /* More arithmetic with volatile */
        temp = temp * g_volatile_trigger;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        arr2[i] = temp;
        result += temp;
    }
    
    return result;
}

int main() {
    const int N = 100;
    const int M = 50;
    const int SIZE = 200;
    
    /* Initialize arrays with pseudo-random values */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(M * sizeof(int));
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    
    /* Simple deterministic random generator */
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    /* Call functions with complex loop nests */
    int checksum1 = compute_checksum(a, b, c, N, M);
    int checksum2 = process_arrays(arr1, arr2, SIZE);
    
    /* Final computation to ensure no optimization */
    int final_result = checksum1 + checksum2;
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < M; ++i) {
        final_result += c[i];
    }
    
    for (int i = 0; i < SIZE; ++i) {
        final_result += arr2[i];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    
    return 0;
}
