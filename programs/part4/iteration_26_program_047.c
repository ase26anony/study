/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto_inc_dec optimization pass
 * Specifically targets the find_inc(true) path with reg1_val = 0
 */

#define SIZE 100
#define INNER_SIZE 50

/* Simple struct to test with non-trivial but aligned types */
struct element {
    int value;
    int tag;
};

/* Function to prevent dead code elimination */
void use_result(int sum) {
    volatile int sink = sum;
    (void)sink;
}

/* Function to process structs */
void process_struct(struct element *e) {
    e->value += e->tag;
}

int main(void) {
    int array[SIZE];
    int matrix[SIZE][INNER_SIZE];
    struct element struct_array[SIZE];
    
    int sum = 0;
    
    /* Pattern 1: Simple pointer traversal with post-increment
     * Should generate: base + 0 addressing in loop body
     */
    {
        int *ptr = array;
        int *end = array + SIZE;
        
        /* Initialize array */
        for (int i = 0; i < SIZE; i++) {
            array[i] = i;
        }
        
        /* Loop that should trigger auto_inc_dec */
        while (ptr < end) {
            sum += *ptr;    /* Should become *(ptr + 0) after optimization */
            ptr++;          /* Separate increment instruction for find_inc */
        }
    }
    
    /* Pattern 2: Indexed access where ivopts creates pointer with zero offset
     * Compiler's ivopts pass may convert index to pointer + 0
     */
    {
        int buffer[SIZE];
        
        for (int i = 0; i < SIZE; i++) {
            buffer[i] = 0;  /* May become *(ptr + 0) after ivopts */
        }
        
        /* Verify initialization */
        for (int i = 0; i < SIZE; i++) {
            sum += buffer[i];
        }
    }
    
    /* Pattern 3: Nested loops with invariant base in inner loop
     * Inner loop accesses use base + 0, outer loop modifies base
     */
    {
        for (int j = 0; j < SIZE; j++) {
            int *base = &matrix[j][0];  /* Base computed in outer loop */
            
            for (int i = 0; i < INNER_SIZE; i++) {
                base[i] = i;  /* Inner loop: base + 0 addressing expected */
            }
        }
        
        /* Verify matrix values */
        for (int j = 0; j < SIZE; j++) {
            for (int i = 0; i < INNER_SIZE; i++) {
                sum += matrix[j][i];
            }
        }
    }
    
    /* Pattern 4: Pointer arithmetic with explicit stride
     * Separate increment instruction clearly visible
     */
    {
        int *ptr = array;
        const int stride = 1;  /* Compile-time constant */
        
        for (int i = 0; i < SIZE; i++) {
            sum += *ptr;        /* Memory access with base + 0 */
            ptr += stride;      /* Explicit increment for find_inc */
        }
    }
    
    /* Pattern 5: Struct array traversal
     * Tests with non-trivial but aligned type
     */
    {
        /* Initialize struct array */
        for (int i = 0; i < SIZE; i++) {
            struct_array[i].value = i;
            struct_array[i].tag = i % 10;
        }
        
        struct element *sptr = struct_array;
        struct element *send = struct_array + SIZE;
        
        while (sptr < send) {
            process_struct(sptr);  /* Function call prevents some optimizations */
            sptr++;                /* But increment still separate */
        }
    }
    
    /* Pattern 6: Multiple memory references in same loop
     * Tests the pass's ability to handle multiple candidates
     */
    {
        int src[SIZE];
        int dst[SIZE];
        
        /* Initialize source */
        for (int i = 0; i < SIZE; i++) {
            src[i] = i;
        }
        
        int *s = src;
        int *d = dst;
        int *end = src + SIZE;
        
        while (s < end) {
            *d = *s;    /* Two memory references, both base + 0 */
            s++;
            d++;
        }
        
        /* Verify copy */
        for (int i = 0; i < SIZE; i++) {
            sum += dst[i];
        }
    }
    
    /* Pattern 7: Loop with if condition but linear access pattern
     * Conditional shouldn't prevent auto_inc_dec optimization
     */
    {
        int data[SIZE];
        int *ptr = data;
        
        for (int i = 0; i < SIZE; i++) {
            data[i] = i * 2;
        }
        
        for (int i = 0; i < SIZE; i++) {
            if (*ptr > 0) {     /* Memory access with base + 0 */
                sum += *ptr;
            }
            ptr++;              /* Separate increment */
        }
    }
    
    use_result(sum);
    
    return 0;
}
