switch_dump (stderr);           // Switch output to stderr
dump_insn_rtx_1 (insn, debug_insn_rtx_flags);  // Print RTL representation of instruction
sel_print ("\n");               // Print newline
restore_dump ();                // Restore original output stream
