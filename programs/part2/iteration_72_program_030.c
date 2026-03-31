#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment on pointer */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Memory dereference (implicit offset 0) */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[16];
    float extra;
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  /* Access first member (offset 0) */
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Pattern 4: Different data types and sizes */
short func_short_type(short *arr, int n) {
    short sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* QImode/HImode access */
        ptr++;          /* Pointer arithmetic */
    }
    return sum;
}

/* Pattern 5: Floating point types */
float func_float_type(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* SFmode access */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 6: While loop with post-decrement */
int func_while_postdec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    while (n-- > 0) {
        sum += ptr[0];  /* Zero offset access */
        ptr--;          /* Post-decrement */
    }
    return sum;
}

/* Pattern 7: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    if (n <= 0) return 0;
    
    do {
        sum += *ptr;    /* Dereference (offset 0) */
        ptr++;          /* Increment */
    } while (--n > 0);
    
    return sum;
}

/* Pattern 8: Separated operations with trivial code between */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Small independent operation */
        int dummy = i * 2;
        (void)dummy;
        
        ptr++;              /* Increment separated by code */
        sum += temp;
    }
    return sum;
}

/* Pattern 9: Multiple increments in same basic block */
int func_multi_inc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i += 2) {
        sum += ptr[0];  /* First access */
        ptr++;          /* First increment */
        
        sum += ptr[0];  /* Second access */
        ptr++;          /* Second increment */
    }
    return sum;
}

/* Pattern 10: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true in practice */
            sum += ptr[0];  /* Zero offset access */
        }
        ptr++;              /* Increment after condition */
    }
    return sum;
}

/* Pattern 11: Long long type for DImode */
long long func_longlong_type(long long *arr, int n) {
    long long sum = 0;
    long long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* DImode access */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 12: Char type for QImode */
int func_char_type(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* QImode access */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 13: Mixed operations in loop increment */
int func_mixed_increment(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        sum += ptr[0];  /* Zero offset access */
        i++;
        ptr++;          /* Both index and pointer increment */
    }
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* Prevent dead code elimination */
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    short short_arr[SIZE];
    float float_arr[SIZE];
    long long longlong_arr[SIZE];
    char char_arr[SIZE];
    struct Data struct_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        short_arr[i] = i % 100;
        float_arr[i] = i * 0.5f;
        longlong_arr[i] = i * 100LL;
        char_arr[i] = 'A' + (i % 26);
        struct_arr[i].value = i * 2;
    }
    
    /* Execute each pattern multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_short_type(short_arr, SIZE);
        result += func_float_type(float_arr, SIZE);
        result += func_while_postdec(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_multi_inc(int_arr, SIZE);
        result += func_always_taken(int_arr, SIZE);
        result += func_longlong_type(longlong_arr, SIZE);
        result += func_char_type(char_arr, SIZE);
        result += func_mixed_increment(int_arr, SIZE);
    }
    
    /* Print checksum to ensure all code executes */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
