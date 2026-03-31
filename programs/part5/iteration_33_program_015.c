/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "plugin-version.h"
#include "ggc.h"

/* Required plugin declarations */
int plugin_is_GPL_compatible = 1;

/* Global plugin name */
static const char *plugin_name = "coverage_plugin";

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int
execute_dummy_pass (void)
{
  /* Do nothing, just return */
  return 0;
}

static bool
gate_dummy_pass (void)
{
  /* Always enable this pass */
  return true;
}

static struct gimple_opt_pass dummy_pass = 
{
  {
    GIMPLE_PASS,
    "dummy-pass",               /* name */
    OPTGROUP_NONE,              /* optinfo_flags */
    gate_dummy_pass,            /* gate */
    execute_dummy_pass,         /* execute */
    NULL,                       /* sub */
    NULL,                       /* next */
    0,                          /* static_pass_number */
    TV_NONE,                    /* tv_id */
    0,                          /* properties_required */
    0,                          /* properties_provided */
    0,                          /* properties_destroyed */
    0,                          /* todo_flags_start */
    0                           /* todo_flags_finish */
  }
};

/* Pass registration info */
static struct register_pass_info pass_info = {
  .pass = &dummy_pass.pass,
  .reference_pass_name = "cfg",  /* Insert after CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info plugin_info_data = {
  .version = "1.0",
  .help = "Plugin to trigger uncovered code in GCC's plugin infrastructure"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy structure for GGC roots */
static tree dummy_tree = NULL_TREE;

static const struct ggc_root_tab dummy_ggc_root_tab[] = {
  {
    .base = (void *)&dummy_tree,
    .nelt = 1,
    .stride = sizeof(tree),
    .cb = NULL,
    .pchw = NULL
  },
  /* Terminating NULL entry as required */
  { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  /* Verify GCC version compatibility */
  if (!plugin_default_version_check (version, &gcc_version)) {
    fprintf(stderr, "Plugin version mismatch\n");
    return 1;
  }

  /* Store plugin name */
  plugin_name = plugin_info->base_name;

  /* Register PLUGIN_PASS_MANAGER_SETUP event */
  register_callback (plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP, 
                     NULL,  /* No callback needed for registration */
                     &pass_info);

  /* Register PLUGIN_INFO event */
  register_callback (plugin_name,
                     PLUGIN_INFO,
                     NULL,  /* No callback needed for registration */
                     &plugin_info_data);

  /* Register PLUGIN_REGISTER_GGC_ROOTS event */
  register_callback (plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,  /* No callback needed for registration */
                     dummy_ggc_root_tab);

  /* Also register a simple callback to verify plugin is working */
  register_callback (plugin_name,
                     PLUGIN_START_PARSE_FUNCTION,
                     NULL,
                     NULL);

  return 0;
}
