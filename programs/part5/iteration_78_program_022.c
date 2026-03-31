/*
 * GCC plugin designed to trigger specific uncovered lines in plugin.cc
 * Lines 458-470: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "pass_manager.h"
#include "ggc.h"

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass dummy_pass = {
  .type = SIMPLE_IPA_PASS,
  .name = "dummy-coverage-pass",
  .gate = NULL,  /* Gate always returns false - pass won't execute */
  .execute = NULL,
  .sub = NULL,
  .next = NULL,
  .static_pass_number = 0,
  .tv_id = TV_NONE,
  .properties_required = 0,
  .properties_provided = 0,
  .properties_destroyed = 0,
  .todo_flags_start = 0,
  .todo_flags_finish = 0
};

/* Pass info structure for registration */
static struct register_pass_info pass_info = {
  .pass = &dummy_pass,
  .reference_pass_name = "ssa",  /* Insert after SSA pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
  .version = "1.0",
  .help = "Plugin to trigger uncovered lines in plugin.cc\n"
          "Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS"
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab ggc_root_table[] = {
  {
    .base = &dummy_ggc_root,
    .nelt = 1,
    .stride = sizeof(dummy_ggc_root),
    .cb = NULL,
    .pchw = NULL
  },
  { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_arg,
                struct plugin_gcc_version *version)
{
  int result;

  /* Set global plugin name */
  plugin_name = plugin_info_arg->base_name;

  /* Register for PLUGIN_PASS_MANAGER_SETUP with NULL callback */
  result = register_callback(plugin_name, 
                            PLUGIN_PASS_MANAGER_SETUP,
                            NULL,  /* NULL callback as required by uncovered code */
                            &pass_info);
  
  if (result != 1) {
    fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
    return 0;
  }

  /* Register for PLUGIN_INFO with NULL callback */
  result = register_callback(plugin_name,
                            PLUGIN_INFO,
                            NULL,  /* NULL callback as required by uncovered code */
                            &plugin_info_data);
  
  if (result != 1) {
    fprintf(stderr, "Failed to register PLUGIN_INFO\n");
    return 0;
  }

  /* Register for PLUGIN_REGISTER_GGC_ROOTS with NULL callback */
  result = register_callback(plugin_name,
                            PLUGIN_REGISTER_GGC_ROOTS,
                            NULL,  /* NULL callback as required by uncovered code */
                            ggc_root_table);
  
  if (result != 1) {
    fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
    return 0;
  }

  /* Optional: Register for finish event to confirm plugin executed */
  result = register_callback(plugin_name,
                            PLUGIN_FINISH,
                            NULL,
                            NULL);
  
  return 1;  /* Success */
}
