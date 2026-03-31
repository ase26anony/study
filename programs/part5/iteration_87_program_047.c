/* Selective scheduling test program targeting sel-sched-dump.cc debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, short *c, char *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = (short)(lcg_rand() % 256);
        d[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int n) {
    int result1 = 0, result2 = 0, result3 = 0;
    int temp1, temp2;
    short temp_short;
    char temp_char;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    /* Creates a tight dependency chain: result1 = (result1 * a[i]) + b[i] */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed operations with conditional accumulation */
    /* Uses different data types and simple conditions */
    for (int i = 0; i < n; i++) {
        temp_short = c[i];
        if (temp_short > 100) {
            result2 += temp_short * a[i];
        } else {
            result2 += temp_short | b[i];
        }
    }
    
    /* Loop 3: Independent parallel chains with bitwise operations */
    /* Two independent chains that get combined */
    temp1 = 0x5A5A5A5A;
    temp2 = 0xA5A5A5A5;
    for (int i = 0; i < n; i++) {
        temp1 = (temp1 ^ a[i]) + (temp1 & b[i]);
        temp2 = (temp2 | b[i]) - (temp2 ^ a[i]);
    }
    result3 = temp1 ^ temp2;
    
    /* Loop 4: Character processing with promotion/demotion */
    /* Mixes char and int operations */
    int char_sum = 0;
    for (int i = 0; i < n; i++) {
        temp_char = d[i];
        if (temp_char < 64) {
            char_sum += (int)temp_char * 2;
        } else {
            char_sum += (int)temp_char / 2;
        }
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + char_sum;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    short *array_c = (short*)malloc(SIZE * sizeof(short));
    char *array_d = (char*)malloc(SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function with selective scheduling opportunities */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination without preventing scheduling */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
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
