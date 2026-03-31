/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_returning_fn_ptr)(double))(char);

/* Function pointer returning pointer to array */
typedef int (*(*fn_returning_array_ptr)(void))[3];

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][(3+2)])[4][5];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][3];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

/* Global struct with complex initializer */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { 
        "test", 
        {{10,11,12},{13,14,15}}
    }, 
    NULL 
};

/* Union with nested arrays */
union DataUnion {
    int (*matrix[2])[3];
    struct {
        char (*strings[2])[10];
        int count;
    } str_data;
};

/* 4. Combine constructs in single declarations */
/* Function prototype with complex parameters */
void process(int (*table[])[5], struct Nested param);

/* Variable with compound literal initializer */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* 5. More global declarations */
/* Global function pointer variable */
int (*(*global_func_ptr)(void))[3];

/* Struct containing function pointer array */
struct Container {
    int (*(*func_array[3])(int))[2];
    union DataUnion data;
};

/* 6. Helper functions for execution */
int simple_func(void) {
    return 42;
}

int (*func_returning_array(void))[3] {
    static int arr[3] = {7, 8, 9};
    return &arr;
}

int process_table(int (*table[])[5], int rows) {
    int sum = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 5; j++) {
            sum += (*table)[j];
        }
        table++;
    }
    return sum;
}

/* 7. Main function with meaningful operations */
int main(void) {
    /* Use complex typedef */
    fn_returning_fn_ptr complex_fp = NULL;
    
    /* Access initialized global struct */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
            sum += global_nested.inner.values[i][j];
        }
    }
    
    /* Use array with parenthesized size */
    char (*local_strings[(1+2)])[20];
    static char str1[20] = "Hello";
    static char str2[20] = "World";
    static char str3[20] = "Test";
    local_strings[0] = &str1;
    local_strings[1] = &str2;
    local_strings[2] = &str3;
    
    /* Use compound literal */
    int (*local_ptr)[2] = (int[][2]){ {10,20}, {30,40} };
    sum += (*local_ptr)[0] + (*local_ptr)[1];
    sum += (*(local_ptr + 1))[0] + (*(local_ptr + 1))[1];
    
    /* Initialize and use function pointer */
    global_func_ptr = (int (*(*)(void))[3])func_returning_array;
    int (*result_array)[3] = global_func_ptr();
    sum += (*result_array)[0] + (*result_array)[1] + (*result_array)[2];
    
    /* Use nested designated initializer */
    int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };
    sum += arr[0][0] + arr[1][2];
    
    /* Create and use struct with function pointer */
    struct Container container;
    /* Initialize one element to prevent undefined behavior */
    container.func_array[0] = NULL;
    
    /* Print result to prevent optimization */
    printf("Sum: %d\n", sum + simple_func());
    
    return 0;
}

/* Additional complex declarations at file scope */
/* Function returning pointer to function returning pointer to array */
int (*(*(*extra_complex)(int))(void))[2];

/* Array of structs with nested initializers */
struct Nested nested_array[2] = {
    { 
        {{1,1,1},{2,2,2}}, 
        { "first", {{1,2,3},{4,5,6}} }, 
        NULL 
    },
    { 
        {{3,3,3},{4,4,4}}, 
        { "second", {{7,8,9},{10,11,12}} }, 
        (fn_simple)simple_func 
    }
};

/* Anonymous struct in union initializer */
union {
    struct {
        int (*func)(int (*)[3]);
        char data[10];
    } s;
    long value;
} anonymous_union = { 
    .s = { NULL, {'a','b','c'} } 
};
