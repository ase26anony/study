#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Helper function to prevent optimization */
static void use(int x) {
    volatile int sink = x;
    (void)sink;
}

/* Function with pointer-based modification */
void process_with_alias(int *p, int *q) {
    /* Test expression uses *p, then block modifies *p */
    if (*p > 0) {
        *p = -1;           /* Direct modification of test expression */
        *q = *p + 1;       /* Potential aliasing if q == p */
    }
}

/* Function with array access and potential aliasing */
void process_array(int arr[], int n, int idx) {
    volatile int threshold = 10;
    
    /* Loop-dependent condition with modification */
    for (int i = 0; i < n; i++) {
        /* Test expression: arr[i] > threshold */
        if (arr[i] > threshold) {
            arr[i] = 0;                 /* Modifies test expression */
            arr[idx] = arr[i] + 5;      /* Potential aliasing if idx == i */
        }
    }
}

/* Function with mixed data types */
void mixed_types_test(volatile float *fp, int *ip) {
    float local_f = *fp;
    int local_i = *ip;
    
    /* Test with float, modification with int */
    if (local_f > 0.5f) {
        local_i = (int)(local_f * 2.0f);  /* Modifies variable used in conversion */
        *ip = local_i;                     /* Store back */
    }
}

/* Function with atomic operations */
void atomic_test(_Atomic int *atomic_var) {
    int expected = *atomic_var;
    
    /* Atomic variable in test expression */
    if (expected > 100) {
        /* Modify the atomic variable - different from test expression */
        atomic_fetch_add(atomic_var, -50);
    }
}

/* Complex scenario with multiple modifications */
int complex_condition(int *a, int *b, int len) {
    int sum = 0;
    volatile int v = 7;
    
    for (int i = 0; i < len; i++) {
        /* Multiple test variables */
        if (a[i] > v && b[i] < 100) {
            /* Multiple modifications in then block */
            a[i] = b[i];            /* Modifies a[i] used in test */
            b[i] = a[i] + v;        /* Modifies b[i] used in test */
            v = v - 1;              /* Modifies volatile test variable */
            sum += a[i];
        }
    }
    return sum;
}

/* Function with struct and pointer aliasing */
struct Data {
    int x;
    int y;
    volatile int flag;
};

void struct_test(struct Data *d1, struct Data *d2) {
    /* Test on struct member */
    if (d1->x > d2->y) {
        d1->x = d2->y;              /* Modifies test expression */
        d1->flag = 1;               /* Volatile write */
        
        /* Potential aliasing if d1 == d2 */
        d2->y = d1->x * 2;
    }
}

/* Main function creating various if-conversion candidates */
int main(void) {
    /* Setup test data */
    volatile int volatile_var = 42;
    int array1[10], array2[10];
    _Atomic int atomic_counter = 150;
    struct Data data1 = {50, 30, 0};
    struct Data data2 = {20, 40, 0};
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        array1[i] = i * 5;
        array2[i] = i * 3;
    }
    
    /* Test 1: Direct modification of test variable */
    int x = 15;
    if (x > 10) {
        x = x - 5;      /* Modifies test variable */
        x = x * 2;      /* Second modification */
        use(x);
    }
    
    /* Test 2: Volatile variable modification */
    if (volatile_var > 20) {
        volatile_var = 10;          /* Modifies volatile test variable */
        use(volatile_var);
    }
    
    /* Test 3: Pointer aliasing scenario */
    int *ptr1 = &array1[3];
    int *ptr2 = &array1[3];  /* Same location */
    process_with_alias(ptr1, ptr2);
    
    /* Test 4: Array processing with potential self-modification */
    process_array(array1, 10, 3);
    
    /* Test 5: Mixed data types */
    volatile float fvar = 1.5f;
    int ival = 0;
    mixed_types_test(&fvar, &ival);
    
    /* Test 6: Atomic operations */
    atomic_test(&atomic_counter);
    
    /* Test 7: Complex condition with multiple modifications */
    int result = complex_condition(array1, array2, 10);
    
    /* Test 8: Struct with potential aliasing */
    struct_test(&data1, &data1);  /* Self-aliasing */
    
    /* Test 9: Nested conditions */
    int y = 25, z = 30;
    if (y > 20) {
        if (z > 25) {
            y = z - 10;     /* Modifies outer condition variable */
            z = y * 2;      /* Modifies inner condition variable */
        }
    }
    
    /* Test 10: Loop with break that modifies condition */
    int counter = 0;
    for (int i = 0; i < 100; i++) {
        if (counter > 50) {
            counter = 0;    /* Modifies loop condition variable */
            break;
        }
        counter += array1[i % 10];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, iVal: %d, Atomic: %d, Volatile: %d\n", 
           result, ival, atomic_counter, volatile_var);
    printf("Data1: x=%d y=%d flag=%d\n", data1.x, data1.y, data1.flag);
    
    return 0;
}
