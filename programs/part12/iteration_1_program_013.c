#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force hardware loop optimization for ARM */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#endif
void nested_loops_arm(int N, int *checksum) {
    volatile int outer_limit = N;
    volatile int early_exit_trigger = N / 3;
    int arr[200][200];
    
    /* Initialize with pseudo-random data */
    for (int x = 0; x < 200; ++x) {
        for (int y = 0; y < 200; ++y) {
            arr[x][y] = (x * 17 + y * 13) % 100;
        }
    }
    
    /* First nested loop structure: for-for with complex control flow */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-invariant calculation that varies with outer loop */
        int scale = i * 2 + 1;
        volatile int inner_early = early_exit_trigger;
        
        /* Multiple basic blocks created by if-else */
        if (i % 2 == 0) {
            scale *= 2;  /* Additional basic block */
        }
        
        /* Inner loop with dependency on outer index */
        for (int j = 0; j < i + 5; ++j) {
            /* Conditional break creating more CFG complexity */
            if (j > inner_early && (j % 7 == 0)) {
                break;  /* Early exit creates separate basic block */
            }
            
            /* Multiple basic blocks within inner loop */
            switch (j % 3) {
                case 0:
                    arr[i % 200][j % 200] += scale;
                    break;
                case 1:
                    arr[i % 200][j % 200] -= scale / 2;
                    break;
                default:
                    arr[i % 200][j % 200] *= (scale % 10) + 1;
                    if (arr[i % 200][j % 200] > 1000) {
                        arr[i % 200][j % 200] = 1000;
                    }
            }
            
            /* Another conditional continue */
            if (arr[i % 200][j % 200] < 0) {
                arr[i % 200][j % 200] = 0;
                continue;
            }
        }
    }
    
    /* Second distinct loop nest in same function: while-for */
    int k = 0;
    volatile int while_limit = N / 2;
    while (k < while_limit) {
        /* Loop-invariant code */
        int offset = k * 3;
        
        /* Inner for loop with different pattern */
        for (int m = 0; m < (k % 10) + 8; ++m) {
            /* Conditional with multiple basic blocks */
            if (m > 5) {
                arr[k % 200][m % 200] += offset;
                if (arr[k % 200][m % 200] % 2 == 0) {
                    arr[k % 200][m % 200] /= 2;
                }
            } else {
                arr[k % 200][m % 200] -= offset;
            }
            
            /* Another early break possibility */
            volatile int break_chance = 3;
            if (m > break_chance && (arr[k % 200][m % 200] > 500)) {
                break;
            }
        }
        
        /* Complex update with condition */
        if (k % 4 == 0) {
            k += 2;
        } else {
            k += 1;
        }
    }
    
    /* Compute checksum to prevent elimination */
    for (int x = 0; x < 200; ++x) {
        for (int y = 0; y < 200; ++y) {
            *checksum += arr[x][y];
        }
    }
}

/* Another function with different nesting pattern */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#endif
void triangular_loops_arm(int N, int *checksum) {
    volatile int dim = N;
    int matrix[150][150];
    
    /* Initialize */
    for (int i = 0; i < 150; ++i) {
        for (int j = 0; j < 150; ++j) {
            matrix[i][j] = i + j * 2;
        }
    }
    
    /* Triangular nested loops */
    for (int a = 0; a < dim; ++a) {
        /* Multiple basic blocks before inner loop */
        int threshold;
        if (a % 3 == 0) {
            threshold = a * 2;
        } else if (a % 3 == 1) {
            threshold = a + 10;
        } else {
            threshold = a / 2 + 5;
        }
        
        /* Inner loop with complex exit condition */
        for (int b = 0; b < a + 1; ++b) {
            /* Nested if-else creating more blocks */
            if (b < threshold) {
                matrix[a % 150][b % 150] += a * b;
                if (matrix[a % 150][b % 150] % 7 == 0) {
                    matrix[a % 150][b % 150] *= 2;
                }
            } else {
                matrix[a % 150][b % 150] -= a + b;
                /* Early continue in else branch */
                if (matrix[a % 150][b % 150] < 0) {
                    matrix[a % 150][b % 150] = 0;
                    continue;
                }
            }
            
            /* Volatile condition for break */
            volatile int stop = dim / 4;
            if (b > stop && (a + b) % 11 == 0) {
                break;
            }
        }
    }
    
    /* Add to checksum */
    for (int i = 0; i < 150; ++i) {
        for (int j = 0; j < 150; ++j) {
            *checksum += matrix[i][j];
        }
    }
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int total_checksum = 0;
    
    srand(time(NULL));
    
    /* Call functions with hardware loop target attributes */
    nested_loops_arm(N, &total_checksum);
    triangular_loops_arm(N, &total_checksum);
    
    /* Additional loop nest in main for more coverage */
    volatile int main_loop_limit = N + 10;
    int local_arr[100][100];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            local_arr[i][j] = rand() % 100;
        }
    }
    
    /* Another nesting pattern: do-while inside for */
    for (int p = 0; p < main_loop_limit; ++p) {
        int q = 0;
        volatile int inner_max = (p % 8) + 3;
        
        do {
            /* Complex conditional structure */
            if (p > q) {
                local_arr[p % 100][q % 100] += p - q;
                if (local_arr[p % 100][q % 100] > 200) {
                    local_arr[p % 100][q % 100] = 200;
                }
            } else if (p < q) {
                local_arr[p % 100][q % 100] -= q - p;
                /* Early break possibility */
                if (local_arr[p % 100][q % 100] < -50) {
                    break;
                }
            } else {
                local_arr[p % 100][q % 100] *= 2;
            }
            
            q++;
        } while (q < inner_max && q < 20);
    }
    
    /* Final checksum computation */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            total_checksum += local_arr[i][j];
        }
    }
    
    printf("Checksum: %d\n", total_checksum);
    return 0;
}
