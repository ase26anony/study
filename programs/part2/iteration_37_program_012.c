/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdint.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#define TARGET_ARM
#else
/* Use attribute to target ARM if compiling for ARM */
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N, int* results) {
    volatile int M = N / 2;
    volatile int K = N / 3;
    
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        results[i] = i * 2;
        /* Simple side effect to prevent removal */
        asm volatile("" : : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < M; j++) {
        results[j + N] = j * 3;
        asm volatile("" : : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside conditional - creates subset relationship */
    int outer_counter = 0;
    while (outer_counter < K) {
        /* Loop 3a: Inner loop - will be subset of Loop 3's blocks */
        for (k = 0; k < 5; k++) {
            results[outer_counter * 5 + k] += k;
            asm volatile("" : : : "memory");
        }
        outer_counter++;
    }
    
    /* Loop 4 and 5: Partially overlapping loops using goto */
    /* This creates the bitmap_intersect_compl_p condition */
    int x = 0;
    int y = 0;
    
    /* Loop 4 */
loop4_start:
    if (x >= N) goto loop4_end;
    
    results[x] += x;
    asm volatile("" : : : "memory");
    
    /* Conditional jump into Loop 5's body */
    if (x % 3 == 0) {
        goto loop5_middle;  /* Jump into middle of another loop */
    }
    
    x++;
    goto loop4_start;
loop4_end:
    
    /* Loop 5 - partially overlaps with Loop 4 via goto */
    y = 0;
loop5_start:
    if (y >= M) goto loop5_end;
    
    results[y + 100] = y * y;
    asm volatile("" : : : "memory");
    
loop5_middle:  /* Label that Loop 4 can jump to */
    results[y + 100] += 1;
    asm volatile("" : : : "memory");
    
    y++;
    goto loop5_start;
loop5_end:
    
    /* Loop 6: Do-while loop for CFG variation */
    int z = 0;
    do {
        results[z + 200] = z % 7;
        asm volatile("" : : : "memory");
        z++;
    } while (z < 10);
}

/* Main function to ensure execution */
int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    int results[400] = {0};
    int i;
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    unsigned long long checksum = 0;
    for (i = 0; i < 400; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Additional dummy loops to increase chance of hw-doloop analysis */
    volatile int count = 50;
    int temp = 0;
    
    /* Another set of adjacent loops */
    for (i = 0; i < count; i++) {
        temp += i;
        asm volatile("" : : : "memory");
    }
    
    for (i = 0; i < count/2; i++) {
        temp -= i;
        asm volatile("" : : : "memory");
    }
    
    /* Nested loop with inner loop that could be subset */
    for (i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            temp += i * j;
            asm volatile("" : : : "memory");
        }
    }
    
    return (int)checksum + temp;
}

/* Alternative version with explicit ARM target attribute */
#ifdef FORCE_ARM_TARGET
__attribute__((target("arch=armv8-a")))
void test_loops_arm() {
    volatile int n = 100;
    int arr[300];
    
    /* Loop A */
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
    
    /* Loop B - starts after A, should be disjoint */
    for (int i = 0; i < n/2; i++) {
        arr[i + 100] = i * 2;
    }
    
    /* Loop C - contains conditional with inner loop */
    int c = 0;
    while (c < n) {
        if (c % 2 == 0) {
            /* Inner loop D - subset of C */
            for (int d = 0; d < 5; d++) {
                arr[c * 5 + d] += d;
            }
        }
        c++;
    }
    
    /* Complex overlapping with switch and goto */
    int x = 0, y = 0;
    
loop_x:
    if (x >= 50) goto end_x;
    switch (x % 4) {
        case 0:
            goto overlap_point;
        case 1:
            arr[x] = 1;
            break;
        default:
            arr[x] = 2;
    }
    x++;
    goto loop_x;

overlap_point:
    arr[x] = 3;
    /* Fall through into loop_y */
    
loop_y:
    if (y >= 30) goto end_y;
    arr[y + 150] = y;
    y++;
    goto loop_y;

end_y:
    x++;
    goto loop_x;
    
end_x:
    return;
}
#endif
