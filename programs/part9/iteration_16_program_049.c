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
extern char global_char_array[50];

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char* name;
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef char StringArray[20][30];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Complex nested type for deep traversal */
struct ComplexContainer {
    union Value values[5];           /* Array of unions */
    MyStruct* struct_ptrs[8];        /* Array of pointers to user structs */
    Comparator compare_func;         /* Callback function pointer */
    struct ComplexContainer* next;   /* Pointer to self type */
};

/* Packed struct with alignment */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
};

/* Deprecated typedef */
typedef int OldIntType __attribute__((deprecated));

/* Function declarations */
extern void process_data(struct ComplexContainer* container);
extern union Value create_value(int type);

#endif /* TEST_TYPES_H */
