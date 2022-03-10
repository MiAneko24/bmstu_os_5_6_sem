#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Serova Maria" );

static int __init md_init( void ) {
    printk( "+ md2| start!\n" );
    printk( "+ md2| data string exported from md1 : %s\n", md1_data );
    printk( "+ md2| string returned md1_proc() is : %s\n", md1_proc());
    // printk("+ md2| string returned by md1_local() : %s\n", md1_local());
    printk("+ md2| string returned by md1_noexport() : %s\n", md1_noexport());
    return 0;
}

static void __exit md_exit( void ) {
    printk( "+ md2| module md2 unloaded!\n" );
}

module_init( md_init );
module_exit( md_exit );