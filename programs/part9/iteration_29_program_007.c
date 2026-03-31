/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile pointer to prevent optimization of pointer arithmetic */
static volatile int *volatile_ptr;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(int *data, int count) {
    int *p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference - should generate (reg + 0) pattern */
        sum += *p;
        p += 1;  /* Post-increment - separate from dereference */
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Another (reg + 0) pattern opportunity */
        *p = sum + i;
        p += 1;
    }
    
    /* Use volatile pointer to block optimization */
    volatile_ptr = data;
    
    return sum;
}

/* Another non-inline function with different pattern */
__attribute__((noinline))
static int process_data_alt(int *data, int count) {
    int *p = data;
    int sum = 0;
    int i = 0;
    
    /* While loop with pointer dereference then increment */
    while (i < count) {
        /* This should create the (reg + 0) address pattern */
        int val = *p;
        sum ^= val;  /* Use XOR to prevent easy optimization */
        p = p + 1;   /* Increment separately */
        i++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Dynamic allocation prevents static analysis */
    int *data = (int*)malloc(count * sizeof(int));
    if (!data) return 1;
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < count; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Call the processing functions */
    int result1 = process_data(data, count);
    int result2 = process_data_alt(data, count);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    /* Use the data one more time */
    volatile int final_check = data[count/2];
    printf("Final check: %d\n", final_check);
    
    free(data);
    return 0;
}
