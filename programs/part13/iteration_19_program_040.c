/* Test program for GCC auto-profile coverage of phi-node-defined conditionals */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Function 1: Loop-carried dependency forming phi node */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' at loop header */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* 'flag' is defined by phi node at start of this block */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Conditional comparing phi-defined variable against 0 */
    if (flag_copy == 0) {  /* Must compare against 0 or 1 */
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Function 2: Multiple return paths creating phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int computed;
    
    if (x > y) {
        computed = 1;  /* First incoming value */
    } else if (x < y) {
        computed = 0;  /* Second incoming value */
    } else {
        computed = 1;  /* Third incoming value */
    }
    
    /* 'computed' is defined by phi node here (merging from 3 paths) */
    int a = computed;
    int b = a;
    volatile_sink = b;
    
    /* Compare against 1 */
    if (b == 1) {
        return x * 2;
    } else {
        return y * 2;
    }
}

/* Function 3: Switch statement with flag variable */
int test_phi_from_switch(char op) {
    int flag;
    
    switch (op) {
        case 'A':
        case 'a':
            flag = 1;
            break;
        case 'B':
        case 'b':
            flag = 0;
            break;
        case 'C':
        case 'c':
            flag = 1;
            break;
        default:
            flag = 0;
            break;
    }
    
    /* 'flag' defined by phi from switch cases */
    int temp = flag;
    volatile_sink = temp;
    
    /* Multiple copy assignments */
    int chain1 = temp;
    int chain2 = chain1;
    int chain3 = chain2;
    
    /* Compare against 0 */
    if (chain3 == 0) {
        return 1000;
    }
    return 2000;
}

/* Function 4: Recursive function creating phi for condition */
int test_phi_from_recursion(int n, int depth) {
    if (depth <= 0) {
        return n % 2;  /* Returns 0 or 1 */
    }
    
    int left = test_phi_from_recursion(n + 1, depth - 1);
    int right = test_phi_from_recursion(n - 1, depth - 1);
    
    /* 'left' and 'right' create phi nodes */
    int result = (left > right) ? left : right;
    
    /* Copy chain */
    int a = result;
    int b = a;
    volatile_sink = b;
    
    /* Compare against 1 */
    if (b == 1) {
        return n * 10;
    }
    return n * 5;
}

/* Function 5: Boolean variable with explicit 0/1 comparison */
bool test_bool_phi(int x, int y) {
    bool condition;
    
    /* Create phi for boolean */
    if (x > 100) {
        condition = true;  /* Becomes 1 */
    } else if (y > 100) {
        condition = false; /* Becomes 0 */
    } else {
        condition = (x + y) > 150;
    }
    
    /* Boolean is already 0/1, create copy chain */
    bool a = condition;
    volatile_sink = a;
    bool b = a;
    bool c = b;
    
    /* Explicit comparison against 0 */
    if (c == 0) {
        return false;
    }
    return true;
}

/* Function 6: Complex loop with nested conditionals */
int test_complex_phi_pattern(int n) {
    int state = 0;
    int sum = 0;
    
    /* Hot loop - will be annotated as hot */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            state = 1;
            sum += i * 2;
        } else {
            state = 0;
            sum += i;
        }
        
        /* Inner conditional using phi-defined variable */
        int local_copy = state;
        volatile_sink = local_copy;
        
        /* This creates many basic blocks with phi->copy->conditional pattern */
        if (local_copy == 1) {
            sum += 10;
        }
    }
    
    /* Final test after loop */
    int final_copy = state;
    if (final_copy == 0) {
        return sum * 2;
    }
    return sum;
}

/* Main function to drive execution and create hot paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to create hot paths for profile estimation */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Mix different patterns to cover various phi formations */
        total += test_phi_after_loop(iteration % 100);
        
        total += test_phi_from_multiple_returns(iteration, iteration / 2);
        
        char ops[] = {'A', 'B', 'C', 'D'};
        total += test_phi_from_switch(ops[iteration % 4]);
        
        if (iteration % 1000 == 0) {  /* Less frequent - expensive */
            total += test_phi_from_recursion(iteration % 10, 3);
        }
        
        total += test_bool_phi(iteration, iteration * 2) ? 1 : 0;
        
        if (iteration % 10 == 0) {  /* Less frequent - heavier computation */
            total += test_complex_phi_pattern(50);
        }
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect(iteration < 95000, 1)) {
            /* Hot path - will be annotated */
            volatile_sink = iteration;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
