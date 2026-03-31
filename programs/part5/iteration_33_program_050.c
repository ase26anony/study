/*
 * GCC Plugin to trigger uncovered lines in plugin.cc
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

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
 * PLUGIN_PASS_MANAGER_SETUP Implementation
 * ============================================ */

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass my_dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "my-dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

/* Create a simple gimple pass */
static unsigned int
execute_my_dummy_pass (void)
{
    /* Do nothing - just a dummy pass */
    return 0;
}

static struct gimple_opt_pass pass_my_dummy_pass = {
    .pass = {
        .type = GIMPLE_PASS,
        .name = "my-dummy-pass",
        .optinfo_flags = OPTGROUP_NONE,
        .tv_id = TV_NONE,
        .properties_required = 0,
        .properties_provided = 0,
        .properties_destroyed = 0,
        .todo_flags_start = 0,
        .todo_flags_finish = 0,
        .execute = execute_my_dummy_pass,
    }
};

/* Register pass info structure */
static struct register_pass_info my_pass_info = {
    .pass = &pass_my_dummy_pass.pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
 * PLUGIN_INFO Implementation
 * ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "This plugin triggers uncovered lines in plugin.cc\n"
            "Specifically targets:\n"
            "  - PLUGIN_PASS_MANAGER_SETUP\n"
            "  - PLUGIN_INFO\n"
            "  - PLUGIN_REGISTER_GGC_ROOTS"
};

/* ============================================
 * PLUGIN_REGISTER_GGC_ROOTS Implementation
 * ============================================ */

/* Dummy structure for GGC roots */
static tree dummy_tree_node = NULL_TREE;
static rtx dummy_rtx = NULL_RTX;

/* GGC root table with dummy entries */
static const struct ggc_root_tab my_ggc_roots[] = {
    {
        .base = (void *)&dummy_tree_node,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    {
        .base = (void *)&dummy_rtx,
        .nelt = 1,
        .stride = sizeof(rtx),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL-terminated array as required */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
 * Plugin Initialization Function
 * ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version)) {
        error("This plugin is only compatible with GCC %s", version->basever);
        return 1;
    }
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP, 
                     NULL,  /* callback can be NULL for registration events */
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
    
    /* Additional callback to verify plugin is active */
    register_callback(plugin_name,
                     PLUGIN_FINISH,
                     NULL,
                     NULL);
    
    printf("Coverage plugin '%s' initialized successfully\n", plugin_name);
    printf("  - Registered PLUGIN_PASS_MANAGER_SETUP\n");
    printf("  - Registered PLUGIN_INFO\n");
    printf("  - Registered PLUGIN_REGISTER_GGC_ROOTS\n");
    
    return 0; /* Success */
}
