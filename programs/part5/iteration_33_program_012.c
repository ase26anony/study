/*
 * GCC Plugin to trigger uncovered lines in plugin.cc
 * Specifically targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass Definition
   ============================================ */

/* Simple dummy pass structure */
static unsigned int
execute_my_pass (void)
{
    /* Do nothing - just a dummy pass for coverage */
    return 0;
}

static bool
gate_my_pass (void)
{
    /* Always enable this pass */
    return true;
}

const pass_data my_pass_data = {
    .type = GIMPLE_PASS,
    .name = "my_dummy_pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

class my_pass : public gimple_opt_pass {
public:
    my_pass(gcc::context *ctxt)
        : gimple_opt_pass(my_pass_data, ctxt) {}
    
    virtual unsigned int execute(function *) { return execute_my_pass(); }
    virtual bool gate(void) { return gate_my_pass(); }
};

static struct opt_pass *
make_my_pass(void)
{
    return new my_pass(g);
}

/* ============================================
   PLUGIN_INFO - Plugin Information Structure
   ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "GCC Plugin for coverage testing of plugin infrastructure\n"
            "This plugin triggers PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO,\n"
            "and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Root Table
   ============================================ */

/* Dummy structure for GGC roots */
static GTY(()) tree dummy_tree_node = NULL_TREE;

static const struct ggc_root_tab my_ggc_root_tab[] = {
    {
        .base = (void *)&dummy_tree_node,
        .nelt = 1,
        .stride = sizeof(dummy_tree_node),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminating NULL entry */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Main Plugin Initialization Function
   ============================================ */

int
plugin_init(struct plugin_name_args *plugin_info,
            struct plugin_gcc_version *version)
{
    struct register_pass_info pass_info;
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
       Trigger PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    
    /* Create and populate pass registration info */
    memset(&pass_info, 0, sizeof(pass_info));
    pass_info.pass = make_my_pass();
    pass_info.reference_pass_name = "ssa";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    
    /* Register callback for pass manager setup */
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* No callback function needed */
                     &pass_info);
    
    /* ============================================
       Trigger PLUGIN_INFO
       ============================================ */
    
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,  /* No callback function needed */
                     &my_plugin_info);
    
    /* ============================================
       Trigger PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,  /* No callback function needed */
                     my_ggc_root_tab);
    
    printf("Plugin %s successfully registered all events\n", plugin_name);
    
    return 0;
}
