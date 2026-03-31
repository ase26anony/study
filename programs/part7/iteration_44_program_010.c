/* Test input for gengtype parser coverage - targeting balanced delimiter parsing */

/* 1. Parentheses () in function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef char *(*string_processor)(const char *, int);

/* 2. Brackets [] in array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[BUFFER_SIZE];

/* 3. Braces {} in aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union DataUnion {
    int i;
    float f;
    double d;
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* 4. Nested and combined delimiters */
/* Array of function pointers (combines [] and ()) */
int (*callback_array[5])(const char *);

/* Function pointer returning pointer to array (combines (), *, and []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line (combines {} and []) */
struct DataContainer {
    int values[2];
    char *names[3];
};

/* 5. GTY-marked declarations (gengtype special context) */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int data;
} TreeNode;

static GTY(()) int (*global_handler)(int) = NULL;

/* 6. Complex nested examples */
/* Function pointer with array parameter and struct return */
struct Result {
    int code;
    char message[256];
};

typedef struct Result (*complex_handler)(
    int param1,
    char *params[],
    void (*callback)(struct Result)
);

/* Multi-dimensional array with function pointer elements */
void (*(*signal_handlers[3][2])(int))(void);

/* 7. More edge cases */
/* Anonymous struct in union */
union Anonymous {
    struct {
        int x;
        int y;
    } point;
    int coordinates[2];
};

/* Typedef with all three delimiters */
typedef struct {
    int (*methods[4])(void);
    union {
        int num;
        char str[20];
    } data;
} Object;

/* Array of structs with function pointers */
struct Handler {
    int id;
    void (*process)(int);
} handlers[] = {
    {1, NULL},
    {2, NULL}
};

/* 8. Pointer to array of function pointers returning structs */
struct Response {
    int status;
    char data[100];
};

struct Response (*(*api_table[5])(void))[10];

/* 9. Nested parentheses in macro-like contexts (gengtype might see these) */
#define DECLARE_CALLBACK(name) void (*name)(int, char *)

/* 10. Final complex example combining everything */
typedef struct GTY(()) MasterType {
    int (*(*vtable[8])(struct MasterType *))[3];
    union {
        struct {
            int counter;
            void (*reset)(void);
        } state;
        char buffer[sizeof(void (*[2])(void))];
    } u;
} MasterType;
