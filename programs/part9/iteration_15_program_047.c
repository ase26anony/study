/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < 3; ++j) {
            result += j;
        }
    }
    
    /* Conditional block */
    if (y > 10) {
        result *= 2;
    } else if (y > 5) {
        result += 5;
    } else {
        result -= 1;
    }
    
    return result;
}

/* Function 2: Different complexity pattern */
double func2(volatile int n) {
    double sum = 0.0;
    
    if (n <= 0) {
        return 0.0;
    }
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        sum += 1.0 / count;
        
        /* Switch statement */
        switch (count % 3) {
            case 0:
                sum += 0.1;
                break;
            case 1:
                sum += 0.2;
                break;
            case 2:
                sum += 0.3;
                break;
        }
        
        count--;
    }
    
    return sum;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int max) {
    static int calls = 0;
    calls++;
    
    if (depth >= max || calls > 100) {
        return depth;
    }
    
    int result = depth;
    
    /* Multiple conditionals */
    if (depth % 4 == 0) {
        result += func3(depth + 1, max);
    } else if (depth % 4 == 1) {
        result += func3(depth + 2, max);
    } else if (depth % 4 == 2) {
        result += func3(depth + 3, max);
    } else {
        result += func3(depth + 4, max);
    }
    
    return result;
}

/* Function 4: Array operations */
void func4(volatile int size) {
    int arr[100];
    
    /* Initialize array */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        arr[i] = i * i;
    }
    
    /* Process array */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        if (arr[i] % 2 == 0) {
            arr[i] /= 2;
        } else {
            arr[i] = arr[i] * 3 + 1;
        }
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode: %d\n", mode);
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result1: %d\n", func1(5, 12));
            printf("Result2: %f\n", func2(8));
            break;
        case 1:
            printf("Result3: %d\n", func3(1, 5));
            func4(20);
            break;
        case 2:
            printf("Result1: %d\n", func1(3, 7));
            printf("Result2: %f\n", func2(4));
            printf("Result3: %d\n", func3(2, 4));
            break;
        case 3:
            func4(15);
            printf("Result1: %d\n", func1(8, 3));
            printf("Result2: %f\n", func2(12));
            printf("Result3: %d\n", func3(3, 6));
            break;
    }
    
    /* Additional execution for more coverage */
    if (mode > 10) {
        for (volatile int i = 0; i < mode % 10; ++i) {
            func1(i, i * 2);
        }
    }
    
    return 0;
}
