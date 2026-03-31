/* test_auto_inc_dec.c
 * Compile with: gcc -O1 -c test_auto_inc_dec.c -o test.o
 * Or: gcc -O2 -fno-unroll-loops -c test_auto_inc_dec.c -o test.o
 */

#define SIZE 100

int main(void) {
    int array[SIZE];
    int sum = 0;
    
    /* Pattern 1: Simple pointer traversal with post-increment
     * This often generates: mem = *(ptr + 0); ptr = ptr + 4;
     */
    int *ptr = array;
    for (int i = 0; i < SIZE; i++) {
        sum += *ptr;  /* Access with base + 0 offset */
        ptr++;        /* Separate increment instruction */
    }
    
    /* Pattern 2: Indexed access where ivopts creates pointer induction */
    int buffer[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = 0;  /* May become: *(base + 0) = 0; base = base + 4; */
    }
    
    /* Pattern 3: Pointer arithmetic with explicit stride */
    int *p = array;
    for (int i = 0; i < SIZE; i++) {
        *p = i;        /* *(base + 0) = i */
        p += 1;        /* base = base + 4 */
    }
    
    /* Pattern 4: Nested loops with invariant base in inner loop */
    int matrix[10][SIZE];
    for (int row = 0; row < 10; row++) {
        int *base = &matrix[row][0];  /* Base computed in outer loop */
        for (int col = 0; col < SIZE; col++) {
            base[col] = row * col;    /* *(base + 0) after optimization? */
        }
    }
    
    /* Pattern 5: Different data types to test various strides */
    char char_array[SIZE];
    char *cptr = char_array;
    for (int i = 0; i < SIZE; i++) {
        *cptr = 'A';  /* *(base + 0) with stride 1 */
        cptr++;
    }
    
    /* Pattern 6: Struct access to test larger strides */
    struct point { int x; int y; };
    struct point points[SIZE];
    struct point *pptr = points;
    for (int i = 0; i < SIZE; i++) {
        pptr->x = i;   /* May become: *(base + 0) = i */
        pptr->y = i * 2;
        pptr++;        /* Increment by sizeof(struct point) */
    }
    
    /* Pattern 7: Reverse traversal to test auto-decrement */
    int *rptr = &array[SIZE - 1];
    for (int i = SIZE - 1; i >= 0; i--) {
        sum += *rptr;  /* *(base + 0) */
        rptr--;        /* base = base - 4 */
    }
    
    /* Pattern 8: Mixed access pattern to avoid over-optimization */
    int data[SIZE * 2];
    int *dptr = data;
    for (int i = 0; i < SIZE; i++) {
        /* Two separate accesses with same base */
        dptr[0] = i;      /* First access: *(base + 0) */
        dptr[SIZE] = i * 2; /* Second access with different offset */
        dptr++;           /* Increment base */
    }
    
    return sum;  /* Prevent dead code elimination */
}
