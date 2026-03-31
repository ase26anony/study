/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
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

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass
   ============================================ */

/* Simple dummy pass structure */
static struct gimple_opt_pass dummy_pass =
{
  {
    GIMPLE_PASS,
    "dummy-pass",           /* name */
    OPTGROUP_NONE,          /* optinfo_flags */
    NULL,                   /* gate */
    NULL,                   /* execute */
    NULL,                   /* sub */
    NULL,                   /* next */
    0,                      /* static_pass_number */
    TV_NONE,                /* tv_id */
    0,                      /* properties_required */
    0,                      /* properties_provided */
    0,                      /* properties_destroyed */
    0,                      /* todo_flags_start */
    0                       /* todo_flags_finish */
  }
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
  .pass = &dummy_pass.pass,          /* Reference to our pass */
  .reference_pass_name = "cfg",      /* Insert after CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER    /* Position in pass list */
};

/* ============================================
   PLUGIN_INFO - Plugin Information
   ============================================ */

static struct plugin_info plugin_info_data = {
  .version = "1.0",
  .help = "Coverage plugin for testing GCC plugin infrastructure\n"
          "This plugin triggers uncovered code in plugin.cc\n"
          "Specifically: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Roots Table
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
    fprintf(stderr, "Error: Plugin version mismatch\n");
    return 1;
  }
  
  printf("Coverage plugin '%s' initializing...\n", plugin_name);
  
  /* ============================================
     Register PLUGIN_PASS_MANAGER_SETUP callback
     ============================================ */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP, 
                    NULL,  /* No callback function needed - infrastructure handles it */
                    &pass_info);
  
  printf("  Registered PLUGIN_PASS_MANAGER_SETUP with dummy pass\n");
  
  /* ============================================
     Register PLUGIN_INFO callback
     ============================================ */
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,  /* No callback function needed */
                    &plugin_info_data);
  
  printf("  Registered PLUGIN_INFO with version: %s\n", plugin_info_data.version);
  
  /* ============================================
     Register PLUGIN_REGISTER_GGC_ROOTS callback
     ============================================ */
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,  /* No callback function needed */
                    dummy_ggc_root_tab);
  
  printf("  Registered PLUGIN_REGISTER_GGC_ROOTS with dummy GGC root table\n");
  
  /* Additional callback to verify plugin is active during compilation */
  register_callback(plugin_name,
                    PLUGIN_ALL_PASSES_START,
                    NULL,
                    NULL);
  
  printf("Coverage plugin initialization complete\n");
  return 0;  /* Success */
}
