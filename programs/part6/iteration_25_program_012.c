/* test_complex_types.c - Designed to exercise gengtype-parse.cc's 
   consume_balanced() function for all delimiter types */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Global function pointer using complex type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_array[2][3])(void);

/* Array with parenthesized size expression */
double (*matrix[ (sizeof(int) > 4) ? 8 : 4 ])[16];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } u;
};

/* Global struct with nested initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { (char*)0x1000, NULL },
    { .x = 42 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combined constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

struct {
    int (*table[3])[5];
    struct Nested nested;
} combined = {
    .table = { (int(*)[5])&arr[0], (int(*)[5])&arr[1], NULL },
    .nested = global_nested
};

/* Function prototype with complex parameters */
void process(int (*table[])[5], struct { int x; int (*func)(void); } param);

/* 5. Additional global declarations */
int (*(*global_var)(void))[3];

struct Data { 
    int (*func)(int);
    struct Nested nested;
} global_data = { 
    NULL, 
    { {{7, 8, 9}, {10, 11, 12}}, { NULL, NULL }, { .y = 3.14 } }
};

/* Simple functions to assign to function pointers */
int simple_func(int x, int y) { return x + y; }
int func_taking_double(double d) { return (int)d; }
int func_returning_int(char c) { return (int)c; }
int* func_returning_ptr_to_array(int x) { static int arr[3] = {1, 2, 3}; return arr; }

/* Main function to use the complex types */
int main(void) {
    /* 1. Use complex function pointer type */
    int (*(*local_fp)(int (*)(double)))(char);
    
    /* Assign compatible function (simplified for example) */
    /* Note: In real code, we'd need proper function matching the type */
    
    /* 2. Access nested array elements */
    int sum = 0;
    sum += global_nested.a[0][0];  /* 1 */
    sum += global_nested.a[1][2];  /* 6 */
    sum += arr[0][1];              /* 2 */
    sum += arr[1][0];              /* 4 */
    
    /* 3. Use the combined struct */
    if (combined.nested.inner.p == (char*)0x1000) {
        sum += 10;
    }
    
    /* 4. Access global_data */
    sum += global_data.nested.a[0][2];  /* 9 */
    
    /* 5. Use ptr_to_array */
    sum += ptr_to_array[0][0];  /* 1 */
    sum += ptr_to_array[2][1];  /* 6 */
    
    /* 6. Complex local declaration with initializer */
    struct {
        int (*(*func_array[2])(void))[3];
        struct Nested nested;
    } local_complex = {
        .func_array = { NULL, NULL },
        .nested = { 
            {{13, 14, 15}, {16, 17, 18}}, 
            { NULL, &simple_func }, 
            { .x = 99 }
        }
    };
    
    sum += local_complex.nested.a[0][0];  /* 13 */
    sum += local_complex.nested.u.x;      /* 99 */
    
    /* 7. Even more complex declaration mixing all delimiters */
    int (*(*mixed[2])(int))[3] = {
        [0] = &func_returning_ptr_to_array,
        [1] = NULL
    };
    
    /* 8. Nested initializer with all brace types */
    struct {
        int (*func)(int, int);
        int arr[2][2];
        struct {
            char *p;
            int x;
        } inner;
    } deep_nested = {
        .func = &simple_func,
        .arr = { {20, 21}, {22, 23} },
        .inner = { 
            .p = "test",
            .x = 100 
        }
    };
    
    sum += deep_nested.arr[0][0];  /* 20 */
    sum += deep_nested.inner.x;    /* 100 */
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);  /* Expected: 1+6+2+4+10+9+1+6+13+99+20+100 = 271 */
    
    return 0;
}

/* Additional function definitions */
void process(int (*table[])[5], struct { int x; int (*func)(void); } param) {
    /* Function body not important for parser coverage */
    (void)table;
    (void)param;
}
