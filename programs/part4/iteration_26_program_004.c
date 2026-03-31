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
        sum += *p;      /* Should become *(p + 0) after optimization */
        p++;            /* Separate increment instruction */
    }
    
    /* Prevent dead code elimination */
    arr[0] = sum;
}

/* Pattern 2: Indexed array access with post-increment index */
void pattern2(char *buffer, int n) {
    int i = 0;
    
    /* Indexed access where ivopts may create pointer with offset 0 */
    while (i < n) {
        buffer[i] = 0;  /* May become *(base + 0) after optimization */
        i++;            /* Separate increment */
    }
}

/* Pattern 3: Nested loops with invariant base pointer */
void pattern3(int matrix[][INNER_SIZE], int rows) {
    for (int j = 0; j < rows; j++) {
        int *base = &matrix[j][0];  /* Base computed in outer loop */
        
        /* Inner loop accesses with base + 0 */
        for (int i = 0; i < INNER_SIZE; i++) {
            base[i] = i;  /* Should become *(base + 0) in loop body */
        }
    }
}

/* Pattern 4: Pointer arithmetic with explicit stride */
void pattern4(float *data, int count, int stride) {
    float total = 0.0f;
    float *ptr = data;
    
    /* Explicit increment separate from access */
    for (int i = 0; i < count; i++) {
        total += *ptr;   /* *(ptr + 0) */
        ptr += stride;   /* Candidate increment instruction */
    }
    
    data[0] = total;
}

/* Pattern 5: Struct access with pointer traversal */
struct Item {
    int id;
    float value;
    char tag;
};

void pattern5(struct Item *items, int n) {
    struct Item *current = items;
    
    /* Access struct through pointer, then increment */
    for (int i = 0; i < n; i++) {
        current->id = i;      /* Should be *(current + 0) for first field */
        current->value = i * 0.5f;
        current++;           /* Separate increment */
    }
}

/* Pattern 6: Mixed access patterns to encourage optimization */
void pattern6(int *a, int *b, int n) {
    int *pa = a;
    int *pb = b;
    
    /* Two separate pointer streams */
    for (int i = 0; i < n; i++) {
        *pa = *pb;    /* Both should be *(ptr + 0) */
        pa++;
        pb++;
    }
}

/* Pattern 7: Loop with pointer increment in middle */
void pattern7(unsigned char *buf, int len) {
    unsigned char *p = buf;
    int i = 0;
    
    while (i < len) {
        unsigned char val = *p;  /* *(p + 0) */
        i++;
        p++;                     /* Increment after use */
        
        /* Some computation to prevent over-optimization */
        buf[0] ^= val;
    }
}

/* Main function to ensure all patterns are used */
int main() {
    int array1[SIZE];
    char buffer[SIZE];
    int matrix[SIZE][INNER_SIZE];
    float data[SIZE];
    struct Item items[SIZE];
    int array2[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        buffer[i] = 'A' + (i % 26);
        data[i] = i * 0.1f;
        items[i].id = i;
        items[i].value = i * 0.5f;
        items[i].tag = 'a' + (i % 26);
        array2[i] = SIZE - i;
    }
    
    /* Call all patterns to ensure they're compiled */
    pattern1(array1, SIZE);
    pattern2(buffer, SIZE);
    pattern3(matrix, SIZE / 2);
    pattern4(data, SIZE, 1);
    pattern5(items, SIZE);
    pattern6(array1, array2, SIZE);
    pattern7((unsigned char *)buffer, SIZE);
    
    /* Return something based on results to prevent dead code elimination */
    return array1[0] + buffer[0] + (int)data[0] + items[0].id;
}
