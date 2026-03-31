/* Test file to exercise gengtype's balanced delimiter parsing */

/* Parentheses case: function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*callback)(const char *);

/* Brackets case: array declarations */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
static int variable_len[sizeof(long)];

/* Braces case: aggregate definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

union MixedUnion {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

enum Color { RED, GREEN, BLUE };

/* Static initializers with braces */
int global_init[3] = {1, 2, 3};
struct SimpleStruct global_struct = {42, 'A'};

/* Nested and combined delimiters */

/* Array of function pointers (combines [] and ()) */
int (*func_array[5])(const char*);

/* Function pointer returning pointer to array (combines (), *, and []) */
int (*(*get_matrix(void))[10]);

/* Struct with array member and initializer (combines {} and []) */
struct DataContainer {
    int values[2];
    char *names[3];
} data_instance = {
    .values = {10, 20},
    .names = {"a", "b", "c"}
};

/* Complex nested example */
typedef struct Node {
    struct Node *(*get_next)(void);
    int (*process)(int data[5]);
    union {
        int i;
        struct {
            int x;
            int y[2];
        } coord;
    } payload;
} NodeType;

/* GTY-marked declarations (gengtype specifically looks for these) */
typedef struct GTY(()) GtyStruct {
    int GTY((skip)) *pointer_field;
    struct GtyStruct *next;
} GtyStruct;

/* Another GTY example with function pointer */
static GTY(()) int (*gty_callback)(int) = NULL;

/* Multi-level pointer with array and function */
char *(*(*complex_decl)[5])(int, char[]);

/* Anonymous struct in union */
union Anonymous {
    struct {
        int a;
        int b;
    };
    float f;
};

/* Bitfield in struct (tests brace handling with unusual content) */
struct WithBitfield {
    unsigned int flag:1;
    unsigned int count:7;
    int array[4];
};

/* Empty braces cases */
struct EmptyStruct {};
int empty_array[] = {};
