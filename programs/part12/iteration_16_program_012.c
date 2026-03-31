/* Test program for GCC AutoFDO profile reading coverage.
   Specifically targets phi nodes with boolean constants, copy propagation chains,
   and conditional branches comparing to 0/1. */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use(int);
extern void use_ptr(int*);

/* Global volatile to force runtime evaluation */
volatile int g_seed = 0;

/* Test 1: Basic phi with 0/1 constants, single copy, compare to 0 */
__attribute__((noinline, noipa))
int test1(volatile int cond) {
    int x;
    if (cond > 0) {
        x = 1;  /* Becomes phi argument 1 */
    } else {
        x = 0;  /* Becomes phi argument 0 */
    }
    /* x is defined by phi(0, 1) at this point */
    int y = x;  /* Single copy assignment */
    if (y == 0) {  /* Conditional comparing copy to 0 */
        use(100);
        return 1;
    } else {
        use(200);
        return 2;
    }
}

/* Test 2: Longer copy chain, compare to 1 */
__attribute__((noinline, noipa))
int test2(volatile int cond) {
    int a;
    if (cond & 1) {
        a = 0;
    } else {
        a = 1;
    }
    /* a = phi(0, 1) */
    int b = a;      /* First copy */
    int c = b;      /* Second copy */
    int d = c;      /* Third copy */
    if (d == 1) {   /* Compare final copy to 1 */
        use(300);
        return 3;
    } else {
        use(400);
        return 4;
    }
}

/* Test 3: Phi with three incoming edges */
__attribute__((noinline, noipa))
int test3(volatile int mode) {
    int val;
    if (mode == 1) {
        val = 1;
    } else if (mode == 2) {
        val = 0;
    } else {
        val = 1;  /* Third phi argument */
    }
    /* val = phi(1, 0, 1) */
    int tmp1 = val;
    int tmp2 = tmp1;
    if (tmp2 != 0) {  /* Compare using != operator */
        use(500);
        return 5;
    }
    use(600);
    return 6;
}

/* Test 4: Nested control flow with loop */
__attribute__((noinline, noipa))
int test4(volatile int iter) {
    int sum = 0;
    for (int i = 0; i < (iter & 3); i++) {
        int flag;
        if (i & 1) {
            flag = 1;
        } else {
            flag = 0;
        }
        /* flag = phi(1, 0) inside loop */
        int f1 = flag;
        int f2 = f1;
        if (f2 == 0) {
            sum += i * 10;
        } else {
            sum += i * 20;
        }
    }
    use(sum);
    return sum;
}

/* Test 5: Multiple phi nodes feeding each other */
__attribute__((noinline, noipa))
int test5(volatile int sel1, volatile int sel2) {
    int x, y;
    
    if (sel1 > 0) {
        x = 1;
    } else {
        x = 0;
    }
    /* x = phi(1, 0) */
    
    if (sel2 > 0) {
        y = x;  /* Use phi result */
    } else {
        y = 1;
    }
    /* y = phi(x, 1) */
    
    int z = y;
    if (z == 0) {
        use(700);
        return 7;
    }
    use(800);
    return 8;
}

/* Test 6: Switch statement generating phi */
__attribute__((noinline, noipa))
int test6(volatile int code) {
    int status;
    switch (code & 3) {
        case 0: status = 0; break;
        case 1: status = 1; break;
        case 2: status = 0; break;
        default: status = 1; break;
    }
    /* status = phi(0, 1, 0, 1) */
    int s1 = status;
    int s2 = s1;
    int s3 = s2;
    if (s3 == 1) {
        use(900);
        return 9;
    }
    use(1000);
    return 10;
}

/* Test 7: Copy chain with arithmetic that doesn't break SSA propagation */
__attribute__((noinline, noipa))
int test7(volatile int base) {
    int cond;
    if (base % 2 == 0) {
        cond = 1;
    } else {
        cond = 0;
    }
    /* cond = phi(1, 0) */
    
    /* Chain of copies with trivial arithmetic that gets optimized to copies */
    int t1 = cond + 0;
    int t2 = t1 * 1;
    int t3 = t2 | 0;
    int t4 = t3 & ~0;
    
    if (t4 == 0) {
        use(1100);
        return 11;
    }
    use(1200);
    return 12;
}

/* Test 8: Do-while loop with phi at loop header */
__attribute__((noinline, noipa))
int test8(volatile int limit) {
    int i = 0;
    int result = 0;
    int first_iter = 1;
    
    do {
        int flag;
        if (first_iter) {
            flag = 1;
            first_iter = 0;
        } else {
            flag = (i & 1) ? 1 : 0;
        }
        /* flag = phi(1, phi(1, 0)) - complex phi structure */
        
        int f = flag;
        if (f == 1) {
            result += i * 2;
        } else {
            result += i * 3;
        }
        i++;
    } while (i < (limit & 7));
    
    use(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    g_seed = 42;
    
    /* Call test functions with volatile conditions */
    checksum += test1(g_seed);
    checksum += test2(g_seed + 1);
    checksum += test3(g_seed + 2);
    checksum += test4(g_seed + 3);
    checksum += test5(g_seed + 4, g_seed + 5);
    checksum += test6(g_seed + 6);
    checksum += test7(g_seed + 7);
    checksum += test8(g_seed + 8);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs with different seeds to exercise different paths */
    for (int i = 0; i < 10; i++) {
        g_seed = i * 13;
        test1(g_seed);
        test2(g_seed);
        test3(g_seed);
    }
    
    return 0;
}

/* Dummy implementation of external functions to allow linking */
void use(int x) {
    /* Prevent optimization */
    static volatile int sink;
    sink = x;
}

void use_ptr(int *p) {
    static volatile int sink;
    if (p) sink = *p;
}
