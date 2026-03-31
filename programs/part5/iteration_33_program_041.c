/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   Data structures for the three target events
   ============================================ */

/* 1. For PLUGIN_PASS_MANAGER_SETUP: Custom pass definition */
static unsigned int dummy_pass_execute(void)
{
    /* This is a dummy pass that does nothing */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always enable this pass */
    return true;
}

static struct gimple_opt_pass dummy_pass = {
    {
        GIMPLE_PASS,
        "dummy-coverage-pass",           /* name */
        OPTGROUP_NONE,                   /* optinfo_flags */
        dummy_pass_gate,                 /* gate */
        dummy_pass_execute,              /* execute */
        NULL,                            /* sub */
        NULL,                            /* next */
        0,                               /* static_pass_number */
        TV_NONE,                         /* tv_id */
        PROP_gimple_any,                 /* properties_required */
        0,                               /* properties_provided */
        0,                               /* properties_destroyed */
        0,                               /* todo_flags_start */
        0                                /* todo_flags_finish */
    }
};

/* Register pass info structure */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,           /* Reference to our pass */
    .reference_pass_name = "cfg",       /* Insert after the CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER     /* Position: insert after reference pass */
};

/* 2. For PLUGIN_INFO: Plugin information structure */
static struct plugin_info plugin_info_data = {
    .version = "1.0-coverage",
    .help = "GCC plugin to trigger uncovered code coverage in plugin.cc\n"
            "This plugin registers three events to test the plugin infrastructure."
};

/* 3. For PLUGIN_REGISTER_GGC_ROOTS: GGC root table */
/* Create a dummy type to register */
typedef struct dummy_ggc_type {
    int id;
    tree node;
    struct dummy_ggc_type *next;
} dummy_ggc_type;

/* Static instance for GGC registration */
static dummy_ggc_type dummy_ggc_instance = {0, NULL_TREE, NULL};

/* GGC root table - must be NULL-terminated */
static const struct ggc_root_tab ggc_root_table[] = {
    {
        .base = (void *)&dummy_ggc_instance,
        .nelt = sizeof(dummy_ggc_type),
        .stride = sizeof(dummy_ggc_type),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* NULL terminator */
};

/* ============================================
   Plugin initialization function
   ============================================ */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin version mismatch\n");
        return 1;
    }
    
    printf("Coverage plugin '%s' initializing...\n", plugin_name);
    
    /* ============================================
       Register the three target events
       ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    printf("  Registering PLUGIN_PASS_MANAGER_SETUP...\n");
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback needed for registration */
                      &pass_info);
    
    /* 2. Register PLUGIN_INFO event */
    printf("  Registering PLUGIN_INFO...\n");
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback needed for registration */
                      &plugin_info_data);
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    printf("  Registering PLUGIN_REGISTER_GGC_ROOTS...\n");
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback needed for registration */
                      ggc_root_table);
    
    /* Additional callback to verify plugin is active during compilation */
    register_callback(plugin_name,
                      PLUGIN_ALL_PASSES_START,
                      NULL,  /* We could add a callback here if needed */
                      NULL);
    
    printf("Coverage plugin '%s' initialized successfully\n", plugin_name);
    return 0;
}
