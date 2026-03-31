#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
volatile int global_seed;

/* Different loop variants to increase coverage probability */

NOOPT void loop_decrement_for(int n, volatile int* result) {
    int sum = 0;
    /* Pattern: for (reg != 0; reg--) */
    for (int i = n; i != 0; i--) {
        sum += i * 3;
        *result = sum;  /* Side effect to prevent dead code elimination */
    }
    *result = sum;
}

NOOPT void loop_decrement_while_predec(int n, volatile int* result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: while (--cnt != 0) */
    while (--cnt != 0) {
        sum += cnt * 5;
        *result = sum;
    }
    *result = sum;
}

NOOPT void loop_decrement_while_postdec(int n, volatile int* result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: while (cnt-- != 0) */
    while (cnt-- != 0) {
        sum += (cnt + 1) * 7;
        *result = sum;
    }
    *result = sum;
}

NOOPT void loop_decrement_do_while(int n, volatile int* result) {
    int sum = 0;
    int cnt = n;
    /* Pattern: do { ... } while (--cnt != 0); */
    if (cnt > 0) {
        do {
            sum += cnt * 11;
            *result = sum;
        } while (--cnt != 0);
    }
    *result = sum;
}

NOOPT void loop_decrement_for_complex(int n, volatile int* result) {
    int sum = 0;
    /* Pattern with arithmetic in condition: for (int i = n; (i - 1) != -1; i--) */
    for (int i = n; i != 0; ) {
        sum += i * 13;
        *result = sum;
        i--;
    }
    *result = sum;
}

/* Helper to make loop bound non-constant at compile time */
int get_loop_bound(void) {
    /* Use multiple sources to prevent constant propagation */
    volatile int v = global_seed;
    int from_time = time(NULL) & 0xFF;
    int from_rand = rand() & 0x7F;
    
    /* Ensure bound is reasonable but not trivially small */
    int bound = 100 + (v % 50) + (from_time % 30) + (from_rand % 20);
    return bound & 0xFF;  /* Keep within 0-255 range */
}

int main(int argc, char* argv[]) {
    /* Initialize with non-constant value */
    global_seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(global_seed);
    
    int loop_bound = get_loop_bound();
    printf("Using loop bound: %d\n", loop_bound);
    
    volatile int results[5] = {0};
    int checksum = 0;
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound, &results[0]);
    loop_decrement_while_predec(loop_bound, &results[1]);
    loop_decrement_while_postdec(loop_bound, &results[2]);
    loop_decrement_do_while(loop_bound, &results[3]);
    loop_decrement_for_complex(loop_bound, &results[4]);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional variant with array access to force register usage */
    {
        volatile int local_result = 0;
        int arr[256];
        for (int i = 0; i < 256; i++) arr[i] = i * 2;
        
        int cnt = loop_bound;
        while (cnt-- != 0) {
            local_result += arr[cnt];
        }
        printf("Array variant result: %d\n", local_result);
    }
    
    return 0;
}
