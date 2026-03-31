/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int (*func_taking_double(double d))(double) { 
    static int result = 0;
    return &simple_func; /* Cast would be needed, but simplified for example */
}
int (*func_returning_int_ptr(char c))(char) {
    static int result = 0;
    return &simple_func;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_array[2][3])(void);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

struct DeeplyNested {
    struct {
        struct Nested nested;
        int x;
    } inner_struct;
    union {
        int u_int;
        char u_char;
    } inner_union;
};

/* Global initializers with nested braces */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { "test", {7, 8} },
    &simple_func
};

int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

struct Anonymous {
    int x;
};

void process(int (*table[])[5], struct Anonymous param);

/* 5. Global scope complex declarations */
int (*(*global_func_ptr)(void))[3];

struct Data { 
    int (*func)(int);
    int matrix[2][2];
} global_data = { 
    NULL, 
    {{1,2},{3,4}}
};

/* Complex typedef combining all delimiters */
typedef struct {
    int (*(*member_func)(int))[2];
    struct {
        char data[10];
    } nested;
} ComplexType;

ComplexType global_complex = {
    NULL,
    {"complex"}
};

/* Even more complex declaration mixing all delimiters */
int (*(*(*most_complex[2])(struct {int a;}))[3])(void) = {NULL, NULL};

/* Function using designated initializers with nested braces */
struct Designated {
    int first;
    int second[3];
    struct {
        float f;
    } inner;
} designated_global = {
    .first = 1,
    .second = {2, 3, 4},
    .inner = { .f = 5.5 }
};

/* 6. Main function to use these types and prevent dead code elimination */
int main(void) {
    /* Use the global complex function pointer type */
    fn_complex complex_ptr = NULL;
    
    /* Use array with parenthesized size expression */
    char (*local_strings[(1+1)])[20];
    static char data1[20] = "Hello";
    static char data2[20] = "World";
    local_strings[0] = &data1;
    local_strings[1] = &data2;
    
    /* Access nested array from global struct */
    int sum = 0;
    sum += global_nested.a[0][0];
    sum += global_nested.a[1][2];
    sum += global_nested.inner.values[0];
    
    /* Use designated initializer struct */
    sum += designated_global.first;
    sum += designated_global.second[1];
    
    /* Use the complex array pointer */
    sum += ptr_to_array[0][0];
    sum += ptr_to_array[1][1];
    
    /* Use global array */
    sum += arr[0][2];
    sum += arr[1][0];
    
    /* Use global_data matrix */
    sum += global_data.matrix[0][0];
    sum += global_data.matrix[1][1];
    
    /* Assign to function pointer */
    global_func_ptr = NULL; /* Would need actual function, but NULL for compilation */
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", sum);
    
    /* Use local_strings to prevent unused variable warning */
    printf("String: %s\n", *local_strings[sum % 2]);
    
    return sum > 0 ? 0 : 1;
}

/* Additional function to be pointed to */
int* func_returning_int_array(int size) {
    static int arr[3] = {1, 2, 3};
    return arr;
}

/* Function matching the complex process prototype */
void process(int (*table[])[5], struct Anonymous param) {
    /* Empty implementation - just for declaration */
    (void)table;
    (void)param;
}
