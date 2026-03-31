// Without this struct trick, some toolchains might optimize away
// DWARF info for typedef-only types that aren't "used"

// This ensures encrypted_string gets proper debug info even if
// it's just a typedef alias for a built-in type
