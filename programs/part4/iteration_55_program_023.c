case OMP_CLAUSE_DEPEND_IN:
  pp_cxx_ws_string (this, " in");
  break;
case OMP_CLAUSE_DEPEND_INOUT:
  pp_cxx_ws_string (this, " inout");
  break;
case OMP_CLAUSE_DEPEND_OUT:
  pp_cxx_ws_string (this, " out");
  break;
case OMP_CLAUSE_DEPEND_MUTEXINOUTSET:
  pp_cxx_ws_string (this, " mutexinoutset");
  break;
case OMP_CLAUSE_DEPEND_INOUTSET:
  pp_cxx_ws_string (this, " inoutset");
  break;
case OMP_CLAUSE_DEPEND_LAST:
  pp_cxx_ws_string (this, " destroy");
  break;
default:
  break;
