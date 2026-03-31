/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "context.h"
#include "plugin-version.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

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

/* Pass info structure for registration */
static struct register_pass_info my_pass_info = {
    .pass = &my_dummy_pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "This plugin triggers uncovered code in plugin.cc\n"
            "It registers pass manager setup, plugin info, and GGC roots."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root table entry */
static const struct ggc_root_tab my_ggc_root_tab[] = {
    {
        .base = (void *)&my_dummy_pass,
        .nelt = sizeof(my_dummy_pass),
        .stride = sizeof(my_dummy_pass),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* Terminator */
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
    
    printf("Plugin %s initializing...\n", plugin_name);
    
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
                     my_ggc_root_tab);
    
    printf("Plugin %s successfully registered all target events\n", plugin_name);
    
    return 0;  /* Success */
}
