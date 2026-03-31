#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed, int *shared_acc) {
    volatile int limit = 32; // Volatile to prevent optimization
    int acc1 = 0, acc2 = 0;
    
    // Loop A: Sequential loop with complex control flow
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc1 += i * seed;
            continue;
        }
        if (i % 5 == 0) {
            acc1 -= i;
            break; // Early exit creates different block structure
        }
        acc1 += i;
    }
    
    // Loop B: Sequential loop that shares some blocks with Loop A's structure
    // but has different iteration pattern
    int j = 0;
    while (j < limit) {
        switch (j % 4) {
            case 0: acc2 += j * 2; break;
            case 1: acc2 -= j; break;
            case 2: acc2 += seed; break;
            default: acc2 *= 2; break;
        }
        if (j > limit / 2) {
            acc2 += 100;
            continue;
        }
        j++;
    }
    
    // Loop C: Nested inside a conditional, partially overlapping with A and B
    if (seed > 0) {
        for (int k = 0; k < 16; ++k) {
            acc1 += k * k;
            // Inner loop D: Strict subset of Loop C's blocks
            for (int m = 0; m < 4; ++m) {
                acc2 += m * seed;
                if (m % 2 == 0) continue;
                acc2 -= 1;
            }
            if (k % 7 == 0) break;
        }
    }
    
    *shared_acc += acc1 + acc2;
    return acc1;
}

__attribute__((noinline))
int complex_loops_2(int base, int *shared_acc) {
    volatile int mod = 24;
    int local_acc = 0;
    
    // Loop E: do-while with early continue
    int counter = 0;
    do {
        if (counter % 6 == 0) {
            local_acc += base;
            continue;
        }
        local_acc -= counter;
        counter++;
    } while (counter < mod);
    
    // Loop F: Another for loop with switch, overlapping block structure with E
    for (int x = 0; x < mod; x++) {
        switch (x % 3) {
            case 0: local_acc += x * base; break;
            case 1: 
                local_acc >>= 1;
                if (x > mod / 2) continue;
                break;
            case 2: local_acc *= 3; break;
        }
        
        // Loop G: Inner loop with subset relationship to F
        for (int y = 0; y < 8; y++) {
            local_acc += y;
            if (y == 4) break;
        }
    }
    
    // Loop H: Sequential after F, shares exit block
    int z = mod;
    while (z > 0) {
        local_acc += z;
        z -= 2;
        if (z < mod / 3) break;
    }
    
    *shared_acc += local_acc;
    return local_acc;
}

__attribute__((noinline))
int overlapping_loop_patterns(int init) {
    int total = 0;
    volatile int outer_limit = 8;
    
    // Multiple loops at same nesting level with shared pre-header
    for (int outer = 0; outer < outer_limit; outer++) {
        // Loop I: First inner loop
        int sum1 = 0;
        for (int inner1 = 0; inner1 < 12; inner1++) {
            sum1 += inner1 * outer;
            if (inner1 % 3 == 0) continue;
            sum1 += init;
        }
        total += sum1;
        
        // Loop J: Second inner loop at same level, different structure
        int sum2 = 0;
        int inner2 = 0;
        while (inner2 < 10) {
            sum2 -= inner2;
            inner2++;
            if (inner2 > 5) {
                sum2 += outer * 2;
                continue;
            }
        }
        total += sum2;
        
        // Loop K: Third loop with break that creates shared exit block
        for (int inner3 = 0; inner3 < 15; inner3++) {
            total += inner3;
            if (inner3 == outer + 3) break;
            total -= 1;
        }
    }
    
    return total;
}

int main() {
    volatile int seed = 42; // Volatile to prevent constant propagation
    int shared_accumulator = 0;
    int result1, result2, result3;
    
    // Execute all loop patterns
    result1 = complex_loops_1(seed, &shared_accumulator);
    result2 = complex_loops_2(seed + 1, &shared_accumulator);
    result3 = overlapping_loop_patterns(seed);
    
    // Create data dependencies between results
    int final_checksum = result1 + result2 * 3 + result3 + shared_accumulator;
    
    // Prevent dead code elimination
    printf("Loop analysis test checksum: %d\n", final_checksum);
    
    // Also use results in conditional to prevent optimization
    if (final_checksum > 1000000) {
        printf("Unexpected large checksum\n");
    }
    
    return final_checksum & 0xFF; // Return non-constant value
}
