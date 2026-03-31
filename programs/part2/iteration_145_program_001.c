#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function prototypes */
void process_direct(int *p);
void process_aliased(int *p, int *q);
int process_volatile(volatile int *p);
void process_array(int arr[], int n, int threshold);
float process_mixed_types(int x, float y);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Main function with various if-conversion candidates */
int main() {
    int result = 0;
    
    /* 1. Direct modification of test variable */
    int x = 10;
    if (x > 5) {
        x = x * 2;      /* Modifies test variable */
        x = x - 3;      /* Another modification */
        /* Total: 2 non-label instructions modifying test expression */
    }
    result += x;
    
    /* 2. Volatile variable modification */
    volatile int v = 20;
    if (v > 15) {
        v = 25;         /* Modifies volatile test variable */
        v = v + 5;      /* Another modification */
        /* Volatile prevents optimization, keeps if-conversion check */
    }
    result += v;
    
    /* 3. Pointer-based modification with aliasing */
    int a = 30, b = 40;
    int *ptr1 = &a;
    int *ptr2 = &a;     /* Same address - definite aliasing */
    if (*ptr1 > 25) {
        *ptr2 = 35;     /* Modifies memory used in test expression */
        *ptr1 = *ptr1 + 2;  /* Another modification */
    }
    result += a;
    
    /* 4. Array access with potential aliasing */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 10;
    }
    
    /* Loop with condition modifying test location */
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 25) {
            arr[i] = 0;     /* Modifies array element used in test */
            arr[i] = -1;    /* Second modification */
        }
    }
    
    /* 5. Mixed data types with implicit conversions */
    int int_val = 50;
    float float_val = 3.14f;
    if ((float)int_val > 2.5f) {
        int_val = (int)(float_val * 10.0f);  /* Modifies test variable */
        int_val = int_val % 7;               /* Another modification */
    }
    result += int_val;
    
    /* 6. Function calls with pointer parameters */
    int local_var = 60;
    process_direct(&local_var);
    result += local_var;
    
    /* 7. Aliased pointers in function */
    int var1 = 70, var2 = 80;
    process_aliased(&var1, &var1);  /* Same pointer - aliasing */
    result += var1;
    
    /* 8. Volatile pointer in function */
    volatile int volatile_var = 90;
    result += process_volatile(&volatile_var);
    
    /* 9. Array processing function */
    int data[5] = {100, 200, 300, 400, 500};
    process_array(data, 5, 250);
    for (int i = 0; i < 5; i++) {
        result += data[i];
    }
    
    /* 10. Mixed type function */
    float float_result = process_mixed_types(123, 4.56f);
    result += (int)float_result;
    
    /* Prevent dead code elimination */
    printf("Final result: %d\n", result);
    return result > 0 ? 0 : 1;
}

/* Function that modifies test expression directly */
void process_direct(int *p) {
    if (*p > 55) {
        *p = *p * 2;    /* Modifies memory used in test */
        *p = *p / 3;    /* Another modification */
    }
}

/* Function with potential aliasing */
void process_aliased(int *p, int *q) {
    /* p and q may alias */
    if (*p > 65) {
        *q = 75;        /* May modify test expression if p == q */
        *p = *p + 5;    /* Another modification */
    }
}

/* Function with volatile pointer */
int process_volatile(volatile int *p) {
    int temp = 0;
    if (*p > 85) {
        *p = 95;        /* Modifies volatile test location */
        *p = *p - 10;   /* Another modification */
        temp = *p;
    }
    return temp;
}

/* Array processing with loop-dependent condition */
void process_array(int arr[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = threshold;     /* Modifies test location */
            arr[i] = arr[i] - 50;   /* Another modification */
        }
    }
}

/* Mixed data types with conversions */
float process_mixed_types(int x, float y) {
    float result = y;
    if ((float)x > 100.0f) {
        x = (int)(y * 20.0f);   /* Modifies variable used in test */
        x = x ^ 0xFF;           /* Another modification (bitwise) */
        result = (float)x;
    }
    return result;
}
