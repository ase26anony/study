This script systematically tests the uncovered code by:

1. **Multiple output naming variations**: Uses `-dumpdir`, `-dumpbase`, `-dumpbase-ext` with different values to trigger allocations and frees of `dumpdir`, `dumpbase`, `dumpbase_ext`, and `outbase`.

2. **Sysroot and machine spec overrides**: Uses `--sysroot`, `-march`, `-B`, and `-specs` to modify `target_system_root`, `spec_machine`, and related flags.

3. **Driver mode switching**: Tests help/version requests (setting `print_help_list`, `print_version`, `print_subprocess_help`) and C++ driver mode (`is_cpp_driver`).

4. **Save-temps and dump complexity**: Uses `-save-temps` with various dump flags (`-fdump-*`) to exercise `save_temps_flag` and dump file generation logic.

5. **Sequential state changes**: Runs multiple compiler invocations with different flag combinations to ensure state is properly reset between runs.

6. **Cleanup verification**: Ends with a simple compilation to verify the driver still functions correctly after all state modifications.

The script handles errors gracefully (using `2>/dev/null || true`) since some flag combinations may fail in certain environments (e.g., non-existent sysroots, unavailable linkers). This ensures the script continues to exercise the initialization/cleanup code paths even when individual compilations fail.

To use this test for coverage analysis:
