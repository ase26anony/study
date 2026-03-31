/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int takes_double(double d) { return (int)d; }
int (*returns_fn_ptr(int (*f)(double)))(char) {
    static int result = 0;
    return NULL; /* Simplified for example */
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float, double);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { "test", {{7, 8}, {9, 10}} }, 
    simple_func 
};

/* Union with nested arrays */
union DataUnion {
    int (*func_array[2])(void);
    struct {
        char data[3][4];
        int count;
    } nested;
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combined constructs in single declarations */
/* Compound literal in initialization */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

/* Function with complex parameter types */
void process_table(int (*table[])[5], struct Nested param) {
    /* Function body - does nothing for this test */
    (void)table;
    (void)param;
}

/* Struct containing function pointer returning pointer to array */
struct Container {
    int (*(*get_array)(void))[3];
    union DataUnion data;
};

/* 5. Global scope declarations */
int (*(*global_func_ptr)(void))[3];
struct Data { 
    int (*func)(int); 
    struct Container *container;
} global_data = { NULL, NULL };

/* Complex typedef chain */
typedef int base_t;
typedef base_t (*level1_t)(base_t);
typedef level1_t (*level2_t)(level1_t, base_t);
typedef struct {
    level2_t complex_func;
    base_t array[2][2];
} super_complex_t;

/* 6. Main function to use the types and prevent dead code elimination */
int main(void) {
    int result = 0;
    
    /* Use global_nested */
    result += global_nested.a[0][0];  /* 1 */
    result += global_nested.a[1][2];  /* 6 */
    result += global_nested.inner.values[0][1];  /* 8 */
    
    /* Use arr with designated initializers */
    result += arr[0][2];  /* 3 */
    result += arr[1][1];  /* 5 */
    
    /* Use ptr_to_array (compound literal) */
    result += ptr_to_array[0][1];  /* 2 */
    result += ptr_to_array[2][0];  /* 5 */
    
    /* Initialize and use function pointers */
    fn_simple local_fn_ptr = simple_func;
    if (local_fn_ptr) {
        result += local_fn_ptr();  /* 42 */
    }
    
    /* Create and use local struct with initializer */
    struct Nested local_nested = {
        {{11, 12, 13}, {14, 15, 16}},
        { "local", {{17, 18}, {19, 20}} },
        NULL
    };
    result += local_nested.a[0][2];  /* 13 */
    
    /* Complex declaration in local scope */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    (void)local_complex;  /* Prevent unused warning */
    
    /* Call function with complex parameters */
    int (*local_table[2])[5];
    process_table(local_table, local_nested);
    
    /* Use union with initializer */
    union DataUnion data_union = {
        .nested = { .data = {{'a','b','c','d'},{'e','f','g','h'},{'i','j','k','l'}}, .count = 12 }
    };
    result += data_union.nested.count;  /* 12 */
    
    /* Final result should be: 1+6+8+3+5+2+5+42+13+12 = 97 */
    printf("Result: %d\n", result);
    
    return (result == 97) ? 0 : 1;
}
