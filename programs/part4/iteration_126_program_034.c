#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger case '(': */
  int (*callback)(int, char*);
  /* More parentheses in bitfield */
  unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* This will trigger case '[': */
  int fixed_array[10];
  char variable_array[];
  /* Multi-dimensional array */
  double matrix[3][4];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int tag;
  /* This will trigger case '{': */
  union {
    int as_int;
    float as_float;
    void* as_ptr;
  } data;
  /* Another nested struct */
  struct {
    short x;
    short y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer type */
  void (*init_func)(struct ComplexType*);
  
  /* Brackets in array declaration */
  struct StructWithParens* items[5];
  
  /* Braces for nested anonymous struct */
  struct {
    int count;
    /* Array within nested struct */
    char names[20][50];
  } metadata;
  
  /* Function pointer returning pointer to array */
  int (*(*complex_callback)[5])(void);
  
  /* Flexible array member at the end */
  unsigned char extra_data[];
};

/* Test case 5: Pointer to struct with GTY marker */
typedef struct GTY(()) Node {
  struct Node* GTY((skip)) next;  /* Skip this field for GC */
  int data;
  /* Array of function pointers */
  void (*handlers[3])(struct Node*);
} Node_t;

/* Test case 6: Union with all bracket types */
union GTY(()) AllBracketsUnion {
  /* Parentheses in bitfield expression */
  unsigned int flags: (8 * sizeof(int) - 4);
  
  /* Brackets in array */
  long values[2];
  
  /* Braces for anonymous struct */
  struct {
    float x;
    float y;
    /* Array in anonymous struct */
    int coords[2];
  } position;
};

#endif /* TEST_GTY_H */
