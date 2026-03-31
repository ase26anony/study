/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible;

/* ============================================
 * 1. Custom Pass for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Simple dummy pass that does nothing */
static unsigned int
execute_dummy_pass (void)
{
  /* This pass does nothing, just for demonstration */
  return 0;
}

static bool
gate_dummy_pass (void)
{
  /* Always run this pass */
  return true;
}

/* Define the pass structure */
namespace {
const pass_data dummy_pass_data =
{
  GIMPLE_PASS,        /* type */
  "dummy-pass",       /* name */
  OPTGROUP_NONE,      /* optinfo_flags */
  TV_NONE,            /* tv_id */
  PROP_gimple_any,    /* properties_required */
  0,                  /* properties_provided */
  0,                  /* properties_destroyed */
  0,                  /* todo_flags_start */
  0                   /* todo_flags_finish */
};

class pass_dummy : public gimple_opt_pass
{
public:
  pass_dummy(gcc::context *ctxt)
    : gimple_opt_pass(dummy_pass_data, ctxt)
  {}

  /* opt_pass methods: */
  virtual bool gate (function *) { return gate_dummy_pass(); }
  virtual unsigned int execute (function *) { return execute_dummy_pass(); }
};
} /* anon namespace */

/* Create an instance of our dummy pass */
static struct opt_pass *
make_pass_dummy (gcc::context *ctxt)
{
  return new pass_dummy(ctxt);
}

/* ============================================
 * 2. GGC Roots Data for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy structure for GGC roots */
static GTY(()) tree dummy_tree_node = NULL_TREE;

static const struct ggc_root_tab dummy_ggc_root_tab[] = {
  {
    .base = (void *)&dummy_tree_node,
    .nelt = 1,
    .stride = sizeof(dummy_tree_node),
    .cb = NULL,
    .pchw = NULL
  },
  /* Required NULL terminator */
  { NULL, 0, 0, NULL, NULL }
};

/* ============================================
 * 3. Plugin Info for PLUGIN_INFO
 * ============================================ */

static struct plugin_info my_plugin_info = {
  .version = "1.0",
  .help = "GCC plugin to trigger uncovered code in plugin.cc\n"
          "This plugin registers dummy components to exercise:\n"
          "1. PLUGIN_PASS_MANAGER_SETUP\n"
          "2. PLUGIN_INFO\n"
          "3. PLUGIN_REGISTER_GGC_ROOTS"
};

/* ============================================
 * Main Plugin Initialization Function
 * ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  struct plugin_pass pass_info;
  struct register_pass_info reg_pass_info;
  const char *plugin_name = plugin_info->base_name;
  
  /* Check GCC version compatibility */
  if (!plugin_default_version_check (version, &gcc_version))
    return 1;
  
  /* ============================================
   * Register PLUGIN_PASS_MANAGER_SETUP callback
   * ============================================ */
  
  /* Create pass registration info */
  memset(&reg_pass_info, 0, sizeof(reg_pass_info));
  reg_pass_info.pass = make_pass_dummy (g);
  reg_pass_info.reference_pass_name = "ssa";
  reg_pass_info.ref_pass_instance_number = 1;
  reg_pass_info.pos_op = PASS_POS_INSERT_AFTER;
  
  /* Register the callback - note: callback function is NULL as expected */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,  /* No callback function needed */
                    &reg_pass_info);
  
  /* ============================================
   * Register PLUGIN_INFO callback
   * ============================================ */
  
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,  /* No callback function needed */
                    &my_plugin_info);
  
  /* ============================================
   * Register PLUGIN_REGISTER_GGC_ROOTS callback
   * ============================================ */
  
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,  /* No callback function needed */
                    dummy_ggc_root_tab);
  
  /* Additional callback to demonstrate plugin is working */
  register_callback(plugin_name,
                    PLUGIN_FINISH,
                    NULL,
                    NULL);
  
  return 0; /* Success */
}
