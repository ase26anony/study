#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment on pointer */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *ptr, int n) {
    double sum = 0.0;
    ptr += n;  /* Start from end */
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Dereference with zero offset */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[60];
};

int func_struct_first_member(struct Data *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0].value;  /* Access first member with zero offset */
        ptr++;                /* Increment pointer */
    }
    return sum;
}

/* Pattern 4: Different data types and sizes */
short func_mixed_types(short *sptr, char *cptr, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        sum += sptr[0];  /* HImode access */
        sum += cptr[0];  /* QImode access */
        sptr++;
        cptr++;
    }
    return sum;
}

/* Pattern 5: While loop with separated operations */
float func_separated_ops(float *ptr, int n) {
    float sum = 0.0f;
    int i = 0;
    while (i < n) {
        float temp = ptr[0];  /* Memory access with zero offset */
        /* Small independent operation */
        int dummy = i * 2;
        (void)dummy;
        sum += temp;
        ptr++;                /* Increment after independent code */
        i++;
    }
    return sum;
}

/* Pattern 6: Do-while loop ensuring execution */
long func_do_while(long *ptr, int n) {
    long sum = 0;
    int i = 0;
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Pointer increment */
        i++;
    } while (i < n);
    
    return sum;
}

/* Pattern 7: Nested pointer arithmetic */
void func_nested_access(int **pptr, int n, int *result) {
    *result = 0;
    for (int i = 0; i < n; i++) {
        *result += (*pptr)[0];  /* Dereference then zero offset */
        (*pptr)++;              /* Increment through pointer */
    }
}

/* Pattern 8: Conditional with always-taken branch */
int func_conditional_inc(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true in our usage */
            sum += ptr[0];  /* Memory access with zero offset */
        }
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Pattern 9: Multiple increments in same block */
int func_multi_inc(int *ptr1, int *ptr2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr1[0];  /* First memory access */
        sum += ptr2[0];  /* Second memory access */
        ptr1++;          /* First increment */
        ptr2++;          /* Second increment */
    }
    return sum;
}

/* Pattern 10: Floating point with different modes */
double func_float_ops(float *fptr, double *dptr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += fptr[0];  /* SFmode access */
        sum += dptr[0];  /* DFmode access */
        fptr++;
        dptr++;
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    /* Initialize test data */
    const int SIZE = 100;
    
    int int_arr[SIZE];
    double double_arr[SIZE];
    short short_arr[SIZE];
    char char_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    struct Data struct_arr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = i % 32767;
        char_arr[i] = i % 127;
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 100L;
        struct_arr[i].value = i * 2;
    }
    
    volatile int total = 0;  /* Prevent dead code elimination */
    
    /* Call each function multiple times */
    for (int iter = 0; iter < 10; iter++) {
        total += func_zero_index_postinc(int_arr, SIZE);
        total += func_ptr_deref_predec(double_arr, SIZE);
        total += func_struct_first_member(struct_arr, SIZE);
        total += func_mixed_types(short_arr, char_arr, SIZE);
        total += func_separated_ops(float_arr, SIZE);
        total += func_do_while(long_arr, SIZE);
        
        int *ptr = int_arr;
        int nested_result;
        func_nested_access(&ptr, SIZE, &nested_result);
        total += nested_result;
        
        total += func_conditional_inc(int_arr, SIZE);
        total += func_multi_inc(int_arr, short_arr, SIZE);
        total += func_float_ops(float_arr, double_arr, SIZE);
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", total);
    
    return 0;
}
