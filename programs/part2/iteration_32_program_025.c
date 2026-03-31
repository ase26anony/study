/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;

/* Function to prevent loop optimization */
__attribute__((noinline))
static void use(int value) {
    global_counter += value;
}

/* 1. DISJOINT LOOPS - Two sequential loops with no block intersection */
__attribute__((noinline))
void disjoint_loops(volatile int limit) {
    int sum1 = 0;
    int sum2 = 0;
    
    /* First loop - completely separate blocks */
    for (int i = 0; i < limit; i++) {
        sum1 += i * 2;
        if (i % 3 == 0) {
            sum1 -= 1;  /* Unique block to first loop */
        }
    }
    
    /* Second loop - completely separate blocks */
    for (int j = 0; j < limit; j++) {
        sum2 += j * 3;
        if (j % 4 == 0) {
            sum2 += 2;  /* Unique block to second loop */
        }
    }
    
    use(sum1 + sum2);
}

/* 2. PERFECTLY NESTED LOOPS - Inner loop blocks are subset of outer loop */
__attribute__((noinline))
void perfectly_nested(volatile int outer_limit, volatile int inner_limit) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < outer_limit; i++) {
        total += i;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            total += j;
            if (j % 2 == 0) {
                total += 1;  /* Block inside inner loop */
            }
        }
        
        if (i % 2 == 0) {
            total += 2;  /* Block in outer but not inner */
        }
    }
    
    use(total);
}

/* 3. PARTIALLY OVERLAPPING LOOPS - Share some blocks but not all */
__attribute__((noinline))
void partially_overlapping(volatile int limit) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < limit; i++) {
        /* Shared preamble - both loops will have similar entry code */
        int temp = i * i;
        
        if (i & 1) {
            /* SHARED BLOCK PATTERN - both loops execute similar code here */
            result += temp;
            if (temp > 10) {
                result -= 1;  /* Nested condition in shared block */
            }
        } else {
            /* UNIQUE TO LOOP A */
            result += temp * 2;
            if (temp < 5) {
                result += 3;  /* Unique nested block to Loop A */
            }
        }
    }
    
    /* Loop B - shares some control flow patterns with Loop A */
    for (int j = 0; j < limit; j++) {
        /* Shared preamble - similar to Loop A */
        int temp = j * j;
        
        if (j & 1) {
            /* SHARED BLOCK PATTERN - same structure as Loop A */
            result -= temp;  /* Different operation creates different block */
            if (temp > 10) {
                result += 2;  /* Same condition, different action */
            }
        } else {
            /* UNIQUE TO LOOP B */
            result -= temp * 3;
            if (temp < 5) {
                result -= 4;  /* Unique nested block to Loop B */
            }
        }
    }
    
    use(result);
}

/* 4. COMPLEX NESTING WITH MULTIPLE RELATIONSHIPS */
__attribute__((noinline))
void complex_nesting(volatile int limit) {
    int acc = 0;
    
    /* Loop X */
    for (int x = 0; x < limit; x++) {
        acc += x;
        
        /* Loop Y - nested inside X */
        for (int y = 0; y < 2; y++) {
            acc += y;
            
            /* Loop Z - deeply nested */
            for (int z = 0; z < 2; z++) {
                acc += z;
                if (z == 0) {
                    acc += 1;  /* Unique to innermost loop */
                }
            }
        }
        
        /* Another sequential loop after nested structure */
        for (int w = 0; w < 3; w++) {
            acc += w * 2;
        }
    }
    
    use(acc);
}

/* 5. LOOPS WITH EARLY EXITS - Creates more complex CFG */
__attribute__((noinline))
void loops_with_early_exit(volatile int limit) {
    int val = 0;
    
    /* Loop with conditional break */
    for (int i = 0; i < limit; i++) {
        val += i;
        if (val > 100) {
            break;  /* Creates additional basic block */
        }
        if (i % 5 == 0) {
            continue;  /* Another control flow edge */
        }
        val += 1;
    }
    
    /* Another loop with different break condition */
    for (int j = 0; j < limit; j++) {
        val -= j;
        if (val < -50) {
            break;  /* Similar structure but different condition */
        }
        if (j % 3 == 0) {
            continue;
        }
        val -= 2;
    }
    
    use(val);
}

int main(void) {
    volatile int iter_count = 10;
    
    /* Call all loop patterns to ensure they're compiled */
    disjoint_loops(iter_count);
    perfectly_nested(iter_count, 5);
    partially_overlapping(iter_count);
    complex_nesting(iter_count);
    loops_with_early_exit(iter_count);
    
    /* Print result to prevent dead code elimination */
    printf("Final counter: %d\n", global_counter);
    
    return global_counter != 0 ? 0 : 1;
}
