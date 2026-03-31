/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;  /* Local pointer that will be modified */
    
    /* Loop that should generate (reg + 0) addressing */
    for (int i = 0; i < count; i++) {
        /* Critical access: *p where p is a register
         * After optimization, this should become (mem (plus (reg) (const_int 0))) */
        sum += *p;
        
        /* Post-increment - the key to trigger auto-inc-dec recognition */
        p += 1;  /* Equivalent to p++ for int pointer */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
void __attribute__((noinline)) write_pattern(int *ptr, int count, int value) {
    int *p = ptr;
    
    /* Write loop with post-increment */
    for (int i = 0; i < count; i++) {
        *p = value + i;
        p += 1;  /* Post-increment after write */
    }
}

/* Third function with mixed read/write to increase chances */
int __attribute__((noinline)) transform_data(int *ptr, int count) {
    int *p = ptr;
    int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Read using pointer dereference */
        int val = *p;
        
        /* Simple transformation */
        val = val * 2 + global_seed;
        
        /* Write back using same pointer */
        *p = val;
        
        /* Accumulate */
        result += val;
        
        /* Post-increment */
        p += 1;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make size non-constant to optimizer */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Dynamic allocation prevents static analysis */
    int *data = (int*)malloc(size * sizeof(int));
    if (!data) return 1;
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i * 3 + global_seed;
    }
    
    /* Call the critical functions */
    write_pattern(data, size, global_seed);
    int sum1 = process_data(data, size);
    int sum2 = transform_data(data, size);
    
    /* Print results to prevent dead code elimination */
    printf("Sum1: %d, Sum2: %d\n", sum1, sum2);
    
    /* Use result in conditional to prevent optimization */
    if (sum1 != sum2) {
        printf("Results differ\n");
    }
    
    free(data);
    return 0;
}
