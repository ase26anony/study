#include <stdio.h>
#include <stdlib.h>

/* Helper function 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment on pointer */
    }
    return sum;
}

/* Helper function 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *darr, int n) {
    double sum = 0.0;
    double *ptr = darr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Memory access with implicit zero offset */
    }
    return sum;
}

/* Helper function 3: Structure with first member access */
struct Data {
    int value;
    char padding[60];
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  /* Access first member (zero offset) */
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Helper function 4: Different data types and access sizes */
short func_mixed_types(short *sarr, char *carr, int n) {
    short sum = 0;
    short *sptr = sarr;
    char *cptr = carr;
    
    for (int i = 0; i < n; i++) {
        sum += sptr[0];     /* 16-bit access with zero offset */
        sum += cptr[0];     /* 8-bit access with zero offset */
        sptr++;             /* Increment short pointer */
        cptr++;             /* Increment char pointer */
    }
    return sum;
}

/* Helper function 5: While loop with pointer arithmetic in body */
float func_while_loop(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    int count = n;
    
    while (count > 0) {
        sum += ptr[0];      /* Zero offset access */
        ptr++;              /* Increment in loop body */
        count--;
    }
    return sum;
}

/* Helper function 6: Do-while loop ensuring execution */
long func_do_while(long *larr, int n) {
    long sum = 0;
    long *ptr = larr;
    int i = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += *ptr;        /* Dereference with implicit zero offset */
        ptr++;              /* Post-increment */
        i++;
    } while (i < n);
    
    return sum;
}

/* Helper function 7: Memory access and arithmetic separated by trivial code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Small independent operation */
        int dummy = i * 2;
        (void)dummy;        /* Prevent unused variable warning */
        
        ptr++;              /* Increment separated from access */
        sum += temp;
    }
    return sum;
}

/* Helper function 8: Nested pointer access */
int func_nested_ptr(int **parr, int n) {
    int sum = 0;
    int **ptr = parr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {  /* Zero offset access to pointer array */
            sum += *(ptr[0]);   /* Dereference the pointer */
        }
        ptr++;                  /* Increment pointer-to-pointer */
    }
    return sum;
}

/* Helper function 9: Conditional increment */
int func_conditional_inc(int *arr, int n, int threshold) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];          /* Zero offset access */
        
        if (ptr[0] > threshold) {
            ptr += 2;           /* Conditional larger increment */
        } else {
            ptr++;              /* Regular increment */
        }
    }
    return sum;
}

/* Helper function 10: Multiple increments in same block */
int func_multi_inc(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < n; i++) {
        sum += ptr1[0];         /* First zero offset access */
        sum += ptr2[0];         /* Second zero offset access */
        ptr1++;                 /* First increment */
        ptr2++;                 /* Second increment */
    }
    return sum;
}

/* Main function to exercise all patterns */
int main() {
    const int SIZE = 100;
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Initialize arrays with different types */
    int int_arr[SIZE];
    double double_arr[SIZE];
    short short_arr[SIZE];
    char char_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    struct Data struct_arr[SIZE];
    int *ptr_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = i % 100;
        char_arr[i] = i % 128;
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 100L;
        struct_arr[i].value = i * 2;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        checksum += func_zero_index_postinc(int_arr, SIZE);
        checksum += (int)func_ptr_deref_predec(double_arr, SIZE);
        checksum += func_struct_first_member(struct_arr, SIZE);
        checksum += func_mixed_types(short_arr, char_arr, SIZE);
        checksum += (int)func_while_loop(float_arr, SIZE);
        checksum += (int)func_do_while(long_arr, SIZE);
        checksum += func_separated_ops(int_arr, SIZE);
        checksum += func_nested_ptr(ptr_arr, SIZE);
        checksum += func_conditional_inc(int_arr, SIZE, 50);
        checksum += func_multi_inc(int_arr, int_arr + SIZE/2, SIZE/2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
