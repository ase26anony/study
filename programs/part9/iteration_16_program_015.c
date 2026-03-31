#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern volatile const int volatile_const_int;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char mutable_string[50];

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
};

/* TYPE_POINTER: Various pointer types */
extern int* int_ptr;
extern struct Point* point_ptr;
extern void* void_ptr;

/* TYPE_ARRAY: Array types */
extern int int_array[10];
extern struct Point point_array[5];
extern char* string_array[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
};

typedef __attribute__((deprecated)) int deprecated_int;

/* For TYPE_LANG_STRUCT - GCC internal structures */
#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__((visibility("hidden"))) HiddenStruct {
    int secret_data;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_TYPES_H */
