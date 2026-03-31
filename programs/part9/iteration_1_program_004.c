/* auto_inc_test.c - Test program for GCC auto-inc-dec optimization */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Pattern 1: Simple pointer dereference followed by increment */
int sum_array_int(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* This should generate (mem (reg)) pattern */
    while (n-- > 0) {
        sum += *p;      /* mem access via register */
        p = p + 1;      /* separate increment - should merge with above */
    }
    return sum;
}

/* Pattern 2: Char pointer with restrict */
void copy_buffer_char(char *restrict dst, const char *restrict src, int n) {
    char *d = dst;
    const char *s = src;
    
    /* Classic copy pattern */
    while (n-- > 0) {
        *d = *s;        /* Two (mem (reg)) accesses */
        d = d + 1;      /* Increment separate from access */
        s = s + 1;
    }
}

/* Pattern 3: Post-increment in expression */
int sum_array_postinc(const int *arr, int n) {
    const int *p = arr;
    int sum = 0;
    
    /* Using post-increment in the access itself */
    while (n-- > 0) {
        sum += *p++;    /* Combined access+increment */
    }
    return sum;
}

/* Pattern 4: Multiple dereferences in same block */
void process_pair(short *restrict a, short *restrict b, int n) {
    short *pa = a;
    short *pb = b;
    
    while (n-- > 0) {
        short val_a = *pa;      /* First access */
        short val_b = *pb;      /* Second access */
        pa = pa + 1;            /* Increment after both accesses */
        pb = pb + 1;
        *pa = val_b;            /* Store with different pointer */
        *pb = val_a;
    }
}

/* Pattern 5: Simple loop with array index (compiler may convert to pointer) */
int sum_array_index(const int arr[], int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];  /* May become *(arr + i) then optimize */
    }
    return sum;
}

/* Pattern 6: Local pointer in main - no function call overhead */
void test_local_pointer(void) {
    int buffer[16];
    int *p = buffer;
    
    /* Initialize */
    for (int i = 0; i < 16; ++i) {
        buffer[i] = i;
    }
    
    /* Process with simple pointer */
    int sum = 0;
    p = buffer;
    for (int i = 0; i < 16; ++i) {
        int val = *p;   /* Should be (mem (reg p)) */
        sum += val;
        p = p + 1;      /* Separate increment */
    }
    
    printf("Local pointer sum: %d\n", sum);
}

/* Pattern 7: Volatile test - ensure sequence isn't optimized away */
int sum_with_volatile(volatile int *arr, int n) {
    volatile int *p = arr;
    int sum = 0;
    
    while (n-- > 0) {
        sum += *p;      /* Volatile access */
        p = p + 1;      /* Increment after volatile access */
    }
    return sum;
}

/* Pattern 8: Nested pointer access */
void fill_pattern(int *restrict dst, int pattern, int n) {
    int *d = dst;
    
    while (n-- > 0) {
        *d = pattern;   /* Store via register */
        d = d + 1;      /* Increment separately */
    }
}

/* Main test driver */
int main(void) {
    const int ARR_SIZE = 100;
    const int BUF_SIZE = 256;
    
    /* Test data */
    int int_array[ARR_SIZE];
    char char_buffer1[BUF_SIZE];
    char char_buffer2[BUF_SIZE];
    short short_array[ARR_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARR_SIZE; ++i) {
        int_array[i] = i % 37;
        short_array[i] = (short)(i % 19);
    }
    
    for (int i = 0; i < BUF_SIZE; ++i) {
        char_buffer1[i] = (char)(i % 26 + 'A');
    }
    
    printf("Testing auto-inc-dec optimization patterns...\n");
    
    /* Test 1: Simple pointer deref + increment */
    int sum1 = sum_array_int(int_array, ARR_SIZE);
    printf("Sum1 (separate inc): %d\n", sum1);
    
    /* Test 2: Char buffer copy */
    copy_buffer_char(char_buffer2, char_buffer1, BUF_SIZE);
    printf("Copy complete, first char: %c\n", char_buffer2[0]);
    
    /* Test 3: Post-increment in expression */
    int sum3 = sum_array_postinc(int_array, ARR_SIZE);
    printf("Sum3 (post-inc): %d\n", sum3);
    
    /* Test 4: Pair processing */
    process_pair(short_array, short_array + ARR_SIZE/2, ARR_SIZE/2);
    printf("Pair processing complete\n");
    
    /* Test 5: Array index */
    int sum5 = sum_array_index(int_array, ARR_SIZE);
    printf("Sum5 (indexed): %d\n", sum5);
    
    /* Test 6: Local pointer */
    test_local_pointer();
    
    /* Test 7: Volatile */
    int sum7 = sum_with_volatile(int_array, 10);  /* Small size for volatile */
    printf("Sum7 (volatile): %d\n", sum7);
    
    /* Test 8: Fill pattern */
    int fill_array[50];
    fill_pattern(fill_array, 0x1234, 50);
    printf("Fill pattern complete, first: 0x%x\n", fill_array[0]);
    
    /* Additional test: Pointer in main itself */
    int *main_ptr = int_array;
    int main_sum = 0;
    for (int i = 0; i < 10; ++i) {
        main_sum += *main_ptr;  /* Direct (mem (reg)) in main */
        main_ptr = main_ptr + 1; /* Separate increment */
    }
    printf("Main pointer sum: %d\n", main_sum);
    
    /* Final verification */
    int total = sum1 + sum3 + sum5 + sum7 + main_sum;
    printf("Total checksum: %d\n", total);
    
    if (total != 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Error: Zero checksum\n");
        return 1;
    }
}
