#ifndef TYPES_H
#define TYPES_H

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
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char tag;
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
} __attribute__((aligned(16)));

/* TYPE_POINTER: Various pointer types */
extern int* int_ptr;
extern struct Point* point_ptr;
extern void* void_ptr;

/* TYPE_ARRAY: Array types */
extern int int_array[10];
extern char char_array[20];
extern struct Point point_array[5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, char*);

/* Complex nested type for deep traversal */
struct ComplexType {
    union Value values[4];           /* Array of unions */
    struct Point* points[8];         /* Array of pointers to structs */
    comparator cmp_func;             /* Callback member */
    callback_func cb_array[3];       /* Array of callbacks */
    volatile const int* volatile* complex_ptr; /* Complex pointer */
};

/* Packed struct with pragma */
#pragma pack(push, 1)
struct PackedData {
    char type;
    int value;
    short count;
} __attribute__((deprecated));
#pragma pack(pop)

/* Function declarations using callbacks */
void register_callback(callback_func cb);
int sort_data(void* data, int count, comparator cmp);

#endif /* TYPES_H */
