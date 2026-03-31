/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer but outside inner */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Perfectly nested inner loop */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* More blocks in outer after inner */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    if (n > 0 && m > 0) {
        sink = n + m;
    }
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional second loop inside first - creates partial overlap */
        if (i % 2 == 0 && j < m) {
            /* Second loop that shares some blocks with first */
            for (j = 0; j < m; j++) {
                sum -= j;
                /* Early exit creates different block structure */
                if (j > n) break;
                sink = j;
            }
        }
        
        /* More code in first loop */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sink = i;
        }
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops sharing a common preheader */
__attribute__((noinline)) 
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Common preheader block - may be included in both loop bitmaps */
    int setup = n + m;
    sink = setup;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i;
        i++;
        /* Split block */
        if (__builtin_expect(i & 1, 1)) {
            sink = i;
        }
    }
    
    /* Code between loops */
    if (setup > 0) {
        sink = sum;
    }
    
    /* Second loop - may share preheader with first */
    for (j = 0; j < m; j++) {
        sum -= j;
        /* Different block structure */
        switch (j % 3) {
            case 0: sink = j; break;
            case 1: sum += j; break;
            default: break;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i + j;
            j++;
            
            /* Early exit condition */
            if (j > k) {
                sink = j;
                break;  /* Creates additional basic block */
            }
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += l;
                /* Conditional inside innermost */
                if (__builtin_expect(l % 5 == 0, 0)) {
                    sink = l;
                }
            }
        }
        
        /* Another inner loop at same level */
        for (j = 0; j < i; j++) {
            sum -= j;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular control flow */
__attribute__((noinline))
void irregular_loops(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlap */
    for (i = 0; i < n; i++) {
        sum += i;
        
        if (i % 2 == 0) {
            /* Jump into another loop's body */
            if (j < m) {
                goto inside_second;
            }
        }
        
        continue;
        
    inside_second:
        /* Second loop that we can enter mid-way */
        while (j < m) {
            sum -= j;
            j++;
            
            if (j % 3 == 0) {
                /* Jump back to first loop */
                goto back_to_first;
            }
        }
        
    back_to_first:
        if (i % 4 == 0) {
            sink = i;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 25;
    
    /* Execute each pattern multiple times with different parameters */
    perfect_nesting(test_n, test_m);
    partial_overlap(test_n, test_m);
    sequential_loops(test_n, test_m);
    complex_nesting(test_n, test_m, test_k);
    irregular_loops(test_n, test_m);
    
    /* Force compiler to keep all functions */
    __attribute__((used)) static void (*funcs[])(void) = {
        (void (*)(void))perfect_nesting,
        (void (*)(void))partial_overlap,
        (void (*)(void))sequential_loops,
        (void (*)(void))complex_nesting,
        (void (*)(void))irregular_loops
    };
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(funcs) > 0, "Functions exist");
    
    return 0;
}
