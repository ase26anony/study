#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "ggc.h"
#include <stdio.h>

int plugin_is_GPL_compatible;

/* Dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

/* Pass registration info */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure */
static struct plugin_info plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin.cc lines 458-470"
};

/* Minimal GGC root table - just a terminator */
static const struct ggc_root_tab ggc_root_tab[] = {
    { NULL, 0, sizeof(void *), NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int event;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin version mismatch\n");
        return 1;
    }
    
    printf("Plugin initialized: %s\n", plugin_info_args->base_name);
    
    /* Register for PLUGIN_INFO event */
    event = PLUGIN_INFO;
    register_callback(plugin_info_args->base_name, event,
                     NULL, (void *)&plugin_info);
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    event = PLUGIN_PASS_MANAGER_SETUP;
    register_callback(plugin_info_args->base_name, event,
                     NULL, (void *)&pass_info);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    event = PLUGIN_REGISTER_GGC_ROOTS;
    register_callback(plugin_info_args->base_name, event,
                     NULL, (void *)ggc_root_tab);
    
    return PLUGIN_SUCCESS;
}
