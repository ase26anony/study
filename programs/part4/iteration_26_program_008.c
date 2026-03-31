/* test_auto_inc_dec.c
 * 
 * This program contains loop patterns designed to trigger the uncovered
 * lines in GCC's auto_inc_dec pass (lines 1352-1358 in auto-inc-dec.cc).
 * The patterns create memory accesses with base+0 addressing where
 * find_inc(true) can find a preceding increment instruction.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50

/* Pattern 1: Simple pointer traversal with post-increment
 * This often generates: mem = *(ptr + 0), followed by ptr = ptr + 4
 */
int pattern1_sum(const int *arr, int size) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + size;
    
    while (p < end) {
        /* Access with current pointer value (base + 0) */
        sum += *p;
        /* Explicit increment - separate instruction */
        p++;
    }
    return sum;
}

/* Pattern 2: Indexed array access where ivopts creates pointer induction
 * The compiler's ivopts pass may convert array[i] to pointer access
 */
void pattern2_zero_buffer(int *buffer, int size) {
    for (int i = 0; i < size; i++) {
        /* This may become: *(base + i*4) -> optimized to pointer with offset 0 */
        buffer[i] = 0;
        /* i++ creates separate increment that find_inc can match */
    }
}

/* Pattern 3: Pointer arithmetic with stride
 * Explicit pointer increment separate from access
 */
void pattern3_fill_sequence(int *arr, int size, int stride) {
    int *ptr = arr;
    for (int i = 0; i < size; i++) {
        *ptr = i;          /* Access at current ptr (base + 0) */
        ptr += stride;     /* Separate increment instruction */
    }
}

/* Pattern 4: Nested loops with invariant base in inner loop
 * Outer loop sets base, inner loop uses base with offset 0
 */
void pattern4_matrix_init(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        int *base = &matrix[j][0];  /* Base set in outer loop */
        for (int i = 0; i < N; i++) {
            /* Inner loop access: base[i] -> may become *(base + 0) */
            base[i] = i * j;
            /* i++ creates separate increment */
        }
    }
}

/* Pattern 5: Struct access with pointer increment
 * Tests with non-primitive types
 */
struct Data {
    int value;
    float weight;
    char tag;
};

void pattern5_struct_traversal(struct Data *data, int count) {
    struct Data *ptr = data;
    for (int i = 0; i < count; i++) {
        /* Access current struct (base + 0) */
        ptr->value = i;
        ptr->weight = i * 0.5f;
        /* Increment after access */
        ptr++;
    }
}

/* Pattern 6: Mixed access pattern to avoid over-optimization
 * Sometimes simpler patterns get optimized differently
 */
int pattern6_mixed_access(int *a, int *b, int size) {
    int sum = 0;
    int *pa = a;
    int *pb = b;
    
    for (int i = 0; i < size; i++) {
        /* Two memory accesses with base+0 pattern */
        int val_a = *pa;
        int val_b = *pb;
        sum += val_a * val_b;
        
        /* Separate increments */
        pa++;
        pb++;
    }
    return sum;
}

/* Pattern 7: Do-while loop for different control flow
 * Different loop structures may affect RTL generation
 */
void pattern7_downcounter(int *arr, int size) {
    int *ptr = arr + size - 1;
    int count = size;
    
    do {
        *ptr = count;      /* Access at ptr + 0 */
        ptr--;             /* Decrement - also test auto-decrement */
        count--;
    } while (count > 0);
}

/* Pattern 8: Char pointer traversal (different increment amount)
 * Tests with byte-sized increments
 */
void pattern8_char_fill(char *str, int len, char fill) {
    char *p = str;
    for (int i = 0; i < len; i++) {
        *p = fill;        /* base + 0 access */
        p++;              /* increment by 1 */
    }
}

/* Main function to exercise all patterns */
int main() {
    /* Initialize test data */
    int array1[N];
    int array2[N];
    int matrix[M][N];
    struct Data structs[N];
    char buffer[N];
    
    /* Initialize with some values */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        structs[i].value = 0;
        structs[i].weight = 0.0f;
        buffer[i] = 'A';
    }
    
    /* Exercise each pattern */
    int sum1 = pattern1_sum(array1, N);
    printf("Pattern1 sum: %d\n", sum1);
    
    pattern2_zero_buffer(array2, N);
    printf("Pattern2 zeroed buffer[0]: %d\n", array2[0]);
    
    pattern3_fill_sequence(array1, N, 1);
    printf("Pattern3 filled sequence[10]: %d\n", array1[10]);
    
    pattern4_matrix_init(matrix);
    printf("Pattern4 matrix[5][5]: %d\n", matrix[5][5]);
    
    pattern5_struct_traversal(structs, N);
    printf("Pattern5 structs[10].value: %d\n", structs[10].value);
    
    int sum6 = pattern6_mixed_access(array1, array2, N);
    printf("Pattern6 mixed sum: %d\n", sum6);
    
    pattern7_downcounter(array1, N);
    printf("Pattern7 downcounter[0]: %d\n", array1[0]);
    
    pattern8_char_fill(buffer, N, 'X');
    printf("Pattern8 buffer[0]: %c\n", buffer[0]);
    
    return 0;
}
