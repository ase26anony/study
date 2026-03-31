#include <stdio.h>
#include <stdlib.h>

/* Helper function 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment in same basic block */
    }
    return sum;
}

/* Helper function 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *darr, int n) {
    double sum = 0.0;
    double *ptr = &darr[n-1];
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement */
        sum += *ptr;    /* Dereference with implicit zero offset */
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

/* Helper function 5: While loop with post-decrement */
float func_while_postdec(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = &farr[n-1];
    int count = n;
    
    while (count > 0) {
        sum += ptr[0];  /* Zero offset access */
        ptr--;          /* Post-decrement */
        count--;
    }
    return sum;
}

/* Helper function 6: Do-while loop with pre-increment */
long func_dowhile_preinc(long *larr, int n) {
    long sum = 0;
    long *ptr = larr;
    int i = 0;
    
    do {
        sum += *ptr;    /* Dereference with implicit zero offset */
        ++ptr;          /* Pre-increment */
        i++;
    } while (i < n);
    return sum;
}

/* Helper function 7: Memory access and arithmetic separated by trivial code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int val = ptr[0];   /* Memory access with zero offset */
        
        /* Trivial independent operation to separate instructions */
        int dummy = i * 2;
        (void)dummy;
        
        sum += val;
        ptr++;              /* Increment separated from memory access */
    }
    return sum;
}

/* Helper function 8: Conditional that's always taken */
unsigned char func_conditional_access(unsigned char *arr, int n) {
    unsigned char sum = 0;
    unsigned char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true condition */
            sum += ptr[0];  /* Zero offset access */
        }
        ptr++;              /* Increment after conditional */
    }
    return sum;
}

/* Helper function 9: Nested access patterns */
int func_nested_patterns(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 0; i < n; i++) {
        sum += ptr1[0];     /* First zero offset access */
        sum += ptr2[0];     /* Second zero offset access */
        ptr1++;             /* Increment first pointer */
        ptr2++;             /* Increment second pointer */
    }
    return sum;
}

/* Helper function 10: Complex expression with zero offset */
double func_complex_expr(double *darr, int *iarr, int n) {
    double sum = 0.0;
    double *dptr = darr;
    int *iptr = iarr;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression using zero-offset accesses */
        sum += dptr[0] * (iptr[0] + 1.0);
        dptr++;
        iptr++;
    }
    return sum;
}

int main() {
    const int SIZE = 100;
    volatile int checksum = 0;  /* Prevent dead code elimination */
    
    /* Initialize arrays with different types */
    int int_arr[SIZE];
    double double_arr[SIZE];
    short short_arr[SIZE];
    char char_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    unsigned char uchar_arr[SIZE];
    struct Data struct_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = i % 100;
        char_arr[i] = i % 128;
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 1000L;
        uchar_arr[i] = i % 256;
        struct_arr[i].value = i * 2;
    }
    
    /* Call all functions multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        checksum += func_zero_index_postinc(int_arr, SIZE);
        checksum += (int)func_ptr_deref_predec(double_arr, SIZE);
        checksum += func_struct_first_member(struct_arr, SIZE);
        checksum += func_mixed_types(short_arr, char_arr, SIZE);
        checksum += (int)func_while_postdec(float_arr, SIZE);
        checksum += (int)func_dowhile_preinc(long_arr, SIZE);
        checksum += func_separated_ops(int_arr, SIZE);
        checksum += func_conditional_access(uchar_arr, SIZE);
        checksum += func_nested_patterns(int_arr, &int_arr[SIZE/2], SIZE/2);
        checksum += (int)func_complex_expr(double_arr, int_arr, SIZE);
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
