/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Required plugin metadata */
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

/* Create a simple pass instance */
static struct gimple_opt_pass pass_my_dummy = {
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
        .execute = NULL,  /* No execution needed for coverage */
    }
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = {
    .pass = &pass_my_dummy.pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

/* Plugin info structure for PLUGIN_INFO */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "GCC plugin for coverage testing of plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab my_ggc_roots[] = {
    {
        .base = (void *)&my_dummy_pass,
        .nelt = sizeof(my_dummy_pass) / sizeof(void *),
        .stride = sizeof(void *),
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
    
    printf("Plugin %s registered all callbacks successfully\n", plugin_name);
    
    return 0;  /* Success */
}
