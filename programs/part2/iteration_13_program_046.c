This program implements all the requested features:

1. **AVX-512 Blend Intrinsic Usage**: Uses all `_mm512_mask_blend_*` intrinsics with constant masks (0xAA..., 0xCC..., 0x55... patterns).

2. **Explicit Vector Type Declarations**: Uses `__m512i`, `__m512`, `__m512d` types. For HF/BF modes, uses `__m512i` with casting via `_mm512_castsi512_ph/pbh`.

3. **Loop-Based Blend Operations**: Each blend case includes loops where masks vary based on loop indices (`i % 3`, `i & 1`, etc.).

4. **Multi-Stage Vector Processing Pipeline**: Chains multiple blend operations with arithmetic operations (add, multiply, shift) between them.

5. **Conditional Compilation**: Uses `#ifdef __AVX512BW__`, `#ifdef __AVX512F__`, and `#ifdef __AVX512BF16__` to guard specific code paths.

6. **Inline Assembly for Mode Hinting**: For HF/BF modes where compiler might optimize away, uses inline assembly with "v" constraints to force materialization.

7. **Execution Flow**: Initializes arrays with distinct patterns, performs blend operations, stores results, computes checksums, and prints final output.

**Compilation commands:**
