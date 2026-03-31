#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to enable hardware loop optimizations */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, volatile int break_early) {
    int arr1[SIZE][SIZE];
    int arr2[SIZE][SIZE];
    int result[SIZE][SIZE];
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr1[i][j] = (i * 17 + j * 13) % 100;
            arr2[i][j] = (i * 11 + j * 19) % 100;
        }
    }
    
    /* First nested loop structure: for-for with complex control flow */
    for (volatile int i = 0; i < N; ++i) {
        /* Loop-invariant calculation that varies with outer loop */
        int scale = i * 2 + 1;
        
        /* Inner loop with dependency on outer loop index */
        for (int j = 0; j < i + 1; ++j) {
            /* Multiple basic blocks created by if-else */
            if (j % 3 == 0) {
                result[i][j] = arr1[i][j] * scale + arr2[i][j];
            } else if (j % 3 == 1) {
                result[i][j] = arr1[i][j] + arr2[i][j] * scale;
            } else {
                result[i][j] = arr1[i][j] - arr2[i][j];
            }
            
            /* Early exit based on volatile condition */
            if (break_early && j > N/2) {
                break;
            }
            
            /* Additional conditional continue */
            if (result[i][j] > 1000) {
                continue;
            }
        }
        
        /* Switch statement to create more basic blocks */
        switch (i % 4) {
            case 0:
                arr1[i][0] += 1;
                break;
            case 1:
                arr1[i][0] -= 1;
                break;
            case 2:
                arr1[i][0] *= 2;
                break;
            default:
                arr1[i][0] /= 2;
                break;
        }
    }
    
    /* Second distinct loop nest: while-for with different pattern */
    int k = 0;
    while (k < N) {
        volatile int limit = k * 2;
        if (limit > SIZE) limit = SIZE;
        
        /* Inner for loop */
        for (int m = 0; m < limit; ++m) {
            /* Conditional that creates separate basic blocks */
            if (k % 2 == 0) {
                result[k][m] += arr2[m][k];
            } else {
                result[k][m] -= arr2[m][k];
            }
            
            /* Another early break possibility */
            if (break_early && result[k][m] < -1000) {
                break;
            }
        }
        
        /* Loop-invariant code in outer loop */
        int adjustment = (k * k) % 10;
        for (int n = 0; n < SIZE; ++n) {
            arr1[k][n] += adjustment;
        }
        
        k += (break_early ? 2 : 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            checksum += result[i][j];
        }
    }
    printf("Checksum 1: %lld\n", checksum);
}

/* Another function with different loop structure */
__attribute__((target("arch=armv8-a")))
void complex_nested_loops(int M, volatile int flag) {
    int matrix[SIZE][SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            matrix[i][j] = i * SIZE + j;
        }
    }
    
    /* Triple nested loops */
    for (int a = 0; a < M; ++a) {
        int outer_var = a * 3;
        
        for (int b = a; b < M; ++b) {
            /* Multiple conditionals creating basic blocks */
            if (b % 2 == 0) {
                for (int c = 0; c < b; ++c) {
                    matrix[a][c] += outer_var;
                    
                    /* Conditional break with volatile */
                    if (flag && c > M/3) {
                        break;
                    }
                    
                    /* Nested if-else */
                    if (matrix[a][c] % 7 == 0) {
                        matrix[a][c] /= 2;
                    } else {
                        matrix[a][c] *= 2;
                    }
                }
            } else {
                for (int c = b; c > 0; --c) {
                    matrix[b][c] -= outer_var;
                }
            }
        }
        
        /* Do-while loop inside for */
        int d = 0;
        do {
            matrix[a][d] += a * d;
            d++;
        } while (d < a + 5 && d < SIZE);
    }
    
    /* Checksum */
    long long sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            sum += matrix[i][j];
        }
    }
    printf("Checksum 2: %lld\n", sum);
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bounds from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    volatile int break_flag = (argc > 2) ? atoi(argv[2]) : 1;
    
    srand(time(NULL));
    
    /* Call functions with hardware loop target attributes */
    nested_loops_arm(N, break_flag);
    complex_nested_loops(N / 2, break_flag);
    
    /* Additional loop nest in main without attribute 
       (compiler may still analyze it with -march flag) */
    int arr[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        arr[i] = rand() % 100;
    }
    
    volatile int temp = 0;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        while (j < i + 1) {
            arr[j] += arr[i];
            if (temp && j > N/4) {
                break;
            }
            j += (i % 3) + 1;
        }
    }
    
    int final_sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        final_sum += arr[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
