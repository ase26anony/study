/* hw-doloop-test.c - Test program for hardware loop bitmap intersection analysis */
#include <stdio.h>

volatile int global_counter = 0;

/* Function to prevent optimization */
__attribute__((noinline)) 
int use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* 1. DISJOINT LOOPS - Two sequential loops with no block intersection */
__attribute__((noinline))
int disjoint_loops(int limit) {
    int local_sum = 0;
    volatile int barrier = limit;
    
    /* First loop - completely separate blocks */
    for (int i = 0; i < barrier; i++) {
        local_sum += i * 2;
        /* Simple arithmetic to keep loop alive */
        if (local_sum & 1) {
            local_sum ^= 0x55;
        }
    }
    
    /* Second loop - separate from first, no intersection */
    for (int j = 0; j < barrier; j++) {
        local_sum -= j * 3;
        /* Different operations to ensure separate blocks */
        if (local_sum & 2) {
            local_sum |= 0xAA;
        }
    }
    
    return local_sum;
}

/* 2. PERFECTLY NESTED LOOPS - Inner loop blocks are subset of outer */
__attribute__((noinline))
int perfectly_nested(int outer_limit, int inner_limit) {
    int total = 0;
    volatile int outer_barrier = outer_limit;
    volatile int inner_barrier = inner_limit;
    
    for (int i = 0; i < outer_barrier; i++) {
        /* Outer loop preamble */
        total += i * 10;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_barrier; j++) {
            total += j * 5;
            /* Simple conditional to create basic blocks */
            if (j & 1) {
                total ^= 0xFF;
            } else {
                total &= 0x7F;
            }
        }
        
        /* Outer loop postamble */
        total -= i * 2;
    }
    
    return total;
}

/* 3. PARTIALLY OVERLAPPING LOOPS - Share some blocks but not all */
__attribute__((noinline))
int partially_overlapping(int limit) {
    int result = 0;
    volatile int barrier = limit;
    
    /* Loop A - has unique blocks in else branch */
    for (int i = 0; i < barrier; i++) {
        /* Shared preamble with Loop B */
        result += i;
        
        if (i & 1) {
            /* SHARED BLOCK - Both loops execute this */
            result ^= 0x1234;
            global_counter++;
        } else {
            /* UNIQUE TO LOOP A */
            result += 1000;
            /* Complex enough to not be merged */
            for (int k = 0; k < 2; k++) {
                result -= k * 50;
            }
        }
        
        /* Shared postamble */
        result &= 0xFFFF;
    }
    
    /* Loop B - overlaps with Loop A in if branch */
    for (int j = 0; j < barrier; j++) {
        /* Shared preamble (same as Loop A) */
        result += j;
        
        if (j & 1) {
            /* SHARED BLOCK - Same as Loop A's if branch */
            result ^= 0x1234;
            global_counter--;
        } else {
            /* UNIQUE TO LOOP B - Different from Loop A */
            result -= 500;
            /* Different structure than Loop A's else */
            if (j % 3 == 0) {
                result |= 0x8000;
            } else {
                result &= 0x7FFF;
            }
        }
        
        /* Shared postamble */
        result &= 0xFFFF;
    }
    
    return result;
}

/* 4. COMPLEX NESTING WITH BREAK/CONTINUE - Creates more block variations */
__attribute__((noinline))
int complex_nesting(int limit) {
    int acc = 0;
    volatile int bar = limit;
    
    /* Outer loop with early exit */
    for (int i = 0; i < bar * 2; i++) {
        if (i >= bar) {
            /* Unique block for early exit */
            acc += 999;
            break;
        }
        
        /* Middle loop with continue */
        for (int j = 0; j < bar; j++) {
            if (j == bar / 2) {
                /* Unique continue block */
                acc += 111;
                continue;
            }
            
            /* Innermost loop */
            for (int k = 0; k < 3; k++) {
                acc += k * (i + j);
                /* Conditional creating multiple blocks */
                if (acc > 10000) {
                    acc >>= 1;
                }
            }
        }
    }
    
    return acc;
}

/* Main driver that combines all patterns */
int main(void) {
    int total = 0;
    volatile int seed = 10;
    
    /* Trigger disjoint loops analysis */
    total += disjoint_loops(seed);
    
    /* Trigger perfectly nested analysis */
    total += perfectly_nested(seed, seed / 2);
    
    /* Trigger partially overlapping analysis */
    total += partially_overlapping(seed);
    
    /* Trigger complex nesting analysis */
    total += complex_nesting(seed);
    
    /* Mix with global counter */
    total += global_counter;
    
    /* Use the result to prevent elimination */
    printf("Result: %d\n", use_value(total));
    
    return total != 0 ? 0 : 1;
}
