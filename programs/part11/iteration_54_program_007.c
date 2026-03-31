/* test_hw_doloop.c - Test program to cover hw-doloop.cc bitmap intersection logic */
#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline))
void pattern_a_perfect_nesting(int n, int m) {
    int i, j;
    for (i = 0; i < n; i++) {
        sink = i;  /* Create separate block in outer loop */
        for (j = 0; j < m; j++) {
            sink = i + j;  /* Inner loop body */
            /* Add conditional to create more blocks */
            if (__builtin_expect(j & 1, 0)) {
                sink += 1;
            }
        }
        /* Another block in outer loop after inner */
        sink = i * 2;
    }
}

/* Pattern B: Partially overlapping loops - share some blocks but not nested */
__attribute__((noinline))
void pattern_b_partial_overlap(int n, int m) {
    int i = 0, j = 0;
    
    /* Shared setup block */
    int shared = n + m;
    
    /* First loop with early exit */
    while (i < n) {
        sink = i;
        if (i == shared) {
            /* Jump to second loop's blocks */
            goto overlap_section;
        }
        i++;
    }
    
    /* Code between loops */
    sink = -1;
    
overlap_section:
    /* Second loop that shares the overlap_section block */
    while (j < m) {
        sink = j;
        /* This block is only in second loop */
        if (j & 1) {
            sink += 100;
        }
        j++;
        /* Shared block with first loop when i == shared */
        if (j == m/2) {
            sink = shared;
        }
    }
}

/* Pattern C: Sequential loops sharing a preheader */
__attribute__((noinline)) 
void pattern_c_sequential_shared(int n, int m) {
    /* Shared preheader block */
    int setup = n * m;
    sink = setup;
    
    /* First loop */
    int i;
    for (i = 0; i < n; i++) {
        sink = i + setup;
        /* Complex body to create multiple blocks */
        if (__builtin_expect(i & 3, 1)) {
            sink += 2;
        } else {
            sink -= 1;
        }
    }
    
    /* Shared middle block */
    sink = setup * 2;
    
    /* Second loop */
    int j;
    for (j = 0; j < m; j++) {
        sink = j + setup;
        /* Different body structure */
        switch (j & 3) {
            case 0: sink += 10; break;
            case 1: sink += 20; break;
            default: sink += 30; break;
        }
    }
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void pattern_d_complex_nesting(int n, int m, int k) {
    int i, j, l;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        sink = i;
        
        /* Middle loop - perfectly nested in outer */
        for (j = 0; j < m; j++) {
            sink = i + j;
            
            /* Innermost loop - perfectly nested */
            for (l = 0; l < k; l++) {
                sink = i + j + l;
                if (__builtin_expect(l & 1, 0)) {
                    sink *= 2;
                }
            }
            
            /* Block only in middle loop */
            if (j == m/2) {
                sink += 1000;
            }
        }
        
        /* Another inner loop at same level as middle */
        for (l = 0; l < k; l++) {
            sink = i - l;
            /* Early exit creates different block pattern */
            if (l > k/2) break;
        }
    }
}

/* Pattern E: Do-while and while loops mixed */
__attribute__((noinline))
void pattern_e_mixed_loops(int n) {
    int i = 0;
    
    /* do-while loop */
    do {
        sink = i;
        i++;
        if (i > n/2) {
            /* Nested while loop inside do-while */
            int j = 0;
            while (j < 3) {
                sink = i + j;
                j++;
            }
        }
    } while (i < n);
    
    /* Separate while loop sharing some blocks through goto */
    int k = 0;
    shared_label:
    while (k < n) {
        sink = k * 2;
        k++;
        if (k == n/3) {
            goto shared_label;
        }
    }
}

/* Pattern F: Loops with irregular control flow using switch */
__attribute__((noinline))
void pattern_f_irregular(int n) {
    int i = 0;
    
    while (i < n) {
        switch (i % 4) {
            case 0:
                /* Loop within case */
                for (int j = 0; j < 2; j++) {
                    sink = i + j;
                }
                break;
            case 1:
                sink = i * 3;
                /* Another loop in different case */
                do {
                    sink += 1;
                } while (sink < i + 10);
                break;
            default:
                sink = i;
                break;
        }
        i++;
    }
}

/* Main function to call all patterns */
int main() {
    /* Call each pattern with different parameters to create
       different loop structures and block patterns */
    pattern_a_perfect_nesting(100, 50);
    pattern_b_partial_overlap(100, 50);
    pattern_c_sequential_shared(100, 50);
    pattern_d_complex_nesting(10, 20, 30);
    pattern_e_mixed_loops(100);
    pattern_f_irregular(100);
    
    return sink != 0;
}
