/* type_zoo.h - Header file with declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_llong;
extern _Bool global_bool;

/* String type */
extern char *global_string;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int *ptr;
    double data[5];
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int id;
    char name[32];
    void *metadata;
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_chars[4];
    void *as_ptr;
};

typedef union {
    long long timestamp;
    double precision;
} TimestampUnion;

/* Pointer types */
extern int *global_int_ptr;
extern int **global_int_ptr_ptr;
extern struct SimpleStruct *global_struct_ptr;
extern void (*global_func_ptr)(void);

/* Array types */
extern int global_int_array[10];
extern struct SimpleStruct global_struct_array[5];
extern char *global_ptr_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

#ifdef __GNUC__
struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    void *generic;
};
#endif

/* Function declarations */
void process_scalars(int seed);
void process_structs(void);
void process_pointers(void);
void process_arrays(void);
void process_unions(void);
void process_callbacks(void);

#endif /* TYPE_ZOO_H */
