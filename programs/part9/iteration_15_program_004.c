/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        result += i;
        
        /* Nested condition */
        if (i % 2 == 0) {
            result *= 2;
        } else {
            result -= 1;
        }
    }
    
    /* Conditional based on y */
    if (y > 10) {
        for (volatile int j = 0; j < 3; ++j) {
            result += y * j;
        }
    } else if (y < 0) {
        result = -result;
    } else {
        result += 100;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int a, volatile double b) {
    double total = 0.0;
    
    /* While loop */
    volatile int count = a;
    while (count > 0) {
        total += b * count;
        
        /* Switch statement */
        switch (count % 3) {
            case 0:
                total *= 1.1;
                break;
            case 1:
                total /= 1.05;
                break;
            case 2:
                total -= 0.5;
                break;
        }
        
        count--;
    }
    
    /* Multiple conditionals */
    if (total > 100.0) {
        for (volatile int k = 0; k < 5; ++k) {
            total -= k * 0.5;
        }
    } else if (total < 0.0) {
        total = -total;
    }
    
    return total;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int n, volatile int depth) {
    static int calls = 0;
    calls++;
    
    if (depth <= 0 || n <= 0) {
        return calls;
    }
    
    int sum = 0;
    for (volatile int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            sum += func3(n - 1, depth - 1);
        } else {
            sum += i;
        }
    }
    
    return sum;
}

/* Function 4: Simple utility */
void func4(volatile char flag) {
    volatile int x = 10;
    
    do {
        x--;
        
        if (flag == 'A') {
            printf("Flag A: %d\n", x);
        } else if (flag == 'B') {
            printf("Flag B: %d\n", x * 2);
        } else {
            printf("Other: %d\n", x * 3);
        }
    } while (x > 0);
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running with mode: %d\n", mode);
    
    /* Different execution paths based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result1: %d\n", func1(5, 15));
            printf("Result2: %.2f\n", func2(4, 2.5));
            func4('A');
            break;
            
        case 1:
            printf("Result1: %d\n", func1(3, -5));
            printf("Result3: %d\n", func3(3, 2));
            func4('B');
            break;
            
        case 2:
            printf("Result2: %.2f\n", func2(6, 1.8));
            printf("Result3: %d\n", func3(2, 3));
            func4('C');
            break;
            
        case 3:
            printf("Result1: %d\n", func1(8, 8));
            printf("Result2: %.2f\n", func2(2, 10.0));
            printf("Result3: %d\n", func3(4, 1));
            func4('D');
            break;
    }
    
    /* Always execute some common path */
    volatile int common = 0;
    for (volatile int i = 0; i < 10; ++i) {
        common += i;
        if (common > 20) {
            common /= 2;
        }
    }
    
    printf("Common result: %d\n", common);
    
    return 0;
}
