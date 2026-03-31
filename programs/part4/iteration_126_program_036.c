#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger the '(' case */
  int (*callback)(int, char*);
  unsigned int bits: (sizeof(int)*8);  /* More parentheses */
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* These will trigger the '[' case */
  int fixed_array[10];
  char variable_array[];
  int multi_dim[5][3];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int id;
  /* This will trigger the '{' case */
  union {
    int as_int;
    float as_float;
    void* as_ptr;
  } data;
  struct {
    int x;
    int y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer */
  void (*init_func)(struct ComplexType*);
  
  /* Brackets in array declaration */
  struct StructWithParens* items[20];
  
  /* Braces for nested struct */
  struct {
    /* Nested parentheses in bitfield */
    unsigned int flags: (sizeof(unsigned int)*8 - 1);
    
    /* Nested brackets in flexible array member */
    char name[];
  } header;
  
  /* More complex: function pointer returning pointer to array */
  int (*(*complex_callback)[5])(void);
};

/* Test case 5: Even more complex nested case */
typedef struct GTY(()) Node {
  int GTY((skip)) value;  /* GTY annotation with parentheses */
  struct Node* GTY((tag("NODE_TAG"))) children[4];  /* Array with GTY options */
  
  union {
    /* Multiple levels of brackets */
    int matrix[3][3];
    /* Parentheses in type cast expression pattern */
    long (*as_long_ptr);
  } GTY((desc("%1.value"))) payload;
} Node_t;

#endif /* TEST_GTY_H */
