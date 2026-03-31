#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

// Function with ARM target attribute to ensure hardware loop pass runs
__attribute__((target("arch=armv8-a")))
void nested_loops_complex(int N, int arr[SIZE][SIZE]) {
    volatile int early_exit_trigger = 0;
    int sum = 0;
    
    // First nested loop: for inside for with complex control flow
    for (int i = 0; i < N; ++i) {
        // Loop-invariant computation that varies with outer loop
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 5;
        
        // Multiple basic blocks created by if statement
        if (i % 3 == 0) {
            scale += 10;  // Creates separate basic block
        }
        
        // Inner loop with dependency on outer index
        for (int j = 0; j < inner_limit; ++j) {
            // Complex conditional with early exit possibility
            if (early_exit_trigger && j > inner_limit / 2) {
                break;  // Creates exit block
            }
            
            // Nested if creating more basic blocks
            if (j % 2 == 0) {
                arr[i][j] = i * j * scale;
            } else {
                arr[i][j] = i + j + scale;
            }
            
            // Conditional continue
            if (arr[i][j] % 7 == 0) {
                continue;  // Creates backedge with condition
            }
            
            sum += arr[i][j];
        }
        
        // Another if creating separate block after inner loop
        if (i % 5 == 0) {
            early_exit_trigger = !early_exit_trigger;
        }
    }
    
    // Second independent loop nest with different structure
    int k = 0;
    volatile int while_limit = N / 2;
    
    // while loop as outer, for as inner
    while (k < while_limit) {
        int row_sum = 0;
        
        // Switch statement to create multiple basic blocks
        switch (k % 4) {
            case 0:
                row_sum = k * 10;
                break;
            case 1:
                row_sum = k * 20;
                break;
            default:
                row_sum = k * 5;
                break;
        }
        
        // Inner for loop with different iteration pattern
        for (int m = N - 1; m >= 0; --m) {
            // Conditional that could affect bitmap intersection
            if (m < k && arr[k][m] > 0) {
                arr[k][m] += row_sum;
            } else if (m == k) {
                arr[k][m] = row_sum;
            }
            
            // Volatile check for unpredictable control flow
            volatile int random_break = k * m % 13;
            if (random_break == 0) {
                continue;
            }
            
            sum += arr[k][m] % 100;
        }
        
        k += (k % 3 == 0) ? 2 : 1;  // Non-linear increment
    }
    
    printf("Checksum from complex loops: %d\n", sum);
}

// Another function with different loop structure
__attribute__((target("arch=armv8-a")))
void triple_nested_loops(int M, int arr[SIZE][SIZE]) {
    volatile int limit1 = M;
    volatile int limit2 = M / 2;
    int total = 0;
    
    // Triple nested loops for deeper hierarchy
    for (int a = 0; a < limit1; a += 2) {
        int mid_sum = a * 3;
        
        for (int b = a; b < limit2; ++b) {
            // Multiple if-else creating blocks
            int factor;
            if (b % 3 == 0) {
                factor = 2;
            } else if (b % 3 == 1) {
                factor = 3;
            } else {
                factor = 5;
            }
            
            for (int c = 0; c < b; ++c) {
                // Complex condition with partial overlap
                if (c % factor == 0 && a % 2 == 0) {
                    arr[a][c] = (a * b + c) * factor;
                } else if (c % 2 == 0) {
                    arr[a][c] = a + b - c;
                }
                
                // Early exit based on volatile
                volatile int exit_cond = arr[a][c] % 17;
                if (exit_cond == 0 && c > b / 2) {
                    break;
                }
                
                total += arr[a][c] % 50;
            }
            
            // Loop-invariant but outer-loop-varying computation
            mid_sum += b * factor;
        }
        
        // Update array based on computed mid_sum
        for (int d = 0; d < SIZE && d < mid_sum % 20; ++d) {
            arr[a][d] += mid_sum;
        }
    }
    
    printf("Checksum from triple loops: %d\n", total);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    // Non-constant loop bounds from command line
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE) N = SIZE;
    if (N < 10) N = 10;
    
    // Initialize array with pseudo-random data
    int arr[SIZE][SIZE];
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    // Call functions with hardware loop target attributes
    nested_loops_complex(N, arr);
    triple_nested_loops(N / 2, arr);
    
    // Final checksum to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            final_sum += arr[i][j] % 256;
        }
    }
    printf("Final checksum: %d\n", final_sum);
    
    return 0;
}
