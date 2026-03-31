/* types1.c - First translation unit with comprehensive type definitions */

#include "types_common.h"

/* TYPE_SCALAR: Basic scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
_Bool global_bool = 1;
long global_long = 1000L;
unsigned int global_uint = 100U;

/* TYPE_STRING: String types */
const char* global_string = "Hello, World!";
char* mutable_string = "Mutable";

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {
    int data;
    char tag;
} MyStruct;

/* Nested struct with complex members */
struct ComplexData {
    struct Point location;
    MyStruct metadata;
    const char* name;
};

/* Struct with GCC attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((aligned(8)));

/* TYPE_UNDEFINED: Forward declaration */
struct Opaque;
extern struct Opaque* opaque_ptr;

/* Function using various types */
void process_data(struct ComplexData* data) {
    /* Function body - gengtype cares about declarations, not definitions */
}

/* Main function (minimal) */
int main(void) {
    return 0;
}
