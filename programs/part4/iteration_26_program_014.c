/* test_auto_inc_dec.c
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) should succeed.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_simple_pointer(int *arr, int size) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access uses *(ptr + 0), then ptr is incremented */
    for (int i = 0; i < size; i++) {
        sum += *p;      /* Should become base + 0 at RTL level */
        p++;            /* Separate increment instruction for find_inc to find */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer induction variable */
void pattern2_indexed_access(int *buffer, int size) {
    /* Simple initialization loop - compiler's ivopts should create
     * a pointer induction variable with base + 0 addressing */
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;  /* Should become *(ptr + 0) after optimization */
    }
}

/* Pattern 3: Explicit pointer arithmetic with stride */
int pattern3_explicit_stride(int *arr, int size, int stride) {
    int total = 0;
    int *ptr = arr;
    int *end = arr + size * stride;
    
    /* Explicit increment separate from access */
    for (; ptr < end; ) {
        total += *ptr;  /* base + 0 */
        ptr += stride;  /* constant increment for find_inc */
    }
    return total;
}

/* Pattern 4: Nested loops with invariant base pointer */
void pattern4_nested_loops(int matrix[M][N]) {
    /* Outer loop calculates base, inner loop uses it with zero offset */
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base calculated in outer loop */
        
        /* Inner loop accesses should be base + 0 */
        for (int i = 0; i < N; i++) {
            base[i] = i * j;  /* Should become *(base + 0) after ivopts */
        }
    }
}

/* Pattern 5: Struct access to ensure proper scaling */
struct Data {
    int a;
    int b;
    float c;
};

int pattern5_struct_access(struct Data *arr, int size) {
    int sum = 0;
    struct Data *ptr = arr;
    
    /* Struct pointer increment - fixed stride of sizeof(struct Data) */
    for (int i = 0; i < size; i++) {
        sum += ptr->a;  /* Access through pointer - should be base + 0 */
        ptr++;          /* Constant increment by struct size */
    }
    return sum;
}

/* Pattern 6: Char pointer with different stride */
int pattern6_char_access(char *str, int length) {
    int count = 0;
    char *p = str;
    
    /* Char pointer - increment by 1 */
    for (int i = 0; i < length; i++) {
        if (*p == 'a')  /* base + 0 access */
            count++;
        p++;            /* increment by 1 */
    }
    return count;
}

/* Pattern 7: Mixed access pattern to avoid over-optimization */
void pattern7_mixed_access(int *dest, int *src1, int *src2, int size) {
    int *d = dest;
    int *s1 = src1;
    int *s2 = src2;
    
    /* Multiple pointers being incremented - each access should be base + 0 */
    for (int i = 0; i < size; i++) {
        *d = *s1 + *s2;  /* Three separate base + 0 accesses */
        d++;
        s1++;
        s2++;
    }
}

/* Pattern 8: Do-while loop to ensure loop body executes at least once */
int pattern8_dowhile(int *arr, int size) {
    if (size <= 0) return 0;
    
    int sum = 0;
    int *p = arr;
    int count = size;
    
    /* Do-while ensures the loop body is present even with low optimization */
    do {
        sum += *p;  /* base + 0 */
        p++;        /* increment */
        count--;
    } while (count > 0);
    
    return sum;
}

/* Pattern 9: Reverse traversal with auto-decrement possibility */
int pattern9_reverse(int *arr, int size) {
    int sum = 0;
    int *p = arr + size - 1;
    
    /* Decrement instead of increment */
    for (int i = 0; i < size; i++) {
        sum += *p;  /* base + 0 */
        p--;        /* decrement for auto-dec optimization */
    }
    return sum;
}

/* Pattern 10: Complex enough to avoid being optimized away entirely */
int pattern10_complex_enough(int *arr, int size) {
    volatile int sink;  /* Prevent dead code elimination */
    int *p = arr;
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple operations to create interesting RTL */
        int val = *p;           /* Load with base + 0 */
        result ^= val;
        *p = result;           /* Store with base + 0 */
        p++;                   /* Increment */
        
        /* Volatile to prevent reordering */
        sink = result;
    }
    
    return result;
}

int main() {
    /* Initialize test data */
    int arr1[N];
    int arr2[N];
    int matrix[M][N];
    struct Data struct_arr[N];
    char str[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        str[i] = (i % 26) + 'a';
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
        struct_arr[i].c = i * 0.5f;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = j * N + i;
        }
    }
    
    /* Execute all patterns to ensure code is generated */
    int result = 0;
    
    result += pattern1_simple_pointer(arr1, N);
    pattern2_indexed_access(arr2, N);
    result += pattern3_explicit_stride(arr1, N, 1);
    pattern4_nested_loops(matrix);
    result += pattern5_struct_access(struct_arr, N);
    result += pattern6_char_access(str, N);
    pattern7_mixed_access(arr1, arr2, arr1, N);
    result += pattern8_dowhile(arr1, N);
    result += pattern9_reverse(arr1, N);
    result += pattern10_complex_enough(arr1, N);
    
    printf("Result: %d\n", result);
    return 0;
}
