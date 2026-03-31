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
struct FirstMember {
    int value;
    char padding[60];
};

short func_struct_first_member(struct FirstMember *sarr, int n) {
    short sum = 0;
    struct FirstMember *ptr = sarr;
    
    for (int i = 0; i < n; i++) {
        sum += (short)ptr[0].value;  /* Access first member with zero offset */
        ptr++;                       /* Pointer increment */
    }
    return sum;
}

/* Helper function 4: Mixed types in loop increment expression */
float func_loop_inc(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    
    /* Loop with increment in the increment expression */
    for (int i = 0; i < n; ptr++, i++) {
        sum += ptr[0];  /* Zero offset access */
    }
    return sum;
}

/* Helper function 5: While loop with post-decrement */
long func_while_postdec(long *larr, int n) {
    long sum = 0;
    long *ptr = larr + n - 1;
    
    while (n-- > 0) {
        sum += ptr[0];  /* Zero offset access */
        ptr--;          /* Post-decrement */
    }
    return sum;
}

/* Helper function 6: Do-while with pre-increment */
char func_dowhile_preinc(char *carr, int n) {
    char sum = 0;
    char *ptr = carr - 1;  /* Start one before beginning */
    
    if (n <= 0) return 0;
    
    do {
        ++ptr;          /* Pre-increment */
        sum += ptr[0];  /* Zero offset access */
    } while (--n > 0);
    
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
        (void)dummy;
        
        ptr++;              /* Pointer increment separated by code */
        sum += temp;
    }
    return sum;
}

/* Helper function 8: Conditional that's always taken */
double func_always_taken_cond(double *darr, int n) {
    double sum = 0.0;
    double *ptr = darr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true condition */
            sum += ptr[0];  /* Zero offset access */
        }
        ptr++;              /* Pointer increment */
    }
    return sum;
}

/* Helper function 9: Different access sizes via char pointer */
unsigned short func_char_ptr_inc(unsigned char *barr, int n) {
    unsigned short sum = 0;
    unsigned char *ptr = barr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* QImode access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Helper function 10: Nested pointer arithmetic */
int func_nested_ptr(int **parr, int n) {
    int sum = 0;
    int **ptr = parr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {      /* Zero offset access to pointer */
            sum += *(ptr[0]);       /* Dereference the pointer */
        }
        ptr++;                     /* Increment pointer-to-pointer */
    }
    return sum;
}

int main(void) {
    /* Initialize arrays of different types and sizes */
    int int_arr[100];
    double double_arr[100];
    float float_arr[100];
    long long_arr[100];
    char char_arr[100];
    unsigned char uchar_arr[100];
    struct FirstMember struct_arr[50];
    int *ptr_arr[50];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        float_arr[i] = i * 0.75f;
        long_arr[i] = i * 1000L;
        char_arr[i] = (char)(i % 128);
        uchar_arr[i] = (unsigned char)(i % 256);
    }
    
    for (int i = 0; i < 50; i++) {
        struct_arr[i].value = i * 2;
        ptr_arr[i] = &int_arr[i * 2];
    }
    
    volatile int total = 0;  /* Prevent dead code elimination */
    
    /* Call each function multiple times with different patterns */
    for (int iter = 0; iter < 10; iter++) {
        total += func_zero_index_postinc(int_arr, 50);
        total += (int)func_ptr_deref_predec(double_arr, 50);
        total += func_struct_first_member(struct_arr, 25);
        total += (int)func_loop_inc(float_arr, 50);
        total += (int)func_while_postdec(long_arr, 50);
        total += func_dowhile_preinc(char_arr, 50);
        total += func_separated_ops(int_arr, 50);
        total += (int)func_always_taken_cond(double_arr, 50);
        total += func_char_ptr_inc(uchar_arr, 50);
        total += func_nested_ptr(ptr_arr, 25);
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", total);
    
    return 0;
}
