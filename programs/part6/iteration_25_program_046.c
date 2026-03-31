/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*func_ptr_t)(int);
typedef func_ptr_t (*meta_func_t)(func_ptr_t);
int (*(*global_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointers */
int simple_func(int x) { return x * 2; }
int (*func_for_double)(double) = NULL;

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(void);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*callback)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } value;
};

/* Global struct with complex initializer */
char global_char = 'A';
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { &global_char, NULL },
    { .l = 100 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4} };

struct Anonymous {
    int x;
};

void process(int (*table[])[5], struct Anonymous param);

/* Complex type mixing all delimiters */
typedef struct {
    int (*methods[3])(void);
    struct {
        char data[10];
    } config;
} Service;

Service global_service = {
    .methods = { NULL, NULL, NULL },
    .config = { .data = "test" }
};

/* 5. More global declarations with complex syntax */
int (*(*global_var)(void))[3] = NULL;

struct Data { 
    int (*func)(int); 
    int values[2][2];
} global_data = { 
    .func = simple_func, 
    .values = {{10, 20}, {30, 40}}
};

/* Function returning complex type */
struct Data* (*get_data_handler(void))(int) {
    static struct Data* (*handler)(int) = NULL;
    return handler;
}

/* 6. Even more complex nested types */
typedef union {
    struct {
        int (*compare)(const void*, const void*);
        void (*process[2])(int);
    } ops;
    void* (*allocators[4])(size_t);
} Utility;

Utility global_utility = {
    .ops = {
        .compare = NULL,
        .process = { NULL, NULL }
    }
};

/* Main function to use the complex types */
int main(void) {
    /* 1. Use complex function pointer types */
    meta_func_t mfunc = NULL;
    global_func_ptr = NULL;
    
    /* 2. Access nested array elements */
    int sum = 0;
    sum += global_nested.a[0][0];  /* 1 */
    sum += global_nested.a[1][2];  /* 6 */
    sum += arr[0][1];              /* 2 */
    sum += arr[1][2];              /* 6 */
    
    /* 3. Use global_data */
    if (global_data.func) {
        sum += global_data.func(5);  /* 10 */
    }
    
    /* 4. Access values from global_data */
    sum += global_data.values[0][0];  /* 10 */
    sum += global_data.values[1][1];  /* 40 */
    
    /* 5. Use ptr_to_array */
    sum += ptr_to_array[0][0];  /* 1 */
    sum += ptr_to_array[1][1];  /* 4 */
    
    /* 6. Access global_nested inner member */
    if (global_nested.inner.p) {
        sum += *global_nested.inner.p;  /* 'A' = 65 */
    }
    
    /* 7. Use global_utility (even if NULL) */
    if (global_utility.ops.compare) {
        /* Would call if not NULL */
    }
    
    /* 8. Complex local declaration mimicking global patterns */
    int (*(*local_complex)(int (*)(float)))[2] = NULL;
    struct {
        int (*callbacks[2])(void);
        char data[2][3];
    } local_struct = {
        .callbacks = { NULL, NULL },
        .data = { {'a','b','c'}, {'d','e','f'} }
    };
    
    /* Add some data from local struct */
    sum += local_struct.data[0][0];  /* 'a' = 97 */
    sum += local_struct.data[1][2];  /* 'f' = 102 */
    
    /* Final calculation: 1+6+2+6+10+10+40+1+4+65+97+102 = 344 */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}

/* Additional function definitions */
void process(int (*table[])[5], struct Anonymous param) {
    /* Implementation not needed for parser coverage */
    (void)table;
    (void)param;
}

/* Complex function matching global_func_ptr signature */
int (*func_for_char(char c))(char) {
    static int (*result)(char) = NULL;
    (void)c;
    return result;
}
