/* gengtype-coverage-test.c - ISO C99 test program for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Simplified parser stub to demonstrate the logic being tested */
void consume_balanced(char open, char close) {
    /* This simulates the actual consume_balanced function logic */
    printf("consume_balanced called with '%c' '%c'\n", open, close);
}

void advance(void) {
    /* Simulate advance function */
}

void parse_character(char c) {
    switch (c) {
      default:
        advance();
        break;
      case '(':
        consume_balanced('(', ')');
        break;
      case '[':
        consume_balanced('[', ']');
        break;
      case '{':
        consume_balanced('{', '}');
        break;
    }
}

/* Main test content with all required patterns */
const char* generate_test_gt_content(void) {
    return 
    "/* Test .gt file for gengtype parser coverage */\n"
    "/* Requirement 1: Balanced construct nesting with all delimiter pairs */\n"
    "\n"
    "%typedef struct ComplexType {\n"
    "  int (*callback)(int, char**);  /* Function pointer with parentheses */\n"
    "  void (*arr_ptrs[10])(double);  /* Array of function pointers */\n"
    "  union {\n"
    "    struct { int x; double y; } nested;\n"
    "    int matrix[3][4];\n"
    "  } data;\n"
    "  struct {\n"
    "    char* (*get_name)(void);\n"
    "    void (*set_value)(int (*)(float)); /* Nested function pointer */\n"
    "  } ops;\n"
    "} ComplexType;\n"
    "\n"
    "/* Interleaved comments with balanced delimiters */\n"
    "/* Nested comment test: { [ ( test ) ] } */\n"
    "%struct TreeNode {\n"
    "  struct TreeNode* left;\n"
    "  struct TreeNode* right;\n"
    "  int value;\n"
    "  /* Function pointer array inside struct */\n"
    "  void (*handlers[5])(struct TreeNode*);\n"
    "};\n"
    "\n"
    "// Line comment with delimiters: { [ ( ) ] }\n"
    "%union Variant {\n"
    "  int i;\n"
    "  float f;\n"
    "  char* str;\n"
    "  struct {\n"
    "    int tag;\n"
    "    union {\n"
    "      int int_val;\n"
    "      float float_val;\n"
    "    } u;  // Nested union\n"
    "  } tagged;\n"
    "  /* Array in union */\n"
    "  double coords[3];\n"
    "};\n"
    "\n"
    "/* Preprocessor macro with balanced delimiters */\n"
    "#define DEFINE_ARRAY_TYPE(T, N) struct { T data[N]; }\n"
    "#define CALLBACK_TYPE(R, A) R (*)(A)\n"
    "\n"
    "%typedef DEFINE_ARRAY_TYPE(int, 100) LargeArray;\n"
    "%typedef CALLBACK_TYPE(void, int) SimpleCallback;\n"
    "\n"
    "/* GC roots with complex types */\n"
    "GC roots {\n"
    "  struct TreeNode* root_tree;\n"
    "  ComplexType* complex_data;\n"
    "  Variant* variants[10];\n"
    "  /* Function pointer in GC roots */\n"
    "  int (*compare_func)(const void*, const void*);\n"
    "}\n"
    "\n"
    "/* Enum with complex initializers */\n"
    "%enum State {\n"
    "  INIT = 0,\n"
    "  RUNNING = (1 << 0) | (1 << 1),  /* Expression with parentheses */\n"
    "  PAUSED = {2},  /* Braces in enum (should trigger parser) */\n"
    "  STOPPED = 3\n"
    "};\n"
    "\n"
    "/* Requirement 2: Unbalanced edge cases */\n"
    "/* Unbalanced test cases - these should trigger error handling */\n"
    "/*\n"
    "%struct Unbalanced1 {\n"
    "  int x;\n"
    "  /* Missing closing brace here */\n"
    "\n"
    "%struct Unbalanced2 {\n"
    "  char* names[5;  /* Missing closing bracket */\n"
    "};\n"
    "\n"
    "%typedef int (*unbalanced_func(int, float);  /* Missing closing paren */\n"
    "*/\n"
    "\n"
    "/* Complex nested example hitting all cases */\n"
    "%struct UltimateTest {\n"
    "  /* Parentheses: function pointers */\n"
    "  void (*(*get_callback)(int))(void);\n"
    "  \n"
    "  /* Brackets: multi-dimensional arrays */\n"
    "  int (*(*array_3d[2][3][4])(float))[5];\n"
    "  \n"
    "  /* Braces: nested anonymous structs/unions */\n"
    "  struct {\n"
    "    union {\n"
    "      struct {\n"
    "        int depth;\n"
    "      } level3;\n"
    "      char data[256];\n"
    "    } level2;\n"
    "    float matrix[2][2];\n"
    "  } level1;\n"
    "  \n"
    "  /* Mix of all delimiters */\n"
    "  void (*(*mixed[10])(struct { int x; }))[20];\n"
    "};\n"
    "\n"
    "/* Multiple top-level declarations */\n"
    "%typedef unsigned int uint32_t;\n"
    "%typedef long long int64_t;\n"
    "\n"
    "%struct FinalStruct {\n"
    "  uint32_t flags;\n"
    "  int64_t counter;\n"
    "  /* Array of structs containing function pointers */\n"
    "  struct {\n"
    "    const char* name;\n"
    "    int (*action)(int, char**);\n"
    "  } operations[8];\n"
    "};\n"
    "\n"
    "/* End of test file */\n";
}

/* Alternative test with deliberate unbalanced constructs */
const char* generate_unbalanced_test(void) {
    return
    "/* Test file with unbalanced delimiters for error handling */\n"
    "%struct MissingBrace {\n"
    "  int x;\n"
    "  char y;\n"
    "  /* No closing brace - parser should handle this */\n"
    "\n"
    "%struct MissingParen {\n"
    "  int (*func_ptr(int, float);  /* Missing closing paren */\n"
    "};\n"
    "\n"
    "%struct MissingBracket {\n"
    "  int array[10;  /* Missing closing bracket */\n"
    "  double values[5][3;  /* Multiple missing brackets */\n"
    "};\n"
    "\n"
    "/* Properly balanced after errors */\n"
    "%struct RecoveryTest {\n"
    "  int valid;\n"
    "  char* name;\n"
    "};\n";
}

/* Test with comments containing delimiter-like sequences */
const char* generate_comment_test(void) {
    return
    "/* Comment with { nested [ ( patterns ) ] } that should be ignored */\n"
    "%struct CommentTest {\n"
    "  int value;\n"
    "  // Line comment with [unbalanced ( pattern\n"
    "  char* str;\n"
    "  /* Multi-line comment with\n"
    "     { various [ ( nested ) ] } delimiters\n"
    "     that the parser must skip */\n"
    "  float data[10];\n"
    "};\n"
    "\n"
    "#define MACRO_WITH_DELIMS(X) struct { X field; }\n"
    "/* The above macro contains braces */\n"
    "\n"
    "%typedef MACRO_WITH_DELIMS(int) IntWrapper;\n";
}

void simulate_parsing(const char* content) {
    /* Simple simulation of the parser hitting the switch cases */
    printf("Simulating parsing of %zu characters\n", strlen(content));
    
    for (const char* p = content; *p; p++) {
        parse_character(*p);
    }
}

int main(void) {
    FILE* tmpfile = NULL;
    char tmpname[] = "/tmp/gengtype_test_XXXXXX.gt";
    int fd;
    
    /* Create temporary file */
    fd = mkstemps(tmpname, 3);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return 1;
    }
    
    tmpfile = fdopen(fd, "w");
    if (!tmpfile) {
        perror("Failed to open temporary file");
        close(fd);
        return 1;
    }
    
    /* Write test content with all required patterns */
    const char* test_content = generate_test_gt_content();
    size_t written = fwrite(test_content, 1, strlen(test_content), tmpfile);
    
    if (written != strlen(test_content)) {
        perror("Failed to write test content");
        fclose(tmpfile);
        unlink(tmpname);
        return 1;
    }
    
    fclose(tmpfile);
    
    printf("Generated test file: %s\n", tmpname);
    printf("File size: %zu bytes\n", strlen(test_content));
    
    /* Simulate parsing to demonstrate the logic */
    printf("\n=== Simulating parsing of balanced constructs ===\n");
    simulate_parsing(test_content);
    
    /* Also test unbalanced cases */
    printf("\n=== Simulating parsing of unbalanced constructs ===\n");
    const char* unbalanced = generate_unbalanced_test();
    simulate_parsing(unbalanced);
    
    /* Test comment handling */
    printf("\n=== Simulating parsing with comment delimiters ===\n");
    const char* comment_test = generate_comment_test();
    simulate_parsing(comment_test);
    
    /* In a real test environment, you would invoke gengtype here:
     *   char command[256];
     *   snprintf(command, sizeof(command), "./gengtype -p %s", tmpname);
     *   system(command);
     */
    
    /* Cleanup */
    printf("\nTest file preserved for manual inspection: %s\n", tmpname);
    printf("To clean up manually: rm %s\n", tmpname);
    
    return 0;
}
