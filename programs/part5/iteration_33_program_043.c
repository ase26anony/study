/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
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

/* Forward declarations */
static struct opt_pass my_pass;
static struct ggc_root_tab my_ggc_roots[];

/* Custom pass structure */
static unsigned int my_pass_execute (void)
{
    /* Do nothing - just a dummy pass for registration */
    return 0;
}

static bool my_pass_gate (void)
{
    /* Always run this pass */
    return true;
}

static struct opt_pass my_pass = {
    .type = GIMPLE_PASS,
    .name = "my-dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .execute = my_pass_execute,
    .gate = my_pass_gate
};

/* Data for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = {
    .pass = &my_pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Data for PLUGIN_INFO */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Coverage test plugin for plugin.cc uncovered lines"
};

/* Data for PLUGIN_REGISTER_GGC_ROOTS */
static GTY(()) tree my_root_tree = NULL_TREE;

static struct ggc_root_tab my_ggc_roots[] = {
    {
        .base = (void *)&my_root_tree,
        .nelt = 1,
        .stride = sizeof(my_root_tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version))
        return 1;
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP, 
                      NULL,  /* No callback function needed */
                      &my_pass_info);
    
    /* Register callback for PLUGIN_INFO */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,
                      &my_plugin_info);
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,
                      my_ggc_roots);
    
    /* Additional callback to ensure plugin runs during compilation */
    register_callback(plugin_name,
                      PLUGIN_ALL_PASSES_START,
                      NULL,
                      NULL);
    
    return 0;
}
