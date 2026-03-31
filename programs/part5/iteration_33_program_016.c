/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc events */

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

/* Simple dummy pass structure */
struct dummy_pass_data {
    struct opt_pass pass;
};

/* Dummy pass execution function */
static unsigned int
execute_dummy_pass (void)
{
    /* Do nothing, just return */
    return 0;
}

/* Create a dummy pass for registration */
static struct opt_pass *
make_dummy_pass (void)
{
    struct dummy_pass_data *pass_data;
    
    pass_data = XCNEW (struct dummy_pass_data);
    
    pass_data->pass.type = GIMPLE_PASS;
    pass_data->pass.name = "dummy-coverage-pass";
    pass_data->pass.optinfo_flags = OPTGROUP_NONE;
    pass_data->pass.tv_id = TV_NONE;
    pass_data->pass.properties_required = 0;
    pass_data->pass.properties_provided = 0;
    pass_data->pass.properties_destroyed = 0;
    pass_data->pass.todo_flags_start = 0;
    pass_data->pass.todo_flags_finish = 0;
    pass_data->pass.execute = execute_dummy_pass;
    pass_data->pass.sub = NULL;
    pass_data->pass.next = NULL;
    pass_data->pass.static_pass_number = 0;
    
    return &pass_data->pass;
}

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Coverage test plugin for plugin.cc events\n"
            "This plugin triggers PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root table entry */
static GTY(()) tree dummy_tree_node = NULL_TREE;

static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = (void *)&dummy_tree_node,
        .nelt = 1,
        .stride = sizeof(dummy_tree_node),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator as required */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    struct register_pass_info pass_info;
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
       Trigger PLUGIN_PASS_MANAGER_SETUP event
       ============================================ */
    
    /* Create and populate pass registration info */
    memset(&pass_info, 0, sizeof(pass_info));
    pass_info.pass = make_dummy_pass();
    pass_info.reference_pass_name = "ssa";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP, 
                      NULL,  /* No callback function needed */
                      &pass_info);
    
    printf("  Registered PLUGIN_PASS_MANAGER_SETUP event\n");
    
    /* ============================================
       Trigger PLUGIN_INFO event
       ============================================ */
    
    /* Register callback for PLUGIN_INFO */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed */
                      &my_plugin_info);
    
    printf("  Registered PLUGIN_INFO event\n");
    
    /* ============================================
       Trigger PLUGIN_REGISTER_GGC_ROOTS event
       ============================================ */
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed */
                      dummy_ggc_root_tab);
    
    printf("  Registered PLUGIN_REGISTER_GGC_ROOTS event\n");
    
    /* ============================================
       Optional: Register for other events to ensure
       our pass gets executed
       ============================================ */
    
    /* Register for all passes to see our dummy pass in action */
    register_callback(plugin_name, 
                      PLUGIN_ALL_PASSES_START, 
                      NULL, 
                      NULL);
    
    printf("Plugin %s initialization complete\n", plugin_name);
    
    return 0;  /* Success */
}
