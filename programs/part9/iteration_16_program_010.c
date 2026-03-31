/* types1.c - First translation unit with various type definitions */

#include "types_common.h"

/* TYPE_SCALAR: Basic scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
_Bool global_bool = 1;
long long global_long_long = 1234567890LL;

/* TYPE_STRING: String types */
const char* global_string = "Hello, World!";
char global_char_array[] = "String literal";

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
    char label[20];
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    float value;
    char name[32];
} MyStruct;

/* Nested struct with complex members */
struct ComplexStruct {
    struct Point point;
    MyStruct my_struct;
    int counter;
};

/* Struct with __attribute__((packed)) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Struct with alignment attribute */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[4];
    int flags;
};

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;
extern struct Opaque* opaque_ptr;

/* Function using various types */
void process_types(struct Point* p, MyStruct* ms) {
    /* Use the types to ensure they're referenced */
    p->x = global_int;
    ms->data = 100;
}

/* Main function (minimal) */
int main(void) {
    struct Point pt = {10, 20, "origin"};
    MyStruct ms = {42, 3.14f, "test"};
    
    process_types(&pt, &ms);
    
    return 0;
}
