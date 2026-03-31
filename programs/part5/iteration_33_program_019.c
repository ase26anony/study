/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);
static void my_pass_execute(void);

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP: Custom Pass Definition
   ============================================ */

/* Simple dummy pass structure */
struct my_pass_data {
    struct opt_pass pass;
};

/* Pass execution function - does nothing */
static void my_pass_execute(void) {
    /* This is a dummy pass that does nothing */
    if (cfun) {
        /* Just to reference something to avoid unused variable warnings */
        (void)cfun->decl;
    }
}

/* Create the pass instance */
static struct opt_pass *make_my_pass(void) {
    struct my_pass_data *pass_data = XNEW(struct my_pass_data);
    
    pass_data->pass.type = GIMPLE_PASS;
    pass_data->pass.name = "my-dummy-pass";
    pass_data->pass.optinfo_flags = OPTGROUP_NONE;
    pass_data->pass.tv_id = TV_NONE;
    pass_data->pass.properties_required = 0;
    pass_data->pass.properties_provided = 0;
    pass_data->pass.properties_destroyed = 0;
    pass_data->pass.todo_flags_start = 0;
    pass_data->pass.todo_flags_finish = 0;
    pass_data->pass.execute = (opt_pass_execute)my_pass_execute;
    pass_data->pass.sub = NULL;
    pass_data->pass.next = NULL;
    pass_data->pass.static_pass_number = -1;
    
    return &pass_data->pass;
}

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS: GGC Root Table
   ============================================ */

/* Dummy structure for GGC roots */
struct dummy_ggc_struct {
    int dummy_field;
    tree dummy_tree;
};

/* GGC root table with one dummy entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_roots,  /* Base pointer */
        .nelt = 1,                         /* Number of elements */
        .stride = sizeof(struct ggc_root_tab), /* Stride */
        .cb = NULL,                        /* No callback */
        .pchw = NULL                       /* No PCH handling */
    },
    { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* ============================================
   PLUGIN_INFO: Plugin Information Structure
   ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO,\n"
            "and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
    
    const char *plugin_name = plugin_info->base_name;
    struct register_pass_info pass_info;
    
    printf("Coverage plugin initializing: %s\n", plugin_name);
    
    /* ============================================
       Trigger PLUGIN_PASS_MANAGER_SETUP event
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
       Trigger PLUGIN_INFO event
       ============================================ */
    
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed */
                      &my_plugin_info);
    
    /* ============================================
       Trigger PLUGIN_REGISTER_GGC_ROOTS event
       ============================================ */
    
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed */
                      (void *)dummy_ggc_roots);
    
    /* Also register for other events to ensure plugin is active */
    register_callback(plugin_name, PLUGIN_ALL_PASSES_START, NULL, NULL);
    register_callback(plugin_name, PLUGIN_ALL_PASSES_END, NULL, NULL);
    
    printf("Coverage plugin registered all target events\n");
    
    return 0;  /* Success */
}
