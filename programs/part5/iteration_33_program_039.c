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
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass my_pass;
static struct ggc_root_tab dummy_roots[];

/* Custom GIMPLE pass definition */
static unsigned int
execute_my_pass (void)
{
    /* Do nothing - just a dummy pass for coverage */
    return 0;
}

static bool
gate_my_pass (void)
{
    /* Always run this pass */
    return true;
}

/* Define our custom pass */
static struct opt_pass my_pass = 
{
    .type = GIMPLE_PASS,
    .name = "my-coverage-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .execute = execute_my_pass,
    .gate = gate_my_pass,
};

/* Dummy GGC root table entry */
static tree dummy_tree = NULL_TREE;

static struct ggc_root_tab dummy_roots[] = {
    {
        .base = (void *)&dummy_tree,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator as required */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin information structure */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "GCC plugin to trigger coverage for plugin.cc events"
};

/* Plugin initialization function */
int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    struct register_pass_info pass_info;
    const char *plugin_name = plugin_info->base_name;
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    pass_info.pass = &my_pass;
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* No callback function needed */
                     &pass_info);
    
    /* Register callback for PLUGIN_INFO */
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,
                     &my_plugin_info);
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,
                     dummy_roots);
    
    return 0;
}
