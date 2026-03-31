/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */

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

/* ============================================
   PART 1: Data structures for the three events
   ============================================ */

/* 1. For PLUGIN_PASS_MANAGER_SETUP: Create a simple dummy pass */
static unsigned int dummy_pass_execute(void)
{
    /* This pass does nothing - just a placeholder */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always run this pass */
    return true;
}

static struct gimple_opt_pass dummy_pass = {
    {
        GIMPLE_PASS,
        "dummy-coverage-pass",      /* name */
        OPTGROUP_NONE,              /* optinfo_flags */
        dummy_pass_gate,            /* gate */
        dummy_pass_execute,         /* execute */
        NULL,                       /* sub */
        NULL,                       /* next */
        0,                          /* static_pass_number */
        TV_NONE,                    /* tv_id */
        0,                          /* properties_required */
        0,                          /* properties_provided */
        0,                          /* properties_destroyed */
        0,                          /* todo_flags_start */
        0                           /* todo_flags_finish */
    }
};

/* Register pass info structure */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,       /* Reference to our pass */
    .reference_pass_name = "cfg",   /* Insert after the CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* 2. For PLUGIN_INFO: Plugin information structure */
static struct plugin_info plugin_desc = {
    .version = "1.0",
    .help = "Coverage test plugin for plugin.cc uncovered lines\n"
            "This plugin triggers PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO,\n"
            "and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* 3. For PLUGIN_REGISTER_GGC_ROOTS: GGC root table */
/* Create a dummy variable that GCC's garbage collector will track */
static GTY(()) tree dummy_global_tree = NULL_TREE;

/* Define the GGC root table with our dummy variable */
static const struct ggc_root_tab dummy_roots[] = {
    {
        .base = (void *)&dummy_global_tree,
        .nelt = 1,
        .stride = sizeof(dummy_global_tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* Required NULL terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   PART 2: Plugin initialization function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    int ret;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin %s is not compatible with this GCC version\n", 
                plugin_name);
        return 1;
    }
    
    printf("Coverage plugin '%s' initializing...\n", plugin_name);
    
    /* ============================================
       Register callbacks for the three target events
       ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    ret = register_callback(plugin_name, 
                           PLUGIN_PASS_MANAGER_SETUP,
                           NULL,  /* No callback function needed */
                           &pass_info);
    
    if (ret) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
        return 1;
    }
    printf("  Registered PLUGIN_PASS_MANAGER_SETUP\n");
    
    /* 2. Register PLUGIN_INFO event */
    ret = register_callback(plugin_name,
                           PLUGIN_INFO,
                           NULL,  /* No callback function needed */
                           &plugin_desc);
    
    if (ret) {
        fprintf(stderr, "Failed to register PLUGIN_INFO\n");
        return 1;
    }
    printf("  Registered PLUGIN_INFO\n");
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    ret = register_callback(plugin_name,
                           PLUGIN_REGISTER_GGC_ROOTS,
                           NULL,  /* No callback function needed */
                           dummy_roots);
    
    if (ret) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
        return 1;
    }
    printf("  Registered PLUGIN_REGISTER_GGC_ROOTS\n");
    
    /* Optional: Register additional callback to verify plugin is active */
    ret = register_callback(plugin_name,
                           PLUGIN_FINISH,
                           NULL,
                           NULL);
    
    printf("Coverage plugin '%s' initialized successfully\n", plugin_name);
    return 0;  /* Success */
}
