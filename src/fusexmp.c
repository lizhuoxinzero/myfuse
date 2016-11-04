/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fusexmp.c `pkg-config fuse --cflags --libs` -o fusexmp
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "log/log.h"

char *mount_path;
char temppath[1024];
int mount_len;
static char* fmt_path(const char *path)
{
    strcpy(&temppath[mount_len], path);

    return (char *)&temppath;
}


static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;

	res = lstat(fmt_path(path), stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    return creat(fmt_path(path), mode);
}

static int xmp_opendir(const char *path, struct fuse_file_info *fi)
{
    LOG_DEBUG("start opendir");
    //sleep(10);
    LOG_DEBUG("start opendir");
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;


	dp = opendir(fmt_path(path));
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}



static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}
static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;

    LOG_DEBUG("start readdir");
    sleep(10);
	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
    LOG_DEBUG("finish readdir");
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}


static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.opendir	= xmp_opendir,
	.readdir	= xmp_readdir,
	.mkdir		= xmp_mkdir,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.create		= xmp_create,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
};

int main(int argc, char *argv[])
{
	umask(0);
    
    argc--;
    mount_path = argv[argc];
    mount_len = strlen(mount_path);
    memcpy(temppath, mount_path, mount_len);

	return fuse_main(argc, argv, &xmp_oper, NULL);
}
