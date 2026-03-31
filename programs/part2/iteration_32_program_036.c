/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>
#include <stdint.h>

volatile int global_counter = 0;

/* Function to prevent loop elimination */
__attribute__((noinline))
static void use_value(int val) {
    global_counter += val;
}

/* 1. DISJOINT LOOPS - Two sequential loops with no block intersection */
__attribute__((noinline))
void disjoint_loops(volatile int limit_a, volatile int limit_b) {
    int sum = 0;
    
    /* First loop - completely separate blocks */
    for (int i = 0; i < limit_a; i++) {
        sum += i * 2;
        /* Some arithmetic to create basic blocks */
        if (i % 3 == 0) {
            sum -= 1;
        }
    }
    
    /* Second loop - separate from first, no intersection */
    for (int j = 0; j < limit_b; j++) {
        sum += j * 3;
        if (j % 4 == 0) {
            sum += 2;
        }
    }
    
    use_value(sum);
}

/* 2. PERFECTLY NESTED LOOPS - Inner loop blocks are subset of outer */
__attribute__((noinline))
void perfectly_nested(volatile int outer_limit, volatile int inner_limit) {
    int sum = 0;
    
    for (int i = 0; i < outer_limit; i++) {
        /* Outer loop preamble */
        sum += i;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            sum += j * i;
            if (j % 2 == 0) {
                sum += 1;
            }
        }
        
        /* Outer loop postamble */
        sum -= i / 2;
    }
    
    use_value(sum);
}

/* 3. PARTIALLY OVERLAPPING LOOPS - Share some blocks but not all */
__attribute__((noinline))
void partially_overlapping(volatile int limit) {
    int sum = 0;
    
    /* Loop A - has unique blocks and shared blocks */
    for (int i = 0; i < limit; i++) {
        /* Unique block to Loop A */
        sum += i * 5;
        
        /* Shared block - also appears in Loop B */
        if (i & 1) {  /* Shared condition check */
            sum += 3; /* Shared arithmetic */
        } else {
            /* Unique to Loop A */
            sum -= 2;
        }
        
        /* More unique blocks */
        if (i % 3 == 0) {
            sum += 7;
        }
    }
    
    /* Loop B - overlaps with Loop A in the shared if block */
    for (int j = 0; j < limit; j++) {
        /* Unique block to Loop B */
        sum += j * 4;
        
        /* Shared block - identical to one in Loop A */
        if (j & 1) {  /* Same condition */
            sum += 3; /* Same operation */
        } else {
            /* Unique to Loop B */
            sum -= 1;
        }
        
        /* Different unique blocks */
        if (j % 5 == 0) {
            sum += 11;
        }
    }
    
    use_value(sum);
}

/* 4. COMPLEX NESTING WITH BREAKS/CONTINUES - Creates more control flow */
__attribute__((noinline))
void complex_nesting(volatile int limit) {
    int sum = 0;
    
    /* Outer loop with early exit */
    for (int i = 0; i < limit; i++) {
        if (i > limit / 2) {
            /* Early break creates unique blocks */
            break;
        }
        
        /* Middle loop */
        for (int j = 0; j < i; j++) {
            if (j % 3 == 0) {
                continue;  /* Skip creates more blocks */
            }
            sum += j;
            
            /* Innermost loop */
            for (int k = 0; k < 3; k++) {
                sum += k;
                if (k == 1) {
                    sum += 10;  /* Unique block */
                }
            }
        }
    }
    
    /* Another loop at same level, partially overlapping with above */
    for (int x = 0; x < limit; x++) {
        /* This if block might share structure with ones above */
        if (x % 2 == 0) {
            sum += x * 2;
        } else {
            sum += x;
        }
    }
    
    use_value(sum);
}

/* 5. LOOP WITH SWITCH INSIDE - Creates multiple basic blocks */
__attribute__((noinline))
void loop_with_switch(volatile int limit) {
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        switch (i % 4) {
            case 0:
                sum += i;
                break;
            case 1:
                sum += i * 2;
                break;
            case 2:
                sum += i * 3;
                /* Fall through */
            case 3:
                sum += 5;
                break;
        }
    }
    
    /* Another loop that might share some switch structure */
    for (int j = 0; j < limit / 2; j++) {
        switch (j % 3) {
            case 0:
                sum += j;
                break;
            case 1:
                sum += j * 2;
                /* Different from first loop's case 1 */
                break;
            case 2:
                sum += 10;
                break;
        }
    }
    
    use_value(sum);
}

int main(void) {
    volatile int iter_count = 10;
    
    /* Call all loop patterns to ensure they're analyzed together */
    disjoint_loops(iter_count, iter_count + 5);
    perfectly_nested(iter_count, 3);
    partially_overlapping(iter_count);
    complex_nesting(iter_count);
    loop_with_switch(iter_count);
    
    /* Use the result to prevent dead code elimination */
    printf("Final counter: %d\n", global_counter);
    
    return global_counter > 0 ? 0 : 1;
}
