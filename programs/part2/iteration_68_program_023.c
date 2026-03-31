// When debugging why a particular optimization isn't happening:
switch_dump(stderr);
dump_insn_rtx_1(problematic_insn, debug_insn_rtx_flags);
sel_print("\n");
restore_dump();
