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

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP: Custom Pass
   ============================================ */

/* Simple dummy pass structure */
static unsigned int execute_my_pass(void)
{
    /* Do nothing, just a placeholder */
    return 0;
}

static bool gate_my_pass(void)
{
    /* Always run this pass */
    return true;
}

static struct opt_pass my_pass = 
{
    .type = GIMPLE_PASS,
    .name = "my-dummy-pass",
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

static struct opt_pass *make_my_pass(void)
{
    return &my_pass;
}

/* Pass registration info */
static struct register_pass_info my_pass_info = 
{
    .pass = make_my_pass(),
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO: Plugin metadata
   ============================================ */

static struct plugin_info my_plugin_info = 
{
    .version = "1.0",
    .help = "This plugin triggers uncovered code in plugin.cc\n"
            "It registers a dummy pass, plugin info, and GGC roots."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS: GGC Root Table
   ============================================ */

/* Dummy structure for GGC roots */
static GTY(()) tree my_ggc_root = NULL_TREE;

static const struct ggc_root_tab my_ggc_roots[] = 
{
    {
        .base = (void *)&my_ggc_root,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization
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
    
    printf("Plugin %s: Initializing to trigger uncovered code...\n", plugin_name);
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP, 
                     NULL,  /* No callback needed for registration */
                     &my_pass_info);
    
    /* 2. Register PLUGIN_INFO event */
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,
                     &my_plugin_info);
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,
                     my_ggc_roots);
    
    printf("Plugin %s: All three target events registered\n", plugin_name);
    
    return 0;
}
