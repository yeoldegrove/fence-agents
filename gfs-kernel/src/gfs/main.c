/******************************************************************************
*******************************************************************************
**
**  Copyright (C) Sistina Software, Inc.  1997-2003  All rights reserved.
**  Copyright (C) 2004 Red Hat, Inc.  All rights reserved.
**
**  This copyrighted material is made available to anyone wishing to use,
**  modify, copy, or redistribute it subject to the terms and conditions
**  of the GNU General Public License v.2.
**
*******************************************************************************
******************************************************************************/

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/spinlock.h>
#include <asm/semaphore.h>
#include <linux/completion.h>
#include <linux/buffer_head.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/init.h>

#include "gfs.h"
#include "diaper.h"
#include "ops_fstype.h"
#include "proc.h"

/**
 * init_gfs_fs - Register GFS as a filesystem
 *
 * Returns: 0 on success, error code on failure
 */

int __init
init_gfs_fs(void)
{
#ifdef GFS_PROFILE
	int p = FALSE;
#endif
	int error;

	gfs_random_number = xtime.tv_nsec;

#ifdef GFS_TRACE
	error = gfs_trace_init();
	if (error)
		return error;
#endif
#ifdef GFS_PROFILE
	error = gfs_profile_init();
	if (error)
		goto fail_debug;
	p = TRUE;
#endif

	gfs_init_lmh();

	error = gfs_sys_init();
	if (error)
		goto fail_debug;

	error = gfs_proc_init();
	if (error)
		goto fail_sys;

	error = gfs_diaper_init();
	if (error)
		goto fail_proc;

	error = -ENOMEM;

	gfs_glock_cachep = kmem_cache_create("gfs_glock", sizeof(struct gfs_glock),
					     0, 0,
					     NULL, NULL);
	if (!gfs_glock_cachep)
		goto fail_diaper;

	gfs_inode_cachep = kmem_cache_create("gfs_inode", sizeof(struct gfs_inode),
					     0, 0,
					     NULL, NULL);
	if (!gfs_inode_cachep)
		goto fail_diaper;

	gfs_bufdata_cachep = kmem_cache_create("gfs_bufdata", sizeof(struct gfs_bufdata),
					       0, 0,
					       NULL, NULL);
	if (!gfs_bufdata_cachep)
		goto fail_diaper;

	gfs_mhc_cachep = kmem_cache_create("gfs_meta_header_cache", sizeof(struct gfs_meta_header_cache),
					   0, 0,
					   NULL, NULL);
	if (!gfs_mhc_cachep)
		goto fail_diaper;

	error = register_filesystem(&gfs_fs_type);
	if (error)
		goto fail_diaper;

	printk("GFS %s (built %s %s) installed\n",
	       GFS_RELEASE_NAME, __DATE__, __TIME__);

	return 0;

 fail_diaper:
	if (gfs_mhc_cachep)
		kmem_cache_destroy(gfs_mhc_cachep);

	if (gfs_bufdata_cachep)
		kmem_cache_destroy(gfs_bufdata_cachep);

	if (gfs_inode_cachep)
		kmem_cache_destroy(gfs_inode_cachep);

	if (gfs_glock_cachep)
		kmem_cache_destroy(gfs_glock_cachep);

	gfs_diaper_uninit();

 fail_proc:
	gfs_proc_uninit();

 fail_sys:
	gfs_sys_uninit();

 fail_debug:
#ifdef GFS_PROFILE
	if (p)
		gfs_profile_uninit();
#endif
#ifdef GFS_TRACE
	gfs_trace_uninit();
#endif

	return error;
}

/**
 * exit_gfs_fs - Unregister the file system
 *
 */

void __exit
exit_gfs_fs(void)
{
	unregister_filesystem(&gfs_fs_type);

	kmem_cache_destroy(gfs_mhc_cachep);
	kmem_cache_destroy(gfs_bufdata_cachep);
	kmem_cache_destroy(gfs_inode_cachep);
	kmem_cache_destroy(gfs_glock_cachep);

	gfs_diaper_uninit();
	gfs_proc_uninit();
	gfs_sys_uninit();

#ifdef GFS_PROFILE
	gfs_profile_uninit();
#endif
#ifdef GFS_TRACE
	gfs_trace_uninit();
#endif
}

MODULE_DESCRIPTION("Global File System " GFS_RELEASE_NAME);
MODULE_AUTHOR("Red Hat, Inc.");
MODULE_LICENSE("GPL");

module_init(init_gfs_fs);
module_exit(exit_gfs_fs);

