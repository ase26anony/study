#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void process_direct(int *p) {
    /* Candidate 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;           /* Modifies the test expression */
        *p += 2;           /* Additional modification */
    }
}

/* Function with aliasing possibilities */
void process_aliased(int *arr, int i, int j) {
    /* Candidate 2: Potential aliasing */
    if (arr[i] > 10) {
        arr[j] = 0;        /* May alias if i == j */
        arr[i] = arr[i] * 2; /* Definitely modifies test expression */
    }
}

/* Function with mixed types and conversions */
void process_mixed(volatile int *vi, float *f) {
    /* Candidate 3: Mixed types with volatile */
    if ((float)(*vi) > 0.5f) {
        *vi = (int)(*f * 2.0f);  /* Modifies volatile test variable */
        *vi = *vi & 0xFF;        /* Additional operation */
    }
}

/* Function with loop-dependent condition */
void process_array(int *arr, int n, int threshold) {
    /* Candidate 4: Loop with array modification */
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = 0;          /* Modifies test expression */
            arr[i] = threshold;  /* Second modification */
        }
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *a) {
    /* Candidate 5: Atomic variable test */
    int val = atomic_load(a);
    if (val > 100) {
        atomic_store(a, 50);     /* Modifies atomic test variable */
        atomic_fetch_add(a, 1);  /* Additional atomic modification */
    }
}

/* Function with multiple test variables */
void process_multiple(int *a, int *b, volatile int *c) {
    /* Candidate 6: Complex condition */
    if (*a > *b && *c < 100) {
        *a = *b;                 /* Modifies first part of test */
        *c = *c + 1;             /* Modifies second part of test */
        *a = *a * 2;             /* Additional modification */
    }
}

/* Function with struct member access */
struct Data {
    int x;
    volatile int y;
    int *ptr;
};

void process_struct(struct Data *d) {
    /* Candidate 7: Struct member test */
    if (d->x > d->y) {
        d->x = d->y;             /* Modifies test expression member */
        d->y = d->x + 1;         /* Modifies other member */
    }
}

/* Function with pointer arithmetic */
void process_pointer_arithmetic(int *base, int offset) {
    /* Candidate 8: Pointer-based test */
    int *p = base + offset;
    if (*p > 0) {
        *p = 0;                  /* Modifies test expression */
        *(p + 1) = *p;           /* May alias with next iteration */
        *p = *p | 0x1;           /* Additional modification */
    }
}

int main() {
    /* Initialize test data */
    volatile int v1 = 42;
    volatile int v2 = 100;
    int arr1[100];
    int arr2[50];
    _Atomic int atomic_val = 200;
    struct Data data = {50, 75, &v1};
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr1[i] = i;
        if (i < 50) arr2[i] = i * 2;
    }
    
    float fval = 3.14f;
    int sum = 0;
    
    /* Test 1: Direct modification with volatile */
    process_direct((int*)&v1);
    
    /* Test 2: Aliasing case (i == j) */
    process_aliased(arr1, 10, 10);  /* i == j causes aliasing */
    process_aliased(arr1, 20, 30);  /* i != j for contrast */
    
    /* Test 3: Mixed types with volatile */
    process_mixed(&v2, &fval);
    
    /* Test 4: Loop with array modification */
    process_array(arr1, 100, 50);
    
    /* Test 5: Atomic operations */
    process_atomic(&atomic_val);
    
    /* Test 6: Multiple test variables */
    int x = 60, y = 40;
    process_multiple(&x, &y, &v1);
    
    /* Test 7: Struct access */
    process_struct(&data);
    
    /* Test 8: Pointer arithmetic */
    process_pointer_arithmetic(arr2, 5);
    
    /* Additional inline test cases */
    
    /* Inline Candidate 1: Short then block with modification */
    int local = 100;
    if (local > 50) {
        local = 25;              /* Modifies test variable */
        local = local + 10;      /* Additional operation */
    }
    
    /* Inline Candidate 2: Nested conditions */
    int a = 10, b = 20, c = 30;
    if (a < b) {
        if (b < c) {
            a = b;               /* Modifies outer condition variable */
            b = c;               /* Modifies inner condition variable */
        }
    }
    
    /* Inline Candidate 3: Complex expression modification */
    int *ptr1 = &a;
    int *ptr2 = &b;
    if (*ptr1 + *ptr2 > 30) {
        *ptr1 = 0;               /* Modifies part of test expression */
        *ptr2 = *ptr1;           /* Modifies other part */
    }
    
    /* Inline Candidate 4: Loop with break condition modification */
    int counter = 0;
    for (int i = 0; i < 10; i++) {
        if (counter > 5) {
            counter = 0;         /* Modifies loop condition variable */
            break;
        }
        counter++;
    }
    
    /* Use results to prevent dead code elimination */
    sum = v1 + v2 + atomic_val + local + a + b + c + counter + data.x + data.y;
    for (int i = 0; i < 10; i++) {
        sum += arr1[i] + arr2[i % 50];
    }
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
