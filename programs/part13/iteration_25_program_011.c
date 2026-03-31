   gimple *g = gimple_build_call_internal (IFN_GOMP_USE_SIMT, 0);
   tree cond = create_tmp_var_raw (integer_type_node);
   gimple_call_set_lhs (g, cond);
