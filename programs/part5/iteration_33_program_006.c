/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "plugin-version.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass Definition
   ============================================ */

/* Simple dummy pass structure */
static struct gimple_opt_pass dummy_pass =
{
  .pass.type = GIMPLE_PASS,
  .pass.name = "dummy-coverage-pass",
  .pass.optinfo_flags = OPTGROUP_NONE,
  .pass.has_gate = false,
  .pass.has_execute = false,
  .pass.todo_flags_start = 0,
  .pass.todo_flags_finish = 0
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
  .pass = &dummy_pass.pass,
  .reference_pass_name = "cfg",  /* Insert after CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO - Plugin metadata
   ============================================ */

static struct plugin_info plugin_metadata = {
  .version = "1.0",
  .help = "Coverage test plugin for GCC plugin infrastructure\n"
          "This plugin triggers uncovered code in plugin.cc\n"
          "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
          "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC root tables
   ============================================ */

/* Dummy structure for GGC roots */
static tree dummy_ggc_var = NULL_TREE;

static const struct ggc_root_tab dummy_ggc_roots[] = {
  {
    .base = (void *)&dummy_ggc_var,
    .nelt = 1,
    .stride = sizeof(tree),
    .cb = NULL,
    .pchw = NULL
  },
  /* Terminator */
  { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Check GCC version compatibility */
  if (!plugin_default_version_check(version, &gcc_version)) {
    fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
    return 1;
  }
  
  printf("Coverage plugin %s initializing...\n", plugin_name);
  
  /* ============================================
     Register callbacks for the three target events
     ============================================ */
  
  /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,  /* No callback needed - infrastructure handles it */
                    &pass_info);
  
  /* 2. Register PLUGIN_INFO event */
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,  /* No callback needed */
                    &plugin_metadata);
  
  /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,  /* No callback needed */
                    dummy_ggc_roots);
  
  /* Additional callback to verify plugin is active during compilation */
  register_callback(plugin_name,
                    PLUGIN_START_PARSE_FUNCTION,
                    NULL,  /* Would be a real callback in production */
                    NULL);
  
  printf("Coverage plugin %s registered all target events\n", plugin_name);
  
  return 0;  /* Success */
}
