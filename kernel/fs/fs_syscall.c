/* This file use for NCTU OSDI course */


// It's handel the file system APIs 
#include <inc/stdio.h>
#include <inc/syscall.h>
#include <inc/string.h>
#include <kernel/fs/fat/ff.h>
#include <fs.h>

extern struct fs_fd fd_table[FS_FD_MAX];
/*TODO: Lab7, file I/O system call interface.*/
/*Note: Here you need handle the file system call from user.
 *       1. When user open a new file, you can use the fd_new() to alloc a file object(struct fs_fd)
 *       2. When user R/W or seek the file, use the fd_get() to get file object.
 *       3. After get file object call file_* functions into VFS level
 *       4. Update the file objet's position or size when user R/W or seek the file.(You can find the useful marco in ff.h)
 *       5. Remember to use fd_put() to put file object back after user R/W, seek or close the file.
 *       6. Handle the error code, for example, if user call open() but no fd slot can be use, sys_open should return -STATUS_ENOSPC.
 *
 *  Call flow example:
 *        ┌──────────────┐
 *        │     open     │
 *        └──────────────┘
 *               ↓
 *        ╔══════════════╗
 *   ==>  ║   sys_open   ║  file I/O system call interface
 *        ╚══════════════╝
 *               ↓
 *        ┌──────────────┐
 *        │  file_open   │  VFS level file API
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │   fat_open   │  fat level file operator
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │    f_open    │  FAT File System Module
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │    diskio    │  low level file operator
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │     disk     │  simple ATA disk dirver
 *        └──────────────┘
 */

// Below is POSIX like I/O system call 
int sys_open(const char *file, int flags, int mode)
{
    //We dont care the mode.

    int fd = -1;
    int fd_i;

    struct fs_fd* fd_struct = NULL;
    for (fd_i = 0;fd_i < FS_FD_MAX; fd_i++)
    {
        fd_struct = fd_table + fd_i;
        if (strcmp(fd_struct->path, file) == 0 && fd_struct->flags == flags)
            break;
    }

    int fd = (fd_i == -1)? fd_new(): fd_i;

    fd_struct = (fd_i == -1)? fd_table + fd: fd_get(fd);

    if (fd == -1)
        return -STATUS_ENOSPC;

    int retVal = file_open(fd_struct, file, flags);
    if (retVal < 0) {
        sys_close(fd);
        return retVal;
    }
    // update size
    fd_struct->size = ((FIL*) fd_struct->data)->obj.objsize;
    return fd;
}

int sys_close(int fd)
{
    if (!(fd >= 0 && fd < FS_FD_MAX))
        return -STATUS_EBADF;

    int ret = 0;
    
    struct fs_fd* fd_struct = fd_table + fd;
    
   fd_put(fd_struct);
   if (fd_struct->ref_count == 1)
    {
        fd_struct->size = 0; 
        fd_struct->pos = 0;
        fd_struct->path[0] = '\0';
        ret = file_close(fd_struct);
    }
    return ret;
}
int sys_read(int fd, void *buf, size_t len)
{
    if (!(fd >= 0 && fd < FS_FD_MAX))
        return -STATUS_EBADF;
    if (buf == NULL || len < 0)
        return -STATUS_EINVAL;
    
    if (len == 0)
        return 0;
    
    
    struct fs_fd* fd_struct = fd_table + fd;
    int left = fd_struct->size - fd_struct->pos;
    return file_read(fd_struct, buf, (len > left)? left: len);

}
int sys_write(int fd, const void *buf, size_t len)
{
    if (!(fd >= 0 && fd < FS_FD_MAX))
        return -STATUS_EBADF;
    if (buf == NULL || len < 0)
        return -STATUS_EINVAL;
    
    if (len == 0)
        return 0;
    
    
    struct fs_fd* fd_struct = fd_table + fd;
    int ret = file_write(fd_struct, buf, len);
    fd_struct->size = ((FIL*) fd_struct->data)->obj.objsize;
    return ret;
}

/* Note: Check the whence parameter and calcuate the new offset value before do file_seek() */
off_t sys_lseek(int fd, off_t offset, int whence)
{
    if (!(fd >= 0 && fd < FS_FD_MAX))
        return -STATUS_EBADF;
    if (offset < 0 || whence < 0)
        return -STATUS_EINVAL;
    
    struct fs_fd* fd_struct = fd_table + fd;
    int new_offset = 0;
    if (whence == SEEK_SET)
        new_offset = offset;
    else if (whence == SEEK_CUR)
        new_offset = fd_struct->pos + offset;
    else if (whence == SEEK_END)
        new_offset = fd_struct->size + offset;

    if (new_offset < 0)
        return -STATUS_EINVAL;
    
    fd_struct->pos = new_offset;
    return file_lseek(fd_struct, new_offset);

}

int sys_unlink(const char *pathname)
{
    return file_unlink(pathname);
}


              

