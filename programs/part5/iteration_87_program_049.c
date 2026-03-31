/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, short *c, char *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (short)(lcg_rand() % 256);
        d[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int size) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    short result4 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* Creates: result1 = ((result1 * a[i]) + b[i]) & 0xFFF */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 & 0xFFF;  /* Creates dependency chain with mask */
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    /* int + short * char operations with condition */
    int threshold = 500;
    for (int i = 0; i < size; i++) {
        if (a[i] > threshold) {
            result2 += a[i] * (int)c[i];  /* int * short promotion */
        } else {
            result2 += b[i] | (int)d[i];  /* int | char promotion */
        }
    }
    
    /* Loop 3: Independent reduction with bitwise operations */
    /* Creates parallel dependency chains */
    int temp = 0;
    for (int i = 0; i < size; ++i) {
        temp = temp ^ a[i];      /* XOR chain */
        result3 = result3 | temp; /* OR with previous result */
        temp = temp + b[i];      /* ADD chain */
    }
    
    /* Loop 4: Short type loop with data-dependent condition */
    /* Multiple uses of variables in loop */
    short acc = 0;
    short prev = 0;
    for (int i = 0; i < size; i++) {
        short current = c[i];
        if (current > prev) {           /* Data-dependent condition */
            acc = acc + current * 2;    /* Multiply-add chain */
        } else {
            acc = acc - current / 2;    /* Divide-subtract chain */
        }
        prev = current;                 /* Loop-carried dependency */
        result4 = result4 ^ acc;        /* XOR accumulation */
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + (int)result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array_b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    short *array_c = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char *array_d = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Call work function - this is where selective scheduling should occur */
    int final_result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Prevent dead code elimination without using volatile in loops */
    volatile int sink = final_result;
    
    /* Use result to prevent optimization */
    if (sink != 0) {
        printf("Result: %d\n", sink);
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
