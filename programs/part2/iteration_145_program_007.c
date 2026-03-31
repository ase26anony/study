#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void process_pointer(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = *p * 2;      /* Modifies test expression */
        *q = *p + 1;      /* Additional instruction */
    }
}

/* Function with volatile test variable */
void process_volatile(volatile int *vp) {
    /* Pattern 2: Volatile prevents optimization */
    if (*vp > 100) {
        *vp = 50;         /* Modifies volatile test expression */
        int temp = *vp;   /* Additional non-label instruction */
        (void)temp;
    }
}

/* Function with array aliasing possibilities */
void process_array(int arr[], int n, int idx1, int idx2) {
    /* Pattern 3: Potential aliasing in array access */
    for (int i = 0; i < n; i++) {
        if (arr[idx1] > arr[idx2]) {
            arr[idx1] = arr[idx2] - 1;  /* May modify test expression if idx1 == idx2 */
            arr[i] = arr[idx1] * 2;     /* Additional instruction */
        }
    }
}

/* Function with mixed data types */
void process_mixed_types(float *fp, int *ip) {
    /* Pattern 4: Implicit conversions and type mixing */
    if ((float)*ip > 0.5f) {
        *ip = (int)(*fp * 2.0f);  /* Modifies integer part of test expression */
        *fp = (float)*ip / 2.0f;  /* Additional instruction */
    }
}

/* Function with multiple modifications in then block */
void process_multiple_mods(int *a, int *b, int *c) {
    /* Pattern 5: Multiple instructions modifying related values */
    if (*a > *b) {
        *a = *b + *c;      /* Modifies test variable a */
        *b = *a - *c;      /* Modifies test variable b */
        *c = *a + *b;      /* Additional instruction - 3 total non-label insns */
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *atomic_var) {
    /* Pattern 6: Atomic operations prevent certain optimizations */
    int val = atomic_load(atomic_var);
    if (val > 0) {
        atomic_store(atomic_var, val / 2);  /* Modifies atomic test variable */
        int temp = val * 2;                  /* Additional instruction */
        (void)temp;
    }
}

int main() {
    /* Initialize test data with various qualifiers */
    volatile int v1 = 150;
    volatile int v2 = 75;
    int arr[10] = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
    float farr[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
    _Atomic int atomic_val = 42;
    
    int x = 10, y = 20, z = 30;
    int *ptr1 = &x;
    int *ptr2 = &y;
    
    /* Test 1: Direct pointer modification */
    process_pointer(&x, &y);
    
    /* Test 2: Volatile modification */
    process_volatile(&v1);
    
    /* Test 3: Array processing with potential aliasing */
    /* idx1 == idx2 case to trigger modification of test expression */
    process_array(arr, 10, 3, 3);  /* Same index ensures modification */
    
    /* Test 4: Mixed types */
    process_mixed_types(&farr[2], &arr[4]);
    
    /* Test 5: Multiple modifications */
    int a = 100, b = 50, c = 25;
    process_multiple_mods(&a, &b, &c);
    
    /* Test 6: Atomic operations */
    process_atomic(&atomic_val);
    
    /* Test 7: Inline blocks with various patterns */
    for (int i = 0; i < 5; i++) {
        /* Loop-dependent condition with modification */
        if (arr[i] > 20) {
            arr[i] = arr[i] - 10;  /* Modifies test expression */
            int temp = arr[i] * 2;  /* Additional instruction */
            arr[i] = temp / 2;      /* Third instruction */
        }
        
        /* Another pattern with volatile in loop */
        if (v2 > 50) {
            v2 = v2 - 25;           /* Modifies volatile test expression */
            int dummy = v2;         /* Additional instruction */
            (void)dummy;
        }
    }
    
    /* Test 8: Nested pointers and aliasing */
    int *alias1 = &arr[0];
    int *alias2 = &arr[0];  /* Same address - definite aliasing */
    if (*alias1 > 0) {
        *alias2 = 0;        /* Definitely modifies test expression */
        *alias1 = *alias2 + 5;  /* Additional instruction */
    }
    
    /* Test 9: Complex expression with modification */
    int base = 100;
    int offset = 2;
    if (arr[base % 10] > 30) {
        arr[base % 10] = 25;        /* Modifies test expression */
        offset = arr[base % 10];    /* Additional instruction */
        base = offset * 2;          /* Third instruction */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = v1 + v2 + x + y + a + b + c + atomic_val + base + offset;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    for (int i = 0; i < 5; i++) {
        sum += (int)farr[i];
    }
    
    printf("Result: %d\n", sum);
    return sum > 1000 ? 0 : 1;
}
