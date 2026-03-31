#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent constant propagation
    int sum = 0;
    
    // Loop A: Will have blocks that partially overlap with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
            // Early continue creates additional basic block
            continue;
        }
        
        // Loop B: Nested inside A but with early exit
        for (int j = 0; j < 8; ++j) {
            sum += j;
            if (j == i % 5) {
                // Break creates separate exit block
                break;
            }
        }
        
        // Conditional block that may be shared with other loops
        if (i % 2 == 0) {
            sum += 1;
        } else {
            sum -= 1;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int seed) {
    int sum = 0;
    volatile int outer_limit = 12;
    
    // Loop C: Sequential to Loop A from previous function
    // Shares some control flow patterns but different structure
    int k = 0;
    while (k < outer_limit) {
        sum += k * 3;
        
        // Switch statement creates multiple basic blocks
        switch (k % 4) {
            case 0:
                sum += 10;
                break;
            case 1:
                sum += 20;
                // Fall through creates shared block
            case 2:
                sum += 30;
                break;
            default:
                sum += 40;
        }
        
        // Loop D: Strict subset of Loop C's blocks
        for (int m = 0; m < 4; ++m) {
            sum += m * k;
            // Function call creates separate block
            if (m % 2 == 0) {
                sum += complex_loops_1(m); // Recursive call pattern
            }
        }
        
        k++;
        
        // Conditional continue
        if (k % 3 == 0) continue;
        
        // Additional arithmetic
        sum *= 1.01; // Force floating point operation
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_control_flow(int base) {
    int result = base;
    volatile int mod = 7;
    
    // Loop E: Shares header block with Loop F
    for (int x = 0; x < 24; x += 2) {
        result += x;
        
        // Common conditional block
        if (x % mod == 0) {
            result *= 2;
            // Loop F: Starts in same block as E's conditional
            for (int y = 0; y < 3; ++y) {
                result += y * x;
                if (y == 1) {
                    // Early exit creates partial overlap
                    goto partial_exit;
                }
            }
            partial_exit:
            result += 100;
        }
        
        // Loop G: Partially overlaps with E and F
        do {
            result -= x;
            if (result < 0) {
                result = 0;
                break; // Creates another exit block
            }
        } while (++x % 4 != 0);
    }
    
    // Loop H: Sequential but shares post-loop blocks
    int z = 0;
    while (z < 10) {
        result += complex_loops_2(z);
        z++;
        
        // Shared cleanup code block
        if (z == 10) {
            result %= 1000; // Final modification
        }
    }
    
    return result;
}

__attribute__((noinline))
void nested_loop_hierarchy(int iterations) {
    volatile int depth = 3;
    int matrix[4][4] = {0};
    
    // Multi-level nested loops with varying bounds
    for (int l1 = 0; l1 < depth; ++l1) {
        // Outer loop I
        for (int l2 = 0; l2 < 8; ++l2) {
            // Middle loop J (subset of I's blocks)
            matrix[l1][l2 % 4] += l1 * l2;
            
            for (int l3 = 0; l3 < 5; ++l3) {
                // Inner loop K (strict subset of J's blocks)
                if (l3 % 2 == 0) {
                    matrix[l1][l2 % 4] -= l3;
                    continue;
                }
                
                // Additional control flow within innermost loop
                switch (l3) {
                    case 1:
                        matrix[l1][l2 % 4] *= 2;
                        break;
                    case 3:
                        matrix[l1][l2 % 4] /= 2;
                        // Intentional fallthrough
                    default:
                        matrix[l1][l2 % 4] += 1;
                }
            }
            
            // Conditional break in middle loop
            if (matrix[l1][l2 % 4] > 100) {
                break;
            }
        }
        
        // Loop L: At same level as I but different structure
        int counter = 0;
        while (counter++ < 4) {
            matrix[l1][counter % 4] = complex_loops_1(matrix[l1][counter % 4]);
            
            // Nested do-while with early exit
            do {
                if (matrix[l1][counter % 4] < 0) {
                    goto reset_value;
                }
                matrix[l1][counter % 4] >>= 1;
            } while (matrix[l1][counter % 4] > 10);
            
            reset_value:
            matrix[l1][counter % 4] = 0;
        }
    }
    
    // Use matrix to prevent dead code elimination
    volatile int* dummy = (int*)matrix;
    (void)dummy;
}

int main() {
    int total = 0;
    volatile int seed = 42; // Volatile to prevent constant folding
    
    // Execute all loop patterns with data dependencies
    total += complex_loops_1(seed);
    total += complex_loops_2(total);
    total += overlapping_control_flow(total);
    
    // Force nested loop analysis
    nested_loop_hierarchy(total % 10);
    
    // Additional sequential loops to create more opportunities for overlap
    for (int repeat = 0; repeat < 3; ++repeat) {
        total = (total * 1103515245 + 12345) & 0x7fffffff;
        
        // Loop M: Another candidate for hardware loops
        for (int i = 0; i < 32; ++i) {
            total += i;
            if (i == 16) {
                // Creates partial block overlap with outer loop
                total -= complex_loops_2(i);
            }
        }
        
        // Loop N: Shares some blocks with M's post-loop code
        int temp = total;
        while (temp > 0) {
            total += temp % 10;
            temp /= 10;
            
            // Call to function with loops creates interprocedural effects
            if (temp % 7 == 0) {
                total += overlapping_control_flow(temp);
            }
        }
    }
    
    // Final observable output
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
