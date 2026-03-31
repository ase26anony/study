**Key Design Rationale:**

1. **Volatile Control Flow**: Each test function takes a `volatile int` parameter and uses it in an `if` condition, ensuring the parser must analyze the erroneous code block.

2. **Isolated Functions**: Each function targets exactly one uncovered case with `__attribute__((noinline, noipa))` preventing inter-function optimization and ensuring independent parsing.

3. **Valid Context First**: Each function starts with `int local = 0;` to establish normal parsing state before the error.

4. **Specific Error Patterns**:
   - `RT_EXTERN`: Linkage spec without `extern`
   - `RT_STATIC_ASSERT`: Assertion-like expression without keyword
   - `RT_DECLTYPE`: Trailing return type without `decltype`
   - `RT_OPERATOR`: Operator overload syntax without `operator`
   - `RT_CLASS`: Class definition without `class`
   - `RT_TEMPLATE`: Template parameter list without `template`
   - `RT_NAMESPACE`: Namespace body without `namespace`
   - `RT_USING`: Using-directive without `using`
   - `RT_ASM`: Inline assembly without `asm`
   - `RT_TRY`: Try-block without `try`

5. **Main Driver**: The loop with volatile iteration counter ensures all functions are compiled and their bodies parsed, even if not executed at runtime.

**Compilation Commands for Coverage Analysis:**
