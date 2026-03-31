#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function prototypes
void process_direct(int *p);
void process_aliased(int *p, int *q);
int process_volatile(volatile int *vp);
void process_mixed_types(float *fp, int *ip);
void process_array(int arr[], int n, int threshold);

// Global volatile variable to prevent optimizations
volatile int global_volatile = 0;

int main() {
    int result = 0;
    
    // 1. Direct modification of test variable in then block
    int x = 10;
    if (x > 5) {
        x = x * 2;      // Modifies test variable
        x = x - 3;      // Another modification
        result += x;
    }
    
    // 2. Volatile variable modification
    volatile int v = 20;
    if (v > 15) {
        v = 5;          // Modifies volatile test variable
        v = v + 2;      // Another volatile modification
        result += v;
    }
    
    // 3. Pointer-based modification with potential aliasing
    int a = 30, b = 40;
    int *ptr1 = &a;
    int *ptr2 = &a;     // Same address - definite aliasing
    
    if (*ptr1 > 25) {
        *ptr2 = 15;     // Modifies memory accessed by test expression
        *ptr1 = *ptr1 + 1; // Another modification
        result += *ptr1;
    }
    
    // 4. Array access with loop-dependent condition
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 10;
    }
    
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 25) {
            arr[i] = arr[i] / 2;    // Modifies array element used in test
            arr[i] = arr[i] + 1;    // Another modification
            result += arr[i];
        }
    }
    
    // 5. Mixed data types with implicit conversions
    float f = 3.14f;
    int y = 5;
    if ((float)y > 2.5f) {
        y = (int)(f * 2.0f);    // Modifies variable involved in test
        y = y % 3;              // Another modification
        result += y;
    }
    
    // 6. Function calls with pointer arguments
    int val1 = 50;
    process_direct(&val1);
    result += val1;
    
    int val2 = 60, val3 = 70;
    process_aliased(&val2, &val2);  // Same pointer - aliasing
    result += val2;
    
    // 7. Volatile pointer in function
    volatile int vval = 80;
    result += process_volatile(&vval);
    
    // 8. Mixed types in function
    float fval = 90.5f;
    int ival = 100;
    process_mixed_types(&fval, &ival);
    result += ival;
    
    // 9. Process entire array with threshold
    int large_arr[20];
    for (int i = 0; i < 20; i++) {
        large_arr[i] = i * 5;
    }
    process_array(large_arr, 20, 30);
    for (int i = 0; i < 20; i++) {
        result += large_arr[i];
    }
    
    // 10. Complex condition with multiple modifications
    int counter = 0;
    for (int i = 0; i < 100; i++) {
        if (counter > 10 && counter < 90) {
            counter = counter * 2;      // Modifies test variable
            counter = counter % 100;    // Another modification
            counter = counter + 1;      // Third modification
        } else {
            counter = counter + 3;
        }
    }
    result += counter;
    
    // 11. Nested if with modification
    int n1 = 200, n2 = 300;
    if (n1 > 150) {
        if (n2 > 250) {
            n1 = n1 / 2;    // Modifies outer condition variable
            n2 = n2 - 50;   // Modifies inner condition variable
        }
        n1 = n1 + 10;       // Another modification
    }
    result += n1 + n2;
    
    // 12. Atomic operations (prevents certain optimizations)
    _Atomic int atomic_val = 500;
    if (atomic_val > 400) {
        atomic_val = 300;   // Atomic modification
        atomic_val = atomic_val + 50;
    }
    result += atomic_val;
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}

// Function that modifies test expression directly
void process_direct(int *p) {
    if (*p > 45) {
        *p = *p - 10;   // Modifies memory used in test
        *p = *p * 2;    // Another modification
    }
}

// Function with potential aliasing
void process_aliased(int *p, int *q) {
    if (*p > 55) {
        *q = 0;         // May modify *p if p and q alias
        *p = *p + 5;    // Direct modification
    }
}

// Function with volatile pointer
int process_volatile(volatile int *vp) {
    int temp = 0;
    if (*vp > 75) {
        *vp = 25;       // Modifies volatile memory
        *vp = *vp + 5;  // Another volatile modification
        temp = *vp;
    }
    return temp;
}

// Function with mixed types
void process_mixed_types(float *fp, int *ip) {
    if ((float)*ip > 95.0f) {
        *ip = (int)(*fp);   // Modifies integer used in float comparison
        *ip = *ip * 2;      // Another modification
    }
}

// Function processing array with threshold
void process_array(int arr[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = threshold;     // Modifies array element used in test
            arr[i] = arr[i] - 5;    // Another modification
        }
    }
}
