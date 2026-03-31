/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */

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
    .execute = NULL  /* No actual execution needed */
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = {
    .pass = &my_dummy_pass,
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
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static GTY(()) tree dummy_tree_node = NULL_TREE;

static const struct ggc_root_tab my_ggc_roots[] = {
    {
        .base = (void *)&dummy_tree_node,
        .nelt = 1,
        .stride = sizeof(dummy_tree_node),
        .cb = NULL,
        .pchw = NULL
    },
    /* Required NULL terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin %s is incompatible with this GCC version\n", 
                plugin_name);
        return 1;
    }
    
    printf("Coverage plugin '%s' initializing...\n", plugin_name);
    
    /* ============================================
       Register PLUGIN_PASS_MANAGER_SETUP event
       ============================================ */
    int result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback needed - infrastructure handles it */
        &my_pass_info
    );
    
    if (result) {
        fprintf(stderr, "Error: Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
        return 1;
    }
    printf("  Registered PLUGIN_PASS_MANAGER_SETUP\n");
    
    /* ============================================
       Register PLUGIN_INFO event
       ============================================ */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback needed - infrastructure handles it */
        &my_plugin_info
    );
    
    if (result) {
        fprintf(stderr, "Error: Failed to register PLUGIN_INFO\n");
        return 1;
    }
    printf("  Registered PLUGIN_INFO\n");
    
    /* ============================================
       Register PLUGIN_REGISTER_GGC_ROOTS event
       ============================================ */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback needed - infrastructure handles it */
        my_ggc_roots
    );
    
    if (result) {
        fprintf(stderr, "Error: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
        return 1;
    }
    printf("  Registered PLUGIN_REGISTER_GGC_ROOTS\n");
    
    printf("Coverage plugin '%s' initialized successfully\n", plugin_name);
    
    /* Register an additional callback to verify plugin is active during compilation */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    return 0; /* Success */
}
