/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointers */
int simple_func(void) { return 42; }
int (*func_returning_int(double d))(void) { return simple_func; }
int (*(*get_complex_func(void))(int (*)(double)))(char) {
    return complex_func_ptr;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int multi_dim[2][3][4][5];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func_ptr)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } u;
};

struct Data {
    int (*func)(int);
    struct Nested nested;
};

/* Global initializers with nested braces */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { NULL, NULL },
    { .l = 100 }
};

int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

struct Data global_data = { 
    NULL, 
    { 
        {{7,8,9},{10,11,12}}, 
        { (char*)&global_data, NULL },
        { .d = 3.14 }
    } 
};

/* 4. Combined constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct Config {
    int (*process)(int (*table[])[5], struct { int x; } param);
    int values[2][2];
} config = {
    NULL,
    {{10, 20}, {30, 40}}
};

/* 5. More global complex types */
int (*(*global_var)(void))[3];

typedef struct {
    int (*(*nested_func)(int[][2]))(void);
    struct {
        char (*strings[3])[10];
    } inner;
} UltraComplex;

UltraComplex ultra = {
    NULL,
    { { NULL, NULL, NULL } }
};

/* Function to be called via complex function pointer */
int target_func(char c) { return (int)c; }

/* Helper function for array_of_func_ptrs */
int (*return_array_ptr(int x))[10] {
    static int arr[10];
    return &arr;
}

int main(void) {
    int result = 0;
    
    /* 1. Use complex function pointer */
    complex_func_ptr = get_complex_func();
    if (complex_func_ptr) {
        /* This would normally be assigned a real function */
        result += 1;
    }
    
    /* 2. Access nested array elements */
    result += arr[0][0];      /* 1 */
    result += arr[1][2];      /* 6 */
    result += global_nested.a[0][1];  /* 2 */
    result += global_nested.a[1][2];  /* 6 */
    
    /* 3. Use array of function pointers */
    array_of_func_ptrs[0] = return_array_ptr;
    
    /* 4. Access initialized struct */
    result += (int)global_data.nested.u.d;  /* 3 */
    
    /* 5. Use ptr_to_array */
    result += ptr_to_array[0][0];  /* 1 */
    result += ptr_to_array[2][1];  /* 6 */
    
    /* 6. Access config values */
    result += config.values[0][1];  /* 20 */
    result += config.values[1][0];  /* 30 */
    
    /* 7. Initialize and use strings array */
    static char str1[20] = "Hello";
    static char str2[20] = "World";
    strings[0] = &str1;
    strings[1] = &str2;
    
    /* 8. Use ultra complex type */
    ultra.inner.strings[0] = &str1;
    
    /* 9. Test fn1 and fn2 typedefs */
    fn1 f1 = simple_func;
    result += f1();  /* 42 */
    
    /* Final calculation */
    printf("Result: %d\n", result);  /* Should print: Result: 121 */
    
    /* Additional complex local declaration to exercise parser */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    struct {
        int (*func_array[2])(void);
        struct Nested nested;
    } local_struct = {
        { simple_func, NULL },
        { {{13,14,15},{16,17,18}}, {NULL, NULL}, {.l=200} }
    };
    
    result += local_struct.nested.a[0][0];  /* 13 */
    printf("Final result: %d\n", result);  /* Should print: Final result: 134 */
    
    return 0;
}
