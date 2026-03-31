/* Test program to trigger modulo scheduling edge printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 100

/* Global volatile to prevent optimization */
volatile int global_sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int* arr, int n) {
    int sum = arr[0];
    for (int i = 1; i < n; i++) {
        /* Mixed integer operations with carried dependency */
        int temp = arr[i-1] * 3 + 7;
        arr[i] = temp ^ (arr[i] + i);
        sum += arr[i];
        
        /* Fake dependency to prevent optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 2: Floating-point loop with recurrence */
float func2_fp_recurrence(float* arr, int n) {
    float acc = arr[0];
    for (int i = 1; i < n; i++) {
        /* Floating-point operations with different latencies */
        float t1 = arr[i-1] * 1.5f;
        float t2 = t1 + arr[i] * 2.0f;
        arr[i] = t2 - 0.5f;
        acc += arr[i];
        
        /* Memory barrier */
        asm volatile("" : "+r"(acc) : : "memory");
    }
    return acc;
}

/* Function 3: Nested loops with conditional inner logic */
int func3_nested_conditional(int* data, int n, int flag) {
    int total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        /* Conditional inner loop */
        if (flag || outer % 2 == 0) {
            int local_acc = data[0];
            for (int i = 1; i < n; i++) {
                /* Complex dependency chain */
                int val1 = data[i-1] << 2;
                int val2 = val1 | (data[i] & 0xFF);
                int val3 = val2 * (i + 1);
                data[i] = val3 % 1024;
                local_acc ^= data[i];
                
                /* Mix in some floating point */
                float ftemp = (float)val3 * 0.1f;
                if (ftemp > 100.0f) {
                    local_acc += (int)ftemp;
                }
            }
            total += local_acc;
        }
    }
    return total;
}

/* Function 4: Pointer chasing pattern */
int func4_pointer_chase(int* base, int n) {
    int* p = base;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *p;
        /* Pointer chase with stride */
        p = base + ((*p) % (n-1) + 1);
        
        /* Prevent optimization */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    return sum;
}

/* Function 5: Multiple independent statements with final dependency */
void func5_multi_stmts(int* a, int* b, int* c, int n) {
    for (int i = 1; i < n; i++) {
        /* Independent computations */
        int t1 = a[i] * 3;
        int t2 = b[i] + 7;
        int t3 = c[i] ^ 0xAA;
        
        /* Final dependent store with distance 1 */
        a[i] = (a[i-1] + t1 + t2 + t3) & 0xFF;
        b[i] = (b[i-1] * t1) >> 2;
        
        /* Use volatile to preserve order */
        volatile int dummy = t1 + t2;
        (void)dummy;
    }
}

int main() {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    int arr4[SIZE];
    int arr5a[SIZE], arr5b[SIZE], arr5c[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + 7) % 100;
        arr2[i] = (float)(i * 2 + 1) * 0.5f;
        arr3[i] = (i * 5 + 13) % 256;
        arr4[i] = (i + 1) % (SIZE - 1);
        arr5a[i] = i;
        arr5b[i] = i * 2;
        arr5c[i] = i * 3;
    }
    
    /* Execute all functions to ensure they're compiled */
    int result1 = func1_carried_dep((int*)arr1, ITERS);
    float result2 = func2_fp_recurrence(arr2, ITERS);
    int result3 = func3_nested_conditional(arr3, ITERS, 1);
    int result4 = func4_pointer_chase(arr4, ITERS);
    func5_multi_stmts(arr5a, arr5b, arr5c, ITERS);
    
    /* Combine results to prevent dead code elimination */
    global_sink = result1 + (int)result2 + result3 + result4 
                  + arr5a[ITERS-1] + arr5b[ITERS-1];
    
    /* Print minimal output */
    printf("Result: %d\n", global_sink);
    
    return 0;
}
