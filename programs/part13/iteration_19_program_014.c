/* Test program for GCC auto-profile.cc lines 1312-1333
 * Creates phi-defined condition variables with copy chains
 * and hot/cold path annotations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Another copy chain function */
static int another_copy(int x) {
    int tmp1 = x;
    int tmp2 = tmp1;
    volatile_sink = tmp2;
    return tmp2;
}

/* Test 1: Phi node created after loop with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* Hot path */
        } else {
            flag = 0;  /* Cold path */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    int flag_copy2 = another_copy(flag_copy);
    
    /* Critical conditional comparing against 0/1 */
    if (flag_copy2 == 1) {  /* Compare against 1 */
        result += 1000;  /* Hot path */
    } else {
        result += 1;     /* Cold path */
    }
    
    return result;
}

/* Test 2: Phi from multiple return paths with copy chain */
int test_phi_from_multiple_returns(int x) {
    int ret_val;
    
    if (x > 1000) {
        ret_val = 1;  /* Hot path */
    } else if (x > 500) {
        ret_val = 0;  /* Medium path */
    } else {
        ret_val = 0;  /* Cold path */
    }
    
    /* Create copy chain */
    int tmp = ret_val;
    int tmp2 = tmp;
    volatile_sink = tmp2;
    int final_val = tmp2;
    
    /* Conditional with 0/1 comparison */
    if (final_val == 0) {  /* Compare against 0 */
        return x * 2;
    } else {
        return x * 10;  /* Hot path */
    }
}

/* Test 3: Switch statement creating phi node */
int test_phi_from_switch(int mode) {
    int state;
    
    switch (mode % 4) {
        case 0:
            state = 1;  /* Very hot */
            break;
        case 1:
            state = 0;  /* Hot */
            break;
        case 2:
            state = 1;  /* Medium */
            break;
        default:
            state = 0;  /* Cold */
            break;
    }
    
    /* Multiple copy assignments */
    int s1 = state;
    int s2 = s1;
    volatile_sink = s2;
    int s3 = s2;
    int s4 = another_copy(s3);
    
    /* Conditional branch with 0/1 comparison */
    if (s4 != 0) {  /* Compare against 0 (indirectly) */
        return 100 + mode;
    } else {
        return 50 + mode;
    }
}

/* Test 4: Recursive function creating phi for condition */
int test_recursive_phi(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int next_val;
    if (depth % 2 == 0) {
        next_val = test_recursive_phi(depth + 1, max_depth);
    } else {
        next_val = test_recursive_phi(depth + 2, max_depth);
    }
    
    /* Phi node created from recursive calls */
    int phi_val = (next_val > 0) ? 1 : 0;
    
    /* Copy chain */
    int chain1 = phi_val;
    int chain2 = chain1;
    volatile_sink = chain2;
    
    /* Conditional with 0/1 comparison */
    if (chain2 == 1) {  /* Compare against 1 */
        return depth * 10;
    } else {
        return depth * 5;
    }
}

/* Test 5: Complex loop with phi and copy chain */
int test_complex_loop_phi(int n) {
    int sum = 0;
    int hot_flag = 0;
    
    /* Outer loop - creates hot path */
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        
        /* Inner loop with conditional */
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 7 == 0) {
                inner_flag = 1;  /* Hot in inner loop */
            }
            sum += j;
        }
        
        /* Phi from inner loop to outer */
        if (inner_flag == 1) {
            hot_flag = 1;  /* Hot path */
        } else if (i % 100 == 0) {
            hot_flag = 0;  /* Rare cold path */
        }
        /* else: hot_flag remains from previous iteration (phi!) */
    }
    
    /* Create extensive copy chain */
    int f1 = hot_flag;
    int f2 = f1;
    int f3 = copy_chain(f2);
    int f4 = another_copy(f3);
    volatile_sink = f4;
    int f5 = f4;
    
    /* Critical conditional with 0/1 comparison */
    if (f5 == 1) {  /* Compare against 1 */
        return sum * 2;  /* Hot path result */
    } else {
        return sum / 2;  /* Cold path result */
    }
}

/* Test 6: Global variable with phi in hot loop */
static int global_counter = 0;
static int global_flag = 0;

int test_global_phi(int iterations) {
    int local_sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Update global based on condition */
        if (i % 100 == 0) {
            global_flag = 1;  /* Hot update */
        } else if (i % 1000 == 0) {
            global_flag = 0;  /* Very cold update */
        }
        
        global_counter++;
        local_sum += i;
    }
    
    /* Copy chain from global */
    int flag_copy = global_flag;
    int flag_copy2 = flag_copy;
    volatile_sink = flag_copy2;
    int final_flag = another_copy(flag_copy2);
    
    /* Conditional with 0/1 comparison */
    if (final_flag != 0) {  /* Compare against 0 */
        return local_sum + global_counter;
    } else {
        return local_sum;
    }
}

/* Main driver with hot loop */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Seed for variability */
    srand(time(NULL));
    
    printf("Starting auto-profile test patterns...\n");
    
    /* Hot loop calling all test functions */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Mix different patterns to create various phi nodes */
        total += test_phi_after_loop(i % 100 + 1);
        
        if (i % 10 == 0) {  /* Less frequent but still hot */
            total += test_phi_from_multiple_returns(i);
            total += test_phi_from_switch(i);
        }
        
        if (i % 50 == 0) {  /* Medium frequency */
            total += test_recursive_phi(0, 5);
        }
        
        if (i % 100 == 0) {  /* Less frequent */
            total += test_complex_loop_phi(i % 20 + 1);
            total += test_global_phi(i % 30 + 1);
        }
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 7) == 0, 1)) {
            total += 1;  /* Hot path */
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total > 0 ? 0 : 1;
}
