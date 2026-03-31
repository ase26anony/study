```cpp
// test_parser_errors.cc
// Compile with: g++ -std=c++11 -fsyntax-only -fdiagnostics-parseable-fixits test_parser_errors.cc
// Or for more coverage: g++ -std=c++11 -O1 -fno-exceptions -c test_parser_errors.cc
// Or: g++ -std=c++11 -O0 -fpermissive -Wno-error -c test_parser_errors.cc

#include <iostream>

// Valid code to establish normal parsing state
void valid_function() {
    int x = 42;
    std::cout << "Valid function" << std::endl;
}

// 1. RT_EXTERN: Expected 'extern' in linkage specification
void test_extern_error() {
    // Missing 'extern' before linkage specifier
    "C" {
        int y;
    }
}

// 2. RT_STATIC_ASSERT: Expected 'static_assert' keyword
void test_static_assert_error() {
    // Missing 'static_assert' keyword
    (sizeof(int) == 4, "int must be 4 bytes");
}

// 3. RT_DECLTYPE: Expected 'decltype' in trailing return type
void test_decltype_error() {
    // Missing 'decltype' in trailing return type
    auto foo() -> (x);
}

// 4. RT_OPERATOR: Expected 'operator' in conversion function
class TestClass {
public:
    // Missing 'operator' keyword for conversion operator
    int() const;
    
    void valid_method() {}
};

// 5. RT_CLASS: Expected 'class' in class definition
void test_class_error() {
    // Missing 'class' keyword
    MyClass {
    public:
        int x;
    };
}

// 6. RT_TEMPLATE: Expected 'template' keyword
// Missing 'template' keyword before template parameter list
<typename T>
void template_func() {}

// 7. RT_NAMESPACE: Expected 'namespace' keyword
// Missing 'namespace' keyword
MyNamespace {
    int z;
}

// 8. RT_USING: Expected 'using' keyword
void test_using_error() {
    // Missing 'using' keyword for using-directive
    namespace std;
    
    // Missing 'using' keyword for using-declaration
    std::cout;
}

// 9. RT_ASM: Expected 'asm' keyword
void test_asm_error() {
    // Missing 'asm' keyword for inline assembly
    volatile ("nop");
}

// 10. RT_TRY: Expected 'try' keyword
void test_try_error() {
    // Missing 'try' keyword before try-block
    {
        throw 42;
    } catch (...) {
        std::cout << "Caught" << std::endl;
    }
}

// Valid main function to keep overall structure valid
int main() {
    valid_function();
    return 0;
}
```
