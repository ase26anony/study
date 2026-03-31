/* auto_inc_test.c - Test case for auto-inc-dec pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;  /* Local pointer that will be modified */
    
    /* Loop that should generate (reg + 0) addressing */
    for (int i = 0; i < count; i++) {
        /* Critical access: *p where p hasn't been incremented yet */
        sum += *p;      /* This should generate (mem (plus (reg p) (const_int 0))) */
        p += 1;         /* Post-increment - separate statement to encourage (reg+0) pattern */
        
        /* Use volatile to prevent loop optimizations */
        if (dummy_volatile) break;
    }
    
    return sum;
}

/* Second function with write pattern */
__attribute__((noinline))
static void write_pattern(int *ptr, int count, int value) {
    int *p = ptr;
    
    for (int i = 0; i < count; i++) {
        *p = value + i;  /* Write with (reg+0) addressing */
        p += 1;          /* Post-increment */
        
        if (dummy_volatile) break;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *data = (int*)malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call the critical functions */
    int sum = process_data(data, count);
    write_pattern(data, count, 42);
    
    /* Print results to prevent dead code elimination */
    printf("Sum: %d\n", sum);
    printf("First element: %d\n", data[0]);
    
    free(data);
    return 0;
}
