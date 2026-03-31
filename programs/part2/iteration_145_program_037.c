#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void process_pointer(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;           /* Modifies the test expression */
        *q = *q + 1;       /* Additional instruction */
    }
}

/* Function with volatile test variable */
void process_volatile(volatile int *vp) {
    /* Pattern 2: Volatile prevents optimization */
    if (*vp > 10) {
        *vp = 5;           /* Modifies volatile test expression */
        *vp = *vp * 2;     /* Second modification */
    }
}

/* Function with array aliasing possibilities */
void process_array(int arr[], int i, int j) {
    /* Pattern 3: Potential aliasing through array indices */
    if (arr[i] > 100) {
        arr[j] = 50;       /* May alias if i == j */
        arr[i] = arr[i] / 2; /* Definitely modifies test expression */
    }
}

/* Function with mixed data types */
void process_mixed_types(float *fp, int *ip) {
    /* Pattern 4: Implicit conversions and type mixing */
    if ((float)*ip > 0.5f) {
        *ip = (int)(*fp * 2.0f);  /* Modifies integer part of test */
        *fp = *fp + 1.0f;         /* Additional float operation */
    }
}

/* Function with loop-dependent condition */
void process_loop(int data[], int n, int threshold) {
    /* Pattern 5: Loop with side-effect condition */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = 0;          /* Modifies test expression */
            data[i] = data[i] + 1; /* Second instruction in then block */
        }
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *atomic_var) {
    /* Pattern 6: Atomic operations prevent certain optimizations */
    int val = atomic_load(atomic_var);
    if (val > 0) {
        atomic_store(atomic_var, val - 1);  /* Modifies atomic variable */
        /* Additional non-atomic operation on local copy */
        val = val * 2;
    }
}

/* Main function creating various if-conversion scenarios */
int main() {
    /* Initialize test data */
    volatile int v1 = 15;
    volatile int v2 = 20;
    int arr1[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int arr2[10] = {0};
    float farr[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
    _Atomic int atomic_val = 10;
    
    int x = 10, y = 20, z = 30;
    int *p1 = &x;
    int *p2 = &y;
    
    /* Scenario 1: Direct pointer modification */
    process_pointer(&x, &y);
    
    /* Scenario 2: Volatile modification */
    process_volatile(&v1);
    
    /* Scenario 3: Array with potential aliasing (i == j case) */
    process_array(arr1, 3, 3);  /* i == j, definitely modifies test */
    process_array(arr1, 2, 4);  /* i != j, may still trigger analysis */
    
    /* Scenario 4: Mixed types */
    process_mixed_types(&farr[0], &z);
    
    /* Scenario 5: Loop with modifying condition */
    process_loop(arr2, 10, 25);
    
    /* Scenario 6: Atomic operations */
    process_atomic(&atomic_val);
    
    /* Additional inline patterns in main */
    
    /* Pattern A: Short then block with multiple modifications */
    int a = 42, b = 24;
    if (a > b) {
        a = b;          /* First modification */
        b = a + 1;      /* Second instruction */
        a = a - 1;      /* Third instruction - multiple non-label insns */
    }
    
    /* Pattern B: Nested condition with modification */
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        if (arr1[i] > 30) {
            arr1[i] = arr1[i] % 20;  /* Modifies test expression */
            counter++;                /* Additional instruction */
        }
    }
    
    /* Pattern C: Pointer arithmetic with modification */
    int *ptr = arr1;
    if (*(ptr + 2) > 50) {
        *(ptr + 2) = 25;             /* Modifies memory test expression */
        ptr[2] = ptr[2] * 2;         /* Second modification - same location */
    }
    
    /* Pattern D: Complex test expression with modification */
    int m = 10, n = 20, k = 30;
    if ((m + n) > k) {
        m = k - n;      /* Modifies part of test expression (m) */
        n = m + 5;      /* Additional modification */
    }
    
    /* Pattern E: Character array with byte operations */
    char str[] = "TestString";
    if (str[3] > 'a') {
        str[3] = 'z';   /* Modifies test expression */
        str[4] = 'x';   /* Additional modification */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = x + y + z + v1 + v2 + arr1[0] + arr2[0] + (int)farr[0] + 
              (int)atomic_val + a + b + counter + m + n + (int)str[3];
    
    printf("Result: %d\n", sum);
    
    return 0;
}
