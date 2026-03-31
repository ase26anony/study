#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 200
#define MAX_DEPTH 5

// Volatile arrays to prevent optimization
static volatile int array1[ARRAY_SIZE];
static volatile int array2[ARRAY_SIZE];
static volatile int array3[ARRAY_SIZE];
static volatile int global_modifier = 0;

// Recursive loop generator with complex control flow
__attribute__((noinline, noipa, optimize("O3")))
static void generate_nested_loops(volatile int* arr1, volatile int* arr2, 
                                  volatile int* arr3, int depth, int start_idx) {
    volatile int control_var = rand() % 10;
    volatile int loop_bound = (rand() % 20) + 10;
    
    if (depth <= 0) return;
    
    // Outer loop with data-dependent bound
    for (int i = start_idx; i < loop_bound + global_modifier; i++) {
        // First inner loop with partial unrolling
        #pragma GCC unroll 4
        for (int j = 0; j < depth * 3; j++) {
            // Shared computation block that will be part of multiple loops' bitmaps
            int temp = arr1[i] + arr2[j];
            
            // Complex switch that creates shared basic blocks
            switch (control_var) {
                case 0:
                case 1:
                case 2:
                    // Shared handler block - will be in multiple loops' bitmaps
                    arr3[(i + j) % ARRAY_SIZE] = temp * 2;
                    if (temp > 100) {
                        // Early exit to label outside immediate parent
                        goto early_exit;
                    }
                    break;
                case 3:
                case 4:
                    arr1[i] = temp - arr3[j];
                    break;
                default:
                    arr2[j] = temp / (control_var + 1);
                    break;
            }
            
            // Manual loop unrolling creates additional basic blocks
            arr1[i] += 1;
            arr2[j] -= 1;
            if (j % 2 == 0) {
                arr3[(i + j) % ARRAY_SIZE] *= 2;
            } else {
                arr3[(i + j) % ARRAY_SIZE] /= 2;
            }
        }
        
        // Second inner loop that shares some blocks with first inner loop
        for (int k = depth; k < depth * 4; k++) {
            // Same switch structure creates overlapping block bitmaps
            switch (control_var) {
                case 0:
                case 1:
                case 2:
                    // Same shared handler - creates bitmap intersection but not subset
                    arr3[(i + k) % ARRAY_SIZE] = arr1[i] * arr2[k % ARRAY_SIZE];
                    if (arr1[i] < 0) {
                        goto early_exit;
                    }
                    break;
                case 5:
                case 6:
                    arr2[k % ARRAY_SIZE] = arr1[i] - arr3[i];
                    break;
                default:
                    // Different computation but same basic block structure
                    arr1[i] = arr2[k % ARRAY_SIZE] + arr3[i];
                    break;
            }
            
            // Loop distribution pattern: computation then conditional access
            int computed = arr1[i] * k + arr2[k % ARRAY_SIZE];
            
            // Conditional memory access that may be split into separate loop
            if (computed % 7 == 0) {
                arr3[i] = computed;
            }
            
            // Another computation after conditional
            arr2[k % ARRAY_SIZE] = computed - arr1[i];
            
            // Partial unrolling
            arr1[i] += k % 3;
            arr2[k % ARRAY_SIZE] -= k % 5;
            if (k % 4 == 0) {
                arr3[i] *= 3;
            }
        }
        
        // Recursive call for deeper nesting
        if (depth > 1) {
            generate_nested_loops(arr1, arr2, arr3, depth - 1, i + 1);
        }
        
        // Early exit label outside some loops
        early_exit:
        if (i % 11 == 0) {
            // Break to outside of some loop structure
            break;
        }
    }
    
    // Additional loop with goto creating complex control flow
    int counter = 0;
    restart_loop:
    for (int m = 0; m < depth * 2; m++) {
        for (int n = m; n < depth * 3; n++) {
            // Pointer arithmetic to create aliasing concerns
            volatile int* ptr1 = &arr1[m];
            volatile int* ptr2 = &arr2[n];
            volatile int* ptr3 = &arr3[(m + n) % ARRAY_SIZE];
            
            *ptr1 = *ptr2 + *ptr3;
            *ptr2 = *ptr1 - *ptr3;
            *ptr3 = *ptr1 * *ptr2;
            
            // Complex condition with goto
            if ((*ptr1 + *ptr2 + *ptr3) % 13 == 0) {
                counter++;
                if (counter < 3) {
                    goto restart_loop;  // Creates loop with exit to outer scope
                }
            }
        }
    }
}

// Helper function to create loops with different structures
__attribute__((noinline, noipa, optimize("O3")))
static void create_loop_hierarchy(volatile int* a1, volatile int* a2, 
                                  volatile int* a3, int pattern) {
    volatile int mod = pattern;
    
    // Pattern 1: Two loops with shared middle block
    for (int x = 0; x < 15 + mod; x++) {
        // Loop A
        for (int y = 0; y < 10; y++) {
            a1[x] += a2[y];
            // Shared if block
            if (a1[x] > a3[y]) {
                a3[y] = a1[x] - a2[y];
            }
        }
        
        // Loop B (shares the if block above)
        for (int z = 5; z < 15; z++) {
            a2[z] *= a3[x];
            // Same shared if block structure
            if (a1[x] > a3[z]) {
                a3[z] = a1[x] - a2[z];
            }
        }
    }
    
    // Pattern 2: Interleaved loops with switch
    volatile int sw = rand() % 8;
    for (int i = 0; i < 20; i++) {
        switch (sw) {
            case 0:
            case 1:
                for (int j = i; j < i + 5; j++) {
                    a1[j % ARRAY_SIZE] = a2[i] + a3[j % ARRAY_SIZE];
                }
                break;
            case 2:
            case 3:
                for (int j = 0; j < 10; j++) {
                    a2[(i + j) % ARRAY_SIZE] = a1[i] - a3[j];
                }
                // Fall through to shared block
            case 4:
                // Shared block for multiple switch cases
                a3[i] = a1[i] * a2[i];
                break;
            default:
                for (int j = 0; j < 8; j++) {
                    a3[(i + j) % ARRAY_SIZE] = a1[i] / (a2[j] + 1);
                }
                break;
        }
    }
}

int main() {
    // Seed for reproducible randomness
    srand(42);
    
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        array3[i] = (i * 3) % ARRAY_SIZE;
    }
    
    // Multiple calls with different depths to populate loop tree
    for (int iteration = 0; iteration < 4; iteration++) {
        global_modifier = iteration;
        
        // Call recursive generator with different depths
        generate_nested_loops(array1, array2, array3, 2, iteration * 10);
        generate_nested_loops(array1, array2, array3, 3, iteration * 5);
        generate_nested_loops(array1, array2, array3, 4, iteration * 3);
        
        // Create different loop hierarchy patterns
        create_loop_hierarchy(array1, array2, array3, iteration);
        
        // Modify global to prevent loop merging
        global_modifier += rand() % 10;
    }
    
    // Compute checksum to ensure all code is live
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i];
        checksum += array2[i];
        checksum += array3[i];
    }
    
    printf("Final checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
