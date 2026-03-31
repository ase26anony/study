#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  /* Memory access with zero offset */
        arr++;          /* Pointer arithmetic in same basic block */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *darr, int n) {
    double sum = 0.0;
    double *ptr = darr + n - 1;
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement */
        sum += *ptr;    /* Dereference with implicit zero offset */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char name[20];
    float score;
};

int func_struct_first_member(struct Data *data, int n) {
    int total = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        total += ptr->value;  /* Access first member (offset 0) */
        ptr++;                /* Pointer arithmetic */
    }
    return total;
}

/* Pattern 4: Loop with pointer increment in increment expression */
char func_loop_increment_expr(char *buffer, int n) {
    char result = 0;
    char *ptr = buffer;
    
    for (int i = 0; i < n; ptr++, i++) {  /* ptr++ in loop increment */
        result ^= ptr[0];                  /* Zero offset access */
    }
    return result;
}

/* Pattern 5: While loop with post-decrement */
short func_while_postdec(short *sarr, int n) {
    short sum = 0;
    short *ptr = sarr + n - 1;
    
    while (n-- > 0) {
        sum += *ptr;    /* Dereference */
        ptr--;          /* Post-decrement */
    }
    return sum;
}

/* Pattern 6: Do-while with pre-increment */
long func_dowhile_preinc(long *larr, int n) {
    long total = 0;
    long *ptr = larr - 1;  /* Start before array */
    int i = 0;
    
    do {
        ++ptr;              /* Pre-increment */
        total += ptr[0];    /* Zero offset access */
        i++;
    } while (i < n);
    
    return total;
}

/* Pattern 7: Memory access and arithmetic separated by trivial code */
float func_separated_ops(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    
    for (int i = 0; i < n; i++) {
        sum += farr[0];     /* Zero offset access */
        
        /* Small independent operation to separate */
        int temp = i * 2;
        (void)temp;         /* Use to prevent optimization */
        
        farr++;             /* Pointer arithmetic after separation */
    }
    return sum;
}

/* Pattern 8: Multiple types in same function */
void func_mixed_types(void) {
    char cbuf[100];
    int ibuf[100];
    double dbuf[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        cbuf[i] = (char)(i % 256);
        ibuf[i] = i * 2;
        dbuf[i] = i * 0.5;
    }
    
    /* Char access with post-increment */
    char *cptr = cbuf;
    for (int i = 0; i < 10; i++) {
        volatile char c = cptr[0];  /* volatile to prevent elimination */
        cptr++;
    }
    
    /* Int access with pre-decrement */
    int *iptr = ibuf + 10;
    for (int i = 0; i < 10; i++) {
        --iptr;
        volatile int val = *iptr;
    }
    
    /* Double access in loop increment */
    double *dptr = dbuf;
    for (int i = 0; i < 10; dptr++, i++) {
        volatile double d = dptr[0];
    }
}

/* Pattern 9: Conditional that's always taken */
int func_conditional_always(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Zero offset */
        
        /* Always-taken conditional */
        if (i >= 0) {   /* Always true */
            ptr++;      /* Arithmetic inside conditional */
        }
    }
    return sum;
}

/* Pattern 10: Nested zero offset accesses */
int func_nested_access(int **arr, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through pointer-to-pointer with zero offset */
        if (arr[0] != NULL) {          /* First level zero offset */
            total += arr[0][0];        /* Second level zero offset */
        }
        arr++;  /* Increment pointer */
    }
    return total;
}

/* Main function that exercises all patterns */
int main(void) {
    /* Initialize test data */
    int int_arr[100];
    double double_arr[100];
    short short_arr[100];
    long long_arr[100];
    float float_arr[100];
    char char_arr[100];
    struct Data struct_arr[50];
    int *ptr_arr[20];
    
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        short_arr[i] = (short)(i % 32768);
        long_arr[i] = i * 100L;
        float_arr[i] = i * 0.25f;
        char_arr[i] = (char)(i % 128);
    }
    
    for (int i = 0; i < 50; i++) {
        struct_arr[i].value = i * 3;
        snprintf(struct_arr[i].name, 20, "Item%d", i);
        struct_arr[i].score = i * 0.1f;
    }
    
    for (int i = 0; i < 20; i++) {
        ptr_arr[i] = &int_arr[i * 5];
    }
    
    volatile int checksum = 0;  /* volatile to prevent dead code elimination */
    
    /* Call all pattern functions multiple times */
    for (int iter = 0; iter < 3; iter++) {
        checksum += func_zero_index_postinc(int_arr, 10);
        checksum += (int)func_ptr_deref_predec(double_arr, 10);
        checksum += func_struct_first_member(struct_arr, 10);
        checksum += func_loop_increment_expr(char_arr, 10);
        checksum += func_while_postdec(short_arr, 10);
        checksum += (int)func_dowhile_preinc(long_arr, 10);
        checksum += (int)func_separated_ops(float_arr, 10);
        func_mixed_types();
        checksum += func_conditional_always(int_arr, 10);
        checksum += func_nested_access(ptr_arr, 10);
    }
    
    /* Print result to ensure code isn't eliminated */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
