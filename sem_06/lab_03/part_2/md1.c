#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Serova Maria" );

char* md1_data = "Привет мир!";

extern char* md1_proc( void ) {
    return md1_data;
}

static char* md1_local( void ) {
    return md1_data;
}

extern char* md1_noexport( void ) {
    return md1_data;
}

EXPORT_SYMBOL( md1_data );
EXPORT_SYMBOL( md1_proc );

static int __init md_init( void ) {
    printk( "+ md1| start!\n" );
    printk("+ md1| string returned by md1_local() : %s\n", md1_local());
    printk("+ md1| string returned by md1_noexport() : %s\n", md1_noexport());

    return 0;
}

static void __exit md_exit( void ) {
    printk( "+ md1| unloaded!\n" );
}

module_init( md_init );
module_exit( md_exit );