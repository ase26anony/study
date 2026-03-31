/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv8-a")))
#else
#define TARGET_ARM
#endif

/* Volatile variables to prevent constant propagation */
volatile int N = 100;
volatile int M = 50;
volatile int K = 75;

/* Arrays for side effects */
int array1[200];
int array2[200];
int array3[200];

/* Function containing complex loop patterns for CFG analysis */
TARGET_ARM __attribute__((noinline,hot))
void test_loop_patterns(void) {
    volatile int i, j, k;
    int sum = 0;
    
    /* Loop 1: Simple countable loop (disjoint from others initially) */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        sum += array1[i];
    }
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    /* This should create bitmap where Loop2 ⊆ Loop3 */
    for (j = 0; j < M; j++) {
        array2[j] = j * 3;
        sum += array2[j];
        
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < K; k++) {
            array3[k] = k * 4;
            sum += array3[k];
            
            /* Small conditional to add basic blocks */
            if (k % 2 == 0) {
                array3[k] += 1;
            }
        }
    }
    
    /* Loop 4: Do-while loop for CFG variation */
    i = 0;
    do {
        array1[i] += sum;
        i++;
    } while (i < N/2);
    
    /* Loop 5: Creates partial overlap with Loop 6 via goto */
    /* This triggers bitmap_intersect_compl_p checks */
    for (i = 0; i < N; i++) {
        array1[i] = array1[i] * 2;
        
        /* Conditional that can jump into Loop 6 */
        if (array1[i] > 1000 && i < M) {
            /* Label for goto target */
            overlap_target:
            array2[i] = array1[i] / 2;
            /* Continue in Loop 5 */
        }
        
        sum += array1[i];
    }
    
    /* Loop 6: Partially overlaps with Loop 5 */
    for (j = 10; j < N - 10; j++) {
        array2[j] = j * j;
        
        /* This goto creates CFG edge from Loop 6 to inside Loop 5 */
        if (j == 15) {
            goto overlap_target;  /* Creates partial overlap */
        }
        
        /* Another basic block in Loop 6 */
        if (array2[j] % 2 == 0) {
            array2[j] += 1;
        }
        
        sum += array2[j];
    }
    
    /* Loop 7: While loop with complex exit condition */
    i = 0;
    while (i < K) {
        array3[i] = sum - i;
        
        /* Nested conditional to increase basic blocks */
        if (array3[i] > 500) {
            array3[i] = 500;
            /* Early continue creates additional CFG edges */
            i++;
            continue;
        }
        
        /* Another conditional that could be optimized */
        if (i % 3 == 0) {
            array3[i] *= 2;
        }
        
        i++;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum) : "memory");
}

/* Additional function with sibling loops */
TARGET_ARM __attribute__((noinline))
void sibling_loops(void) {
    volatile int x, y;
    int temp = 0;
    
    /* Loop A and Loop B are adjacent but disjoint */
    for (x = 0; x < 30; x++) {
        array1[x + 70] = x * x;
        temp += array1[x + 70];
    }
    
    for (y = 0; y < 30; y++) {
        array2[y + 70] = y * y * y;
        temp -= array2[y + 70];
    }
    
    /* Loop C contains an if with internal label */
    for (x = 0; x < 40; x++) {
        if (x % 5 == 0) {
            internal_label:
            array3[x] = temp + x;
        } else {
            array3[x] = temp - x;
        }
        
        /* Jump to label creates self-loop edge in CFG */
        if (x == 25) {
            goto internal_label;
        }
    }
    
    asm volatile("" : : "r"(temp) : "memory");
}

int main(void) {
    int i;
    int checksum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < 200; i++) {
        array1[i] = 0;
        array2[i] = 0;
        array3[i] = 0;
    }
    
    /* Call functions with complex loop patterns */
    test_loop_patterns();
    sibling_loops();
    
    /* Compute checksum to ensure all loops executed */
    for (i = 0; i < 200; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF;  /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to affect return value */
    return checksum == 0 ? 0 : 1;
}
