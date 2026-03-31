This comprehensive test program:

1. **Explicitly uses all AVX-512 blend intrinsics** for integer and floating-point types
2. **Relies on auto-vectorization** for half-precision and bfloat16 types
3. **Uses volatile variables** to prevent optimization of mask generation
4. **Implements template metaprogramming** with `if constexpr` to cover all modes
5. **Provides mixed data type functions** to ensure different code paths are taken
6. **Includes a volatile-controlled switch** to force runtime selection of blend operations
7. **Calculates a checksum** to prevent dead code elimination

Compile with:
