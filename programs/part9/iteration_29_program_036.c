/* auto_inc_test.c - Test program for auto-increment/decrement recognition */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer operations */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Critical pattern: dereference ptr, then increment
         * Should generate (mem (plus (reg) (const_int 0))) */
        sum += *ptr;
        ptr += 1;  /* Post-increment - may become (reg + 0) in RTL */
    }
    
    /* Use volatile to prevent dead code elimination */
    dummy_volatile = sum;
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void write_data(int *dest, const int *src, int count) {
    int *dptr = dest;
    const int *sptr = src;
    int i;
    
    /* Loop 2: Write using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Similar pattern for store operation */
        *dptr = *sptr;
        dptr += 1;
        sptr += 1;
    }
    
    /* Force side-effect */
    dummy_volatile = (int)(dptr - dest);
}

int main(int argc, char *argv[]) {
    int count = 100;
    int *data;
    int result;
    int i;
    
    /* Use command line argument to make count less predictable */
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0 || count > 10000) {
            count = 100;
        }
    }
    
    /* Allocate and initialize array */
    data = (int *)malloc(count * sizeof(int));
    if (!data) {
        return 1;
    }
    
    for (i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call the critical functions */
    result = process_data(data, count);
    
    /* Create destination array for write test */
    int *dest = (int *)malloc(count * sizeof(int));
    if (dest) {
        write_data(dest, data, count);
        
        /* Verify and use result to prevent elimination */
        int verify = 0;
        for (i = 0; i < count; i++) {
            verify += dest[i];
        }
        
        printf("Read sum: %d, Write sum: %d\n", result, verify);
        free(dest);
    } else {
        printf("Result: %d\n", result);
    }
    
    free(data);
    return 0;
}
