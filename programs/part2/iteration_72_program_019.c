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
        sum += *ptr;    /* Dereference with implicit zero offset */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[12];
    double extra;
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

/* Pattern 4: Different data types and sizes */
void func_mixed_types() {
    char c_arr[100];
    short s_arr[100];
    int i_arr[100];
    long l_arr[100];
    float f_arr[100];
    double d_arr[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        c_arr[i] = i % 128;
        s_arr[i] = i * 2;
        i_arr[i] = i * 3;
        l_arr[i] = i * 4L;
        f_arr[i] = i * 1.5f;
        d_arr[i] = i * 2.5;
    }
    
    /* Access with zero offset and pointer arithmetic */
    char *c_ptr = c_arr;
    short *s_ptr = s_arr;
    int *i_ptr = i_arr;
    long *l_ptr = l_arr;
    float *f_ptr = f_arr;
    double *d_ptr = d_arr;
    
    volatile int dummy = 0;
    
    for (int i = 0; i < 10; i++) {
        dummy += c_ptr[0];  /* QImode access */
        c_ptr++;
        
        dummy += s_ptr[0];  /* HImode access */
        s_ptr++;
        
        dummy += i_ptr[0];  /* SImode access */
        i_ptr++;
        
        dummy += l_ptr[0];  /* DImode access on 64-bit */
        l_ptr++;
        
        dummy += (int)f_ptr[0];  /* SFmode access */
        f_ptr++;
        
        dummy += (int)d_ptr[0];  /* DFmode access */
        d_ptr++;
    }
}

/* Pattern 5: While loop with separated operations */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Small independent calculation */
        int independent = i * 2;
        (void)independent;  /* Prevent unused variable warning */
        
        ptr++;              /* Increment separated by independent code */
        sum += temp;
        i++;
    }
    return sum;
}

/* Pattern 6: Do-while loop with post-decrement */
int func_dowhile_postdec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    int i = n;
    
    do {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr--;          /* Post-decrement */
        i--;
    } while (i > 0);
    
    return sum;
}

/* Pattern 7: Nested pointer arithmetic */
int func_nested_ptr(int **arr, int n) {
    int sum = 0;
    int **ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {  /* Access with zero offset */
            sum += *(ptr[0]);   /* Dereference the pointer */
        }
        ptr++;                  /* Increment pointer to pointer */
    }
    return sum;
}

/* Pattern 8: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        
        /* Conditional that compiler can't eliminate */
        if (n > 0) {    /* Always true given loop condition */
            ptr++;      /* Increment in conditional block */
        }
    }
    return sum;
}

/* Pattern 9: Multiple increments in same block */
int func_multi_inc(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n/2;
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];  /* First memory access */
        sum += ptr2[0];  /* Second memory access */
        
        ptr1++;          /* Increment first pointer */
        ptr2++;          /* Increment second pointer */
    }
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    
    /* Initialize test arrays */
    int arr[SIZE];
    struct Data data_arr[SIZE];
    int *ptr_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3;
        data_arr[i].value = i * 5;
        ptr_arr[i] = &arr[i];
    }
    
    volatile int total = 0;
    
    /* Execute each pattern multiple times */
    for (int iter = 0; iter < 10; iter++) {
        total += func_zero_index_postinc(arr, SIZE);
        total += func_ptr_deref_predec(arr, SIZE);
        total += func_struct_first_member(data_arr, SIZE);
        total += func_separated_ops(arr, SIZE);
        total += func_dowhile_postdec(arr, SIZE);
        total += func_nested_ptr(ptr_arr, SIZE/10);
        total += func_always_taken(arr, SIZE);
        total += func_multi_inc(arr, SIZE);
    }
    
    func_mixed_types();
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
