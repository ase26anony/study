#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 200

/* Function with ARM target attribute to enable hardware loop optimization */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int early_exit_trigger = 0;
    volatile int outer_limit = N;
    int inner_limit;
    
    /* First nested loop: for-for with complex control flow */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-invariant calculation */
        int scale = i * 2 + 1;
        inner_limit = (i < N/2) ? i + 5 : N/3;
        
        /* Multiple basic blocks created by if-else */
        if (i % 3 == 0) {
            scale *= 2;
        } else if (i % 3 == 1) {
            scale += 10;
        } else {
            scale -= 5;
        }
        
        /* Inner loop with early exit condition */
        for (int j = 0; j < inner_limit; ++j) {
            /* Complex conditional with multiple basic blocks */
            if (j % 7 == 0) {
                arr[i][j] += scale * j;
            } else if (j % 7 == 1) {
                arr[i][j] -= scale;
            } else {
                arr[i][j] *= (scale % 10);
            }
            
            /* Early exit based on volatile variable */
            if (early_exit_trigger && j > inner_limit/2) {
                break;
            }
            
            /* Additional conditional continue */
            if (j % 11 == 0) {
                continue;
            }
            
            /* Switch statement creating more basic blocks */
            switch (j % 4) {
                case 0: arr[i][j] += 1; break;
                case 1: arr[i][j] -= 1; break;
                case 2: arr[i][j] *= 2; break;
                default: arr[i][j] /= 2; break;
            }
        }
    }
    
    /* Second nested loop: while-for with different pattern */
    int k = 0;
    volatile int while_limit = N * 2;
    
    while (k < while_limit) {
        int row = k % SIZE;
        int col_start = (k * 3) % SIZE;
        
        /* Loop-invariant code */
        int base = k * k + 10;
        
        /* Inner for loop */
        for (int col = col_start; col < SIZE && col < col_start + 10; ++col) {
            if (row % 2 == 0) {
                arr[row][col] = (arr[row][col] + base) % 1000;
            } else {
                arr[row][col] = (arr[row][col] - base) % 1000;
            }
            
            /* Conditional break with volatile check */
            volatile int random_break = rand() % 1000;
            if (random_break > 950) {
                break;
            }
        }
        
        k += (rand() % 3) + 1;  /* Variable increment */
    }
}

/* Another function with different loop structure */
__attribute__((target("arch=armv8-a")))
void triple_nested_loops(int M, int arr[SIZE][SIZE]) {
    volatile int limit1 = M;
    volatile int limit2 = M/2;
    int limit3;
    
    /* Triple nested loops for more complex bitmap analysis */
    for (int a = 0; a < limit1; ++a) {
        limit3 = (a % 5) + 3;
        
        for (int b = 0; b < limit2; ++b) {
            /* Multiple if conditions creating basic blocks */
            if (a > b) {
                int temp = a - b;
                
                for (int c = 0; c < limit3; ++c) {
                    /* Complex conditional chain */
                    if (c % 2 == 0) {
                        arr[a][b] += temp * c;
                        if (c % 4 == 0) {
                            arr[a][b] *= 2;
                        }
                    } else {
                        arr[a][b] -= temp;
                    }
                    
                    /* Early continue */
                    if (c == limit3/2) {
                        continue;
                    }
                    
                    /* Another conditional */
                    if (arr[a][b] > 10000) {
                        arr[a][b] = 10000;
                    }
                }
            } else {
                /* Different path with its own inner loop */
                for (int c = 0; c < 5; ++c) {
                    arr[a][b] = (arr[a][b] + c) % 500;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for non-constant loop bounds */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE) N = SIZE;
    
    /* Initialize array with pseudo-random data */
    int arr[SIZE][SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Call functions with nested loops */
    nested_loops_arm(N, arr);
    triple_nested_loops(N/2, arr);
    
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
