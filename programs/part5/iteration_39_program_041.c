/* test_plugin.c - GCC plugin to test uncovered lines in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

/* Dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    return 0;
}

static bool dummy_pass_gate(void)
{
    return true;
}

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
    .execute = dummy_pass_execute,
    .gate = dummy_pass_gate
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin.cc lines 458-470"
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    { NULL, 0, sizeof(void *), NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    struct register_pass_info pass_info;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP */
    pass_info.pass = &dummy_pass;
    pass_info.reference_pass_name = "ssa";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    
    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,
                      &pass_info);
    
    /* Register for PLUGIN_INFO */
    register_callback(plugin_info->base_name,
                      PLUGIN_INFO,
                      NULL,
                      &plugin_info_data);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback(plugin_info->base_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,
                      dummy_ggc_roots);
    
    return 0;
}
