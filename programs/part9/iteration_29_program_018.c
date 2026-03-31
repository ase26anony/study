/* auto_inc_trigger.c - Trigger auto-inc/dec recognition for (reg + 0) pattern */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *p = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment - should generate (reg + 0) */
    for (i = 0; i < count; i++) {
        /* Direct pointer dereference before increment */
        sum += *p;      /* This should become (mem (plus (reg p) (const_int 0))) */
        p++;            /* Post-increment */
        
        /* Use volatile to prevent loop optimization */
        if (dummy_volatile) break;
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(int *data, int count, int value) {
    int *p = data;
    int i;
    
    /* Loop 2: Write using pointer post-increment */
    for (i = 0; i < count; i++) {
        *p = value + i; /* Write pattern - should also generate (reg + 0) */
        p++;            /* Post-increment */
    }
}

int main(int argc, char *argv[]) {
    int count = 100;
    int *array;
    int result;
    
    /* Get count from argv to make it non-constant for optimizer */
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    array = (int*)malloc(count * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with sequential values */
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the processing function - this contains the critical pattern */
    result = process_data(array, count);
    
    /* Also call the write function */
    modify_data(array, count, 10);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("First element after modification: %d\n", array[0]);
    
    free(array);
    return 0;
}
