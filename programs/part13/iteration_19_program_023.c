/* Test program for GCC auto-profile coverage of phi-node-defined conditionals */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile_sink = x;
    return volatile_sink;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Loop-carried phi with copy chain */
int test_loop_phi(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for flag */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* Hot path */
        } else {
            flag = 0;  /* Cold path */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined flag */
    int a = flag;
    int b = copy_once(a);
    int c = copy_twice(b);
    
    /* Conditional comparing against 0/1 */
    if (c == 1) {  /* This should be annotated as hot */
        result += 1000;
    } else {
        result += 1;
    }
    
    return result;
}

/* Pattern 2: Multiple return paths creating phi */
int test_multi_return_phi(int x) {
    int flag;
    
    if (x > 1000) {
        flag = 1;
    } else if (x > 500) {
        flag = 0;
    } else {
        flag = 1;  /* Hot path */
    }
    
    /* Copy chain */
    int a = flag;
    int b = a;
    int c = copy_once(b);
    
    /* Compare against 1 */
    if (c == 1) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Pattern 3: Switch statement with phi */
int test_switch_phi(int x) {
    int flag;
    
    switch (x % 4) {
        case 0:
            flag = 1;  /* Hot */
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;  /* Hot */
            break;
        default:
            flag = 0;
            break;
    }
    
    /* Longer copy chain */
    int a = flag;
    int b = copy_once(a);
    int c = copy_once(b);
    int d = copy_twice(c);
    
    /* Compare against 0 */
    if (d == 0) {
        return x - 1;
    } else {
        return x + 1;  /* Hot path */
    }
}

/* Pattern 4: Boolean phi with explicit 0/1 comparison */
bool test_bool_phi(int x, int y) {
    bool condition;
    
    /* Different paths set the boolean */
    if (x > y) {
        condition = true;  /* Hot */
    } else {
        condition = false;
    }
    
    /* Copy through intermediate bool */
    bool a = condition;
    bool b = a;
    bool c = copy_once(b);
    
    /* This generates comparison against 0/1 in SSA */
    if (c) {  /* Equivalent to c == 1 */
        return true;
    } else {
        return false;
    }
}

/* Pattern 5: Nested loops with phi propagation */
int test_nested_loop_phi(int outer, int inner) {
    int hot_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < outer; ++i) {
        int inner_flag = 0;
        
        /* Inner loop creates phi */
        for (int j = 0; j < inner; ++j) {
            if (j % 2 == 0) {
                inner_flag = 1;  /* Hot */
            } else {
                inner_flag = 0;
            }
            sum += j;
        }
        
        /* Propagate phi through copies */
        int a = inner_flag;
        int b = copy_once(a);
        
        /* Compare against 1 */
        if (b == 1) {
            hot_flag = 1;
            sum += 100;
        }
    }
    
    /* Final comparison with copy chain */
    int c = hot_flag;
    int d = copy_twice(c);
    
    if (d == 1) {
        return sum * 2;
    } else {
        return sum;
    }
}

/* Pattern 6: Recursive function creating phi */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int flag;
    if (n % 2 == 0) {
        flag = 1;  /* Hot */
    } else {
        flag = 0;
    }
    
    /* Copy before recursion */
    int a = flag;
    int b = copy_once(a);
    
    int result = test_recursive_phi(n + b, depth - 1);
    
    /* Compare after recursion */
    int c = b;
    if (c == 1) {
        return result + 10;
    } else {
        return result + 1;
    }
}

/* Pattern 7: Phi from pointer comparison */
int test_pointer_phi(int* ptr1, int* ptr2) {
    int same_block;
    
    /* Different control flow paths */
    if (ptr1 && ptr2) {
        same_block = (ptr2 - ptr1 < 100) ? 1 : 0;
    } else {
        same_block = 0;
    }
    
    /* Copy chain */
    int a = same_block;
    int b = a;
    int c = copy_once(b);
    int d = copy_twice(c);
    
    /* Compare against 0 */
    if (d == 0) {
        return 0;
    } else {
        return 1;  /* Hot */
    }
}

/* Main driver to create hot paths */
int main() {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int i = 0; i < 100000; ++i) {
        /* Mix different patterns to avoid pattern recognition */
        total += test_loop_phi(100);
        total += test_multi_return_phi(i % 2000);
        total += test_switch_phi(i);
        total += test_bool_phi(i, 500);
        
        if (i % 10 == 0) {
            total += test_nested_loop_phi(5, 20);
        }
        
        if (i % 100 == 0) {
            total += test_recursive_phi(i, 3);
        }
        
        int arr[100];
        total += test_pointer_phi(&arr[0], &arr[i % 100]);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
