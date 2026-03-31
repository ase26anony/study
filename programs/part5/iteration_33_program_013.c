/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS
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

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* Global plugin name */
static const char *plugin_name = "coverage_plugin";

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
struct dummy_pass_data {
    struct opt_pass pass;
};

/* Gate function for dummy pass (always returns true) */
static bool
dummy_pass_gate (void)
{
    return true;
}

/* Execute function for dummy pass (does nothing) */
static unsigned int
dummy_pass_exec (void)
{
    return 0;
}

/* Create and return a dummy pass */
static struct opt_pass *
make_dummy_pass (void)
{
    struct dummy_pass_data *pass_data;
    
    pass_data = XCNEW (struct dummy_pass_data);
    
    pass_data->pass.type = GIMPLE_PASS;
    pass_data->pass.name = "dummy-coverage-pass";
    pass_data->pass.optinfo_flags = OPTGROUP_NONE;
    pass_data->pass.gate = dummy_pass_gate;
    pass_data->pass.execute = dummy_pass_exec;
    pass_data->pass.todo_flags_start = 0;
    pass_data->pass.todo_flags_finish = 0;
    
    return &pass_data->pass;
}

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

/* Plugin info structure */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "GCC plugin to trigger coverage for plugin.cc\n"
            "This plugin registers dummy components to exercise\n"
            "PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root structure */
static tree dummy_tree = NULL_TREE;

/* GGC root table with one dummy entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_tree,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    struct register_pass_info pass_info;
    struct opt_pass *dummy_pass;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version))
        return 1;
    
    /* Create dummy pass */
    dummy_pass = make_dummy_pass ();
    
    /* Set up pass registration info for PLUGIN_PASS_MANAGER_SETUP */
    pass_info.pass = dummy_pass;
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback (plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback function needed */
                      &pass_info);
    
    /* Register callback for PLUGIN_INFO */
    register_callback (plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed */
                      &plugin_info_data);
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback (plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed */
                      dummy_ggc_roots);
    
    /* Additional registration to ensure plugin is active during compilation */
    register_callback (plugin_name, PLUGIN_ALL_PASSES_START, NULL, NULL);
    
    return 0;
}
