#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

// Function with ARM target attribute to ensure hardware loop pass runs
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int early_break_cond = 0;
    int sum = 0;
    
    // First nested loop: for inside for with complex control flow
    for (int i = 0; i < N; ++i) {
        // Loop-invariant calculation
        int scale = i * 2;
        volatile int inner_limit = i + 5;
        
        // Multiple basic blocks created by if statement
        if (i % 3 == 0) {
            scale += 10;  // Creates separate basic block
        }
        
        // Inner loop with dependency on outer index
        for (int j = 0; j < inner_limit; ++j) {
            // Complex conditional with early exit
            if (early_break_cond && j > inner_limit / 2) {
                break;  // Creates exit block
            }
            
            // Conditional update with multiple paths
            if ((i + j) % 2 == 0) {
                arr[i][j] = scale * j;
            } else {
                arr[i][j] = scale + j;
            }
            
            // Another conditional for more blocks
            if (j % 7 == 0) {
                sum += arr[i][j];
            }
        }
        
        // Additional code between loops
        if (i % 4 == 0) {
            early_break_cond = !early_break_cond;
        }
    }
    
    // Second distinct loop nest: while inside for
    int k = 0;
    while (k < N / 2) {
        volatile int m_limit = N - k;
        
        // Different inner loop structure
        for (int m = 0; m < m_limit; ++m) {
            // Switch statement for more complex CFG
            switch (k % 3) {
                case 0:
                    arr[k][m] += sum;
                    break;
                case 1:
                    arr[k][m] -= sum / 2;
                    break;
                default:
                    arr[k][m] *= 2;
                    break;
            }
            
            // Conditional continue
            if (m % 5 == 0) {
                continue;
            }
            
            arr[m][k] = arr[k][m] + k;
        }
        
        // Loop with multiple increments
        k += (k % 2 == 0) ? 1 : 2;
    }
    
    printf("ARM function checksum: %d\n", sum);
}

// Another function with different nesting pattern
__attribute__((target("arch=armv8-a")))
void complex_nesting_arm(int N, int arr[SIZE][SIZE]) {
    volatile int outer_volatile = N;
    int counter = 0;
    
    // Triple nested loops
    for (int x = 0; x < outer_volatile / 3; ++x) {
        int y_limit = x + 2;
        
        for (int y = 0; y < y_limit; ++y) {
            // Innermost loop with volatile bound
            volatile int z_bound = (x + y) % 10 + 1;
            
            for (int z = 0; z < z_bound; ++z) {
                // Complex conditional structure
                if (x > y && y > z) {
                    arr[x][y] += z * 3;
                } else if (x < y) {
                    arr[x][y] -= z;
                } else {
                    arr[x][y] = x + y + z;
                }
                
                // Early exit based on volatile
                if (counter++ > 1000) {
                    goto exit_inner;  // Alternative control flow
                }
            }
            exit_inner:
            
            // Loop-invariant with side effect
            static int persistent = 0;
            persistent += x;
        }
        
        // Conditional break in outer loop
        if (x > N / 2) {
            break;
        }
    }
    
    printf("Complex nesting counter: %d\n", counter);
}

// RISC-V targeted function
__attribute__((target("arch=rv32imc")))
void riscv_nested_loops(int N, int arr[SIZE][SIZE]) {
    int total = 0;
    
    // Different nesting pattern for RISC-V
    int a = 0;
    while (a < N) {
        volatile int b_limit = (a * 3) % (N + 1);
        
        for (int b = 0; b < b_limit; ++b) {
            // Multiple if-else chains
            if (a % 2 == 0) {
                if (b % 3 == 0) {
                    arr[a][b] = a * b;
                } else {
                    arr[a][b] = a + b;
                }
            } else {
                arr[a][b] = arr[a][b] * 2;
            }
            
            total += arr[a][b];
            
            // Nested conditional for more blocks
            if (b > b_limit / 2) {
                if (total > 10000) {
                    break;
                }
            }
        }
        
        a += (a % 3 == 0) ? 2 : 1;
    }
    
    printf("RISC-V total: %d\n", total);
}

int main(int argc, char *argv[]) {
    // Non-constant loop bounds from command line
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE) N = SIZE;
    
    // Initialize array with pseudo-random data
    int arr[SIZE][SIZE];
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    // Call functions with different target architectures
    nested_loops_arm(N, arr);
    complex_nesting_arm(N, arr);
    riscv_nested_loops(N, arr);
    
    // Final checksum to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            final_sum += arr[i][j];
        }
    }
    
    printf("Final checksum: %d\n", final_sum);
    return 0;
}
