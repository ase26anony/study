/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#define SIZE 100
#define INNER_SIZE 50

/* Pattern 1: Simple pointer traversal with post-increment */
void pattern1(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    
    /* Classic *ptr++ pattern - access with offset 0, then increment */
    for (int i = 0; i < n; i++) {
        sum += *p;  /* Should become *(p + 0) after optimization */
        p++;        /* Separate increment instruction for find_inc to find */
    }
    
    /* Prevent dead code elimination */
    arr[0] = sum;
}

/* Pattern 2: Indexed access where ivopts creates pointer with zero offset */
void pattern2(char *buffer, int n) {
    /* Simple memset-like pattern */
    for (int i = 0; i < n; i++) {
        buffer[i] = 0;  /* May become *(ptr + 0) after ivopts */
    }
}

/* Pattern 3: Nested loops with invariant base in inner loop */
void pattern3(int matrix[][INNER_SIZE], int m, int n) {
    for (int j = 0; j < m; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < n; i++) {
            base[i] = i;  /* Access relative to invariant base */
        }
    }
}

/* Pattern 4: Explicit stride with separate increment */
void pattern4(float *data, int n, int stride) {
    float total = 0.0f;
    float *ptr = data;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < n; i++) {
        total += *ptr;    /* *(ptr + 0) */
        ptr += stride;    /* Candidate increment for find_inc */
    }
    
    data[0] = total;
}

/* Pattern 5: Struct access with pointer increment */
struct Item {
    int id;
    float value;
    char tag;
};

void pattern5(struct Item *items, int n) {
    struct Item *current = items;
    
    /* Access struct fields, then increment pointer */
    for (int i = 0; i < n; i++) {
        current->id = i;      /* Should be *(current + 0) for id field */
        current->value = i * 1.5f;
        current++;           /* Separate increment */
    }
}

/* Pattern 6: Multiple memory references with same base */
void pattern6(int *dest, int *src, int n) {
    int *d = dest;
    int *s = src;
    
    /* Two memory accesses with offset 0, then increments */
    for (int i = 0; i < n; i++) {
        *d = *s;    /* Both *(d + 0) and *(s + 0) */
        d++;
        s++;
    }
}

/* Pattern 7: Loop with compile-time constant bounds */
void pattern7(void) {
    int local_array[SIZE];
    
    /* Compile-time constant bound helps optimization */
    for (int i = 0; i < SIZE; i++) {
        local_array[i] = i * 2;
    }
}

/* Pattern 8: Pointer arithmetic in loop condition */
void pattern8(int *arr, int n) {
    int *end = arr + n;
    int sum = 0;
    
    /* Pointer comparison in condition */
    for (int *p = arr; p < end; p++) {
        sum += *p;  /* *(p + 0) */
    }
    
    arr[0] = sum;
}

/* Main function to exercise all patterns */
int main(void) {
    int array1[SIZE];
    char buffer[SIZE];
    int matrix[10][INNER_SIZE];
    float data[SIZE];
    struct Item items[SIZE];
    int array2[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        buffer[i] = 'A' + (i % 26);
        data[i] = i * 0.5f;
        items[i].id = i;
        items[i].value = i * 2.0f;
        items[i].tag = 'X';
        array2[i] = SIZE - i;
    }
    
    /* Exercise all patterns */
    pattern1(array1, SIZE);
    pattern2(buffer, SIZE);
    pattern3(matrix, 10, INNER_SIZE);
    pattern4(data, SIZE, 2);
    pattern5(items, SIZE);
    pattern6(array1, array2, SIZE);
    pattern7();
    pattern8(array1, SIZE);
    
    /* Return something based on results to prevent optimization */
    return array1[0] + buffer[0] + (int)data[0] + items[0].id;
}
