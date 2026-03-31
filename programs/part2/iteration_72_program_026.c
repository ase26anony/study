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
double func_ptr_deref_predec(double *dptr, int n) {
    double sum = 0.0;
    double *ptr = dptr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Memory access via dereference (zero offset) */
    }
    return sum;
}

/* Helper function 3: Structure with first member access */
struct Data {
    int value;
    char tag;
    float extra;
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
short func_mixed_types(short *sptr, char *cptr, int n) {
    short sum = 0;
    
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
    int count = 0;
    
    while (count < n) {
        sum += ptr[0];      /* Zero offset access */
        ptr++;              /* Post-increment */
        count++;
    }
    return sum;
}

/* Helper function 6: Do-while loop ensuring at least one execution */
long func_do_while(long *larr, int n) {
    long sum = 0;
    long *ptr = larr;
    int count = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += *ptr;        /* Dereference (zero offset) */
        ptr++;              /* Increment pointer */
        count++;
    } while (count < n);
    
    return sum;
}

/* Helper function 7: Memory access and pointer arithmetic separated */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Small independent operation */
        int dummy = temp * 2;
        (void)dummy;        /* Prevent unused variable warning */
        
        ptr++;              /* Pointer increment separated by code */
        
        sum += temp;
    }
    return sum;
}

/* Helper function 8: Nested pointer with zero offset */
int func_nested_ptr(int **pptr, int n) {
    int sum = 0;
    int **ptr = pptr;
    
    for (int i = 0; i < n; i++) {
        sum += (*ptr)[0];   /* Zero offset through dereferenced pointer */
        ptr++;              /* Increment pointer to pointer */
    }
    return sum;
}

/* Helper function 9: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true for our calls */
            sum += ptr[0];  /* Zero offset access */
        }
        ptr++;              /* Increment after conditional */
    }
    return sum;
}

/* Helper function 10: Multiple increments in same block */
int func_multiple_increments(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n/2;
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];     /* Zero offset access */
        sum += ptr2[0];     /* Another zero offset access */
        ptr1++;             /* First increment */
        ptr2++;             /* Second increment */
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    double double_arr[SIZE];
    short short_arr[SIZE];
    char char_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    struct Data data_arr[SIZE];
    int *ptr_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = i % 100;
        char_arr[i] = 'A' + (i % 26);
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 1000L;
        data_arr[i].value = i * 2;
        data_arr[i].tag = 'X';
        data_arr[i].extra = i * 0.5f;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        checksum += func_zero_index_postinc(int_arr, SIZE);
        checksum += (int)func_ptr_deref_predec(double_arr, SIZE);
        checksum += func_struct_first_member(data_arr, SIZE);
        checksum += func_mixed_types(short_arr, char_arr, SIZE);
        checksum += (int)func_while_loop(float_arr, SIZE);
        checksum += (int)func_do_while(long_arr, SIZE);
        checksum += func_separated_ops(int_arr, SIZE);
        checksum += func_nested_ptr(ptr_arr, SIZE/10);
        checksum += func_always_taken(int_arr, SIZE);
        checksum += func_multiple_increments(int_arr, SIZE);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
