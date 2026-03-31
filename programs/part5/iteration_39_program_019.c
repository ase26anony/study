/* test_plugin.c - GCC plugin to test specific plugin events */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

int plugin_is_GPL_compatible;

/* Dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    return 0;
}

static opt_pass *make_dummy_pass(gcc::context *ctxt)
{
    struct opt_pass *pass = new opt_pass();
    pass->name = "dummy-test-pass";
    pass->tv_id = TV_NONE;
    pass->properties_required = 0;
    pass->properties_provided = 0;
    pass->properties_destroyed = 0;
    pass->todo_flags_start = 0;
    pass->todo_flags_finish = 0;
    pass->execute = dummy_pass_execute;
    pass->type = GIMPLE_PASS;
    return pass;
}

/* Dummy GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = NULL,
        .nelt = 0,
        .stride = 0,
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis"
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info_data = {
    .pass = NULL, /* Will be set in plugin_init */
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    struct plugin_pass pass_data;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Create the dummy pass */
    pass_info_data.pass = make_dummy_pass(g);
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL, /* callback is NULL as required by gcc_assert */
                      (void *)&pass_info_data);
    
    /* Register for PLUGIN_INFO event */
    register_callback(plugin_info->base_name,
                      PLUGIN_INFO,
                      NULL,
                      (void *)&plugin_info_data);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(plugin_info->base_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,
                      (void *)dummy_ggc_roots);
    
    return 0;
}
