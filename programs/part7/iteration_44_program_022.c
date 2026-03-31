/* Test input for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. PARENTHESES () cases */
/* Function pointer typedef */
typedef int (*func_ptr_type)(int, char);

/* Complex function pointer declaration */
void (*(*complex_func_ptr)(void))(int);

/* Function pointer in GTY context */
static GTY(()) int (*gty_func_ptr)(double) = 0;

/* 2. BRACKETS [] cases */
/* Simple array */
int simple_array[10];

/* Multi-dimensional array with expression */
extern int matrix[5][(sizeof(int)*2)];

/* Array in struct */
struct ArrayHolder {
    int data[(10 + 5)];
    char *strings[20];
};

/* 3. BRACES {} cases */
/* Struct definition */
struct SimpleStruct {
    int a;
    char b;
    float c;
};

/* Union with nested struct */
union ComplexUnion {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

/* Static initializer */
int initialized_array[3] = {1, 2, 3};

/* 4. NESTED/COMBINED cases */
/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_function(void))[10]);

/* Struct with initialized array member - combines {} and [] */
struct DataContainer {
    int values[2];
    char *(*processor)(int);
} data_instance = { 
    .values = {100, 200},
    .processor = 0
};

/* Complex nested example */
struct NestedExample {
    /* Array of pointers to functions taking array and returning struct */
    struct Result (*(*operations[3])(int params[5]))[2];
    
    /* Union with array initializer */
    union {
        int nums[4];
        struct {
            char *name;
            void (*callback)(void);
        } handler;
    } data;
};

/* GTY-marked structure with all delimiter types */
typedef struct GTY(()) MarkedType {
    /* Parentheses in function pointer */
    void (*destructor)(struct MarkedType *);
    
    /* Brackets in array */
    int scores[((2 * 3) + 1)];
    
    /* Nested struct with braces */
    struct {
        int x;
        int y;
    } position;
    
    /* Array of function pointers */
    int (*validators[3])(const char *);
} MarkedType;

/* Even more complex nested case */
int (*(*(*nested_fp_array[2])[3])(float))[4];

/* Initialized struct with all delimiters */
struct CompleteExample {
    int (*compare)(int a, int b);
    char buffer[256];
    struct {
        int count;
        int *items;
    } metadata;
} global_example = {
    .compare = 0,
    .buffer = {0},
    .metadata = {
        .count = 0,
        .items = (int[5]){1, 2, 3, 4, 5}
    }
};

/* Final edge case: empty braces/brackets */
struct EmptyTypes {
    int empty_array[0];
    struct {} anonymous;
    union {} empty_union;
};
