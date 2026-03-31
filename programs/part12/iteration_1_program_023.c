#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARMv8-A target to ensure hardware loop pass runs */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE], volatile int limit_modifier) {
    volatile int outer_limit = N + limit_modifier;
    volatile int early_exit_threshold = N / 3;
    
    /* First nested loop: for-for with complex control flow */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-invariant calculation */
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 2;
        
        /* Multiple basic blocks created by if-else */
        if (i % 2 == 0) {
            scale *= 2;
        } else {
            scale += 5;
        }
        
        /* Inner loop with conditional break */
        for (int j = 0; j < inner_limit; ++j) {
            arr[i][j] += scale * j;
            
            /* Early exit creates additional basic block */
            volatile int should_break = (j > early_exit_threshold);
            if (should_break && (j % 4 == 0)) {
                break;
            }
            
            /* Another conditional to create more blocks */
            if (j % 3 == 0) {
                arr[i][j] -= 1;
            } else {
                arr[i][j] += 2;
            }
        }
        
        /* Switch statement to create more basic blocks */
        switch (i % 4) {
            case 0: arr[i][0] *= 2; break;
            case 1: arr[i][0] /= 2; break;
            case 2: arr[i][0] += 100; break;
            default: arr[i][0] -= 50; break;
        }
    }
    
    /* Second nested loop: while-for with different pattern */
    int k = 0;
    volatile int while_limit = N - limit_modifier;
    if (while_limit < 0) while_limit = 5;
    
    while (k < while_limit) {
        int m = 0;
        volatile int inner_while_limit = k * 2 + 1;
        
        /* For loop inside while */
        for (m = 0; m < inner_while_limit; ++m) {
            arr[k][m] *= (k + 1);
            
            /* Nested if with continue */
            if (m % 5 == 0) {
                arr[k][m] += k;
                continue;
            }
            
            /* Another conditional break */
            volatile int break_cond = (m > N / 2);
            if (break_cond && (k % 2 == 0)) {
                break;
            }
        }
        
        /* Do-while to add more loop variety */
        int n = 0;
        do {
            arr[k][n % SIZE] -= n;
            n++;
        } while (n < 3);
        
        k++;
    }
}

/* Another function with different loop structure */
__attribute__((target("arch=armv8-a")))
void complex_nested_loops(int N, int arr[SIZE][SIZE], volatile int seed) {
    volatile int mod = seed % 10 + 5;
    
    /* Triple nested loops */
    for (int x = 0; x < mod; ++x) {
        int temp = x * x;
        for (int y = 0; y < x + 1; ++y) {
            /* Multiple conditionals in inner loop */
            if (y % 2 == 0) {
                for (int z = 0; z < y + 2; ++z) {
                    arr[x][y] += temp * z;
                    
                    /* Conditional with logical operators */
                    if (z > 3 && z < 7) {
                        arr[x][y] -= 10;
                    } else if (z == 0 || z == 9) {
                        arr[x][y] += 5;
                    }
                    
                    /* Early return-like break */
                    volatile int stop = (z > 8);
                    if (stop) {
                        break;
                    }
                }
            } else {
                for (int z = 0; z < 3; ++z) {
                    arr[x][y] -= z;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bounds from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE - 1) N = SIZE - 1;
    if (N < 10) N = 10;
    
    /* Initialize array with pseudo-random data */
    int arr[SIZE][SIZE];
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Volatile modifier to prevent optimization */
    volatile int limit_modifier = argc > 2 ? atoi(argv[2]) : 3;
    
    /* Call functions with nested loops */
    nested_loops_arm(N, arr, limit_modifier);
    complex_nested_loops(N, arr, limit_modifier);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
