/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or with: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100
#define INNER_SIZE 50
#define STRIDE 4

/* Pattern 1: Simple pointer traversal with post-increment */
int pattern1_simple_pointer(int *arr) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access uses ptr+0, then ptr increments */
    for (int i = 0; i < SIZE; i++) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment instruction */
    }
    return sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void pattern2_indexed_access(char *buffer) {
    /* Simple indexed access - ivopts may convert to pointer form */
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) with separate ptr increment */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3_nested_loops(int matrix[][INNER_SIZE]) {
    for (int j = 0; j < SIZE; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 pattern */
        for (int i = 0; i < INNER_SIZE; i++) {
            base[i] = i;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit pointer arithmetic with stride */
int pattern4_explicit_stride(int *arr, int stride) {
    int total = 0;
    int *ptr = arr;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < SIZE; i++) {
        total += *ptr;    /* *(ptr + 0) */
        ptr += stride;    /* Candidate increment instruction */
    }
    return total;
}

/* Pattern 5: Struct access to ensure non-trivial element size */
struct Data {
    int a;
    int b;
    float c;
    char d[4];
};

int pattern5_struct_access(struct Data *array) {
    int sum = 0;
    struct Data *ptr = array;
    
    /* Struct pointer traversal */
    for (int i = 0; i < SIZE; i++) {
        sum += ptr->a;  /* Access at offset 0 within struct */
        ptr++;          /* Increment by sizeof(struct Data) */
    }
    return sum;
}

/* Pattern 6: Mixed patterns to increase coverage */
void pattern6_mixed(int *arr1, char *arr2, float *arr3) {
    /* Multiple loops with different access patterns */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 2;
    }
    
    char *p = arr2;
    for (int i = 0; i < SIZE * 2; i++) {
        *p = (char)i;
        p++;
    }
    
    float *fptr = arr3;
    for (int i = 0; i < SIZE; i++) {
        *fptr = (float)i;
        fptr += 1;  /* Explicit increment */
    }
}

/* Pattern 7: Loop with multiple memory references */
int pattern7_multiple_refs(int *src, int *dst) {
    int sum = 0;
    int *s = src;
    int *d = dst;
    
    for (int i = 0; i < SIZE; i++) {
        int val = *s;      /* First access: *(s + 0) */
        *d = val;          /* Second access: *(d + 0) */
        sum += val;
        s++;              /* Candidate increment for src */
        d++;              /* Candidate increment for dst */
    }
    return sum;
}

/* Pattern 8: Do-while loop variant */
int pattern8_dowhile(int *arr) {
    int sum = 0;
    int *p = arr;
    int count = SIZE;
    
    do {
        sum += *p;  /* *(p + 0) */
        p++;        /* Increment instruction */
    } while (--count > 0);
    
    return sum;
}

/* Pattern 9: While loop with pointer */
int pattern9_while(int *arr) {
    int sum = 0;
    int *p = arr;
    int *end = arr + SIZE;
    
    while (p < end) {
        sum += *p;  /* *(p + 0) */
        p++;        /* Separate increment */
    }
    return sum;
}

/* Pattern 10: Access with zero offset through pointer dereference */
int pattern10_direct_deref(int **ptr_arr) {
    int sum = 0;
    int **pp = ptr_arr;
    
    for (int i = 0; i < SIZE; i++) {
        int *p = *pp;      /* Load pointer from array */
        sum += *p;         /* Dereference that pointer (offset 0) */
        pp++;              /* Increment pointer-to-pointer */
    }
    return sum;
}

/* Main function to ensure all patterns are used */
int main() {
    /* Allocate arrays for testing */
    int arr1[SIZE];
    char arr2[SIZE * 2];
    float arr3[SIZE];
    int matrix[SIZE][INNER_SIZE];
    struct Data struct_arr[SIZE];
    int src[SIZE], dst[SIZE];
    int *ptr_arr[SIZE];
    
    /* Initialize pointer array */
    for (int i = 0; i < SIZE; i++) {
        ptr_arr[i] = &arr1[i];
    }
    
    /* Call all patterns to ensure they're compiled */
    int result = 0;
    
    result += pattern1_simple_pointer(arr1);
    pattern2_indexed_access(arr2);
    pattern3_nested_loops(matrix);
    result += pattern4_explicit_stride(arr1, STRIDE);
    result += pattern5_struct_access(struct_arr);
    pattern6_mixed(arr1, arr2, arr3);
    result += pattern7_multiple_refs(src, dst);
    result += pattern8_dowhile(arr1);
    result += pattern9_while(arr1);
    result += pattern10_direct_deref(ptr_arr);
    
    return result > 0 ? 0 : 1;
}
