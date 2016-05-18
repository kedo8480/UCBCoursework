//Kelsey Dowd
//SID: 102348752


#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR
#define ENOATTR ENODATA

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <limits.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#include "aes-crypt.h"
#endif

// Struct for paramaters to pass into private_data for FUSE
typedef struct {
	char* prepend_path;
	char* password;
} arg;

// Encryption Function
void en_crypt(char* total_path, char* pass) {
	// Open files and do_crypt for the encryption
	FILE* file_ptr_in = fopen(total_path, "rb");
	if(!file_ptr_in) {
		fprintf(stderr, "CANT OPEN IN FILE");
	}
	FILE* file_ptr_tmp = fopen("./tempfile.c", "wb+");
	if(!file_ptr_tmp) {
		fprintf(stderr, "CANT OPEN OUT FILE");
	}
	if(!do_crypt(file_ptr_in, file_ptr_tmp, 1, pass)) {
		fprintf(stderr, "ERR IN encrypt\n");
	}
	
	// Close files
	fclose(file_ptr_in);
	fclose(file_ptr_tmp);
	
	// Re-open files and do a copy using do_crypt so that it goes back to original file
	file_ptr_in = fopen(total_path, "wb+");
	if(!file_ptr_in) {
		fprintf(stderr, "CANT OPEN IN FILE");
	}
	file_ptr_tmp = fopen("./tempfile.c", "rb");
	if(!file_ptr_tmp) {
		fprintf(stderr, "CANT OPEN OUT FILE");
	}
	if(!do_crypt(file_ptr_tmp, file_ptr_in, -1, pass)) {
		fprintf(stderr, "ERR IN copy encrypt\n");
	}
	
	// Close files and delete tempfile
	fclose(file_ptr_tmp);
	unlink("./tempfile.c");
	fclose(file_ptr_in);
}

// Decryption Function
void de_crypt(char* total_path, char* pass) {
	
	//Open files and do_crypt for decryption
	FILE* file_ptr_in = fopen(total_path, "rb");
	if(!file_ptr_in) {
		fprintf(stderr, "CANT OPEN IN FILE");
	}
	FILE* file_ptr_tmp = fopen("./tempfile.c", "wb+");
	if(!file_ptr_tmp) {
		fprintf(stderr, "CANT OPEN OUT FILE");
	}
	if(!do_crypt(file_ptr_in, file_ptr_tmp, 0, pass)) {
		fprintf(stderr, "KEKEK************************************FJLKERJEKr\nERR IN decrypt\n");
	}
	
	// Close Files
	fclose(file_ptr_in);
	fclose(file_ptr_tmp);
	
	// Re-open files and do a copy using do_crypt so it goes back to original file
	file_ptr_in = fopen(total_path, "wb+");
	if(!file_ptr_in) {
		fprintf(stderr, "CANT OPEN IN FILE");
	}
	file_ptr_tmp = fopen("./tempfile.c", "rb");
	if(!file_ptr_tmp) {
		fprintf(stderr, "CANT OPEN OUT FILE");
	}
	if(!do_crypt(file_ptr_tmp, file_ptr_in, -1, pass)) {
		fprintf(stderr, "FAILLLLELKJALKS________________________________DFLKVDJLKASFJSDLKF\nERR IN copy decrypt\n");
	}
	
	// Close files and delete tempfile
	fclose(file_ptr_tmp);
	unlink("./tempfile.c");
	fclose(file_ptr_in);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	res = lstat(total_path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;

	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	res = access(total_path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	res = readlink(total_path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(total_path);
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

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(total_path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(total_path, mode);
	else
		res = mknod(total_path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;

	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	res = mkdir(total_path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;

	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	res = unlink(total_path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	res = rmdir(total_path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
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

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);


	res = chmod(total_path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	res = lchown(total_path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	res = truncate(total_path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(total_path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	res = open(total_path, fi->flags);

	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	// define vars
	int fd;
	int res;
	char* encval;
	
	// This re-routes the path to add the main path to the directory you want to mirror
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);
	
	// Pass in the encryption key
	char* pass = ((arg*)(fuse_get_context()->private_data))->password;

	// This gets the extended attributes for the file
	ssize_t valsize = getxattr(total_path, "user.pa5-encfs.encrypted", NULL, 0);
	
	// If there is no extended value for this file
	if (valsize < 0) {
		//Set xattr to False
		if(setxattr(total_path, "user.pa5-encfs.encrypted", "False", strlen("False"), 0)){
	    perror("setxattr error");
	    fprintf(stderr, "path  = %s\n",total_path);
	    fprintf(stderr, "name  = user.pa5-encfs.encrypted\n");
	    fprintf(stderr, "value = True\n");
	    fprintf(stderr, "size  = %zd\n", strlen("True"));
	    exit(EXIT_FAILURE);
		}
		
		// Read file normally
		(void) fi;
		fd = open(total_path, O_RDONLY);
		if (fd == -1)
			return -errno;
		
		res = pread(fd, buf, size, offset);
		if (res == -1) {
			res = -errno;
		}
		
		// Set the encval for comparison later
		encval = malloc(sizeof(char)*(strlen("False")+1));
		strcpy(encval, "False");
		
	}
	
	// Otherwise
	else {
		// Set encval to the value of the encrypted xattr
		encval = malloc(sizeof(char)*(valsize + 1));
		encval[valsize] = '\0';
		valsize = getxattr(total_path, "user.pa5-encfs.encrypted", encval, valsize);
		
	}
	
	// If the xattr is True, it is encrypted
	if(!strcmp(encval, "True")) {
		//decrypt
		de_crypt(total_path, pass);
		
		//read file
		(void) fi;
		fd = open(total_path, O_RDONLY);
		if (fd == -1)
			return -errno;
		res = pread(fd, buf, size, offset);
		if (res == -1) {
			res = -errno;
		}
		
		//re-encrypt
		en_crypt(total_path, pass);	
	}
	
	// If the xattr is False, it is not encrypted
	else if(!strcmp(encval, "False")) {
		// read file normally
		(void) fi;
		fd = open(total_path, O_RDONLY);
		if (fd == -1)
			return -errno;
		res = pread(fd, buf, size, offset);
		if (res == -1) {
			res = -errno;
		}
	}

	free(encval);
	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	// declare vars
	int fd;
	int res;
	char* encval;
	
	// append current path to make the total path
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);
	
	// pass in encryption key
	char* pass = ((arg*)(fuse_get_context()->private_data))->password;
	
	// This gets the extended attributes for the file
	ssize_t valsize = getxattr(total_path, "user.pa5-encfs.encrypted", NULL, 0);	
	
	// If there is no encrypted xattr
	if (valsize < 0) {
		// Set the xattr to False
		if(setxattr(total_path, "user.pa5-encfs.encrypted", "False", strlen("False"), 0)){
	    perror("setxattr error");
	    fprintf(stderr, "path  = %s\n",total_path);
	    fprintf(stderr, "name  = user.pa5-encfs.encrypted\n");
	    fprintf(stderr, "value = False\n");
	    fprintf(stderr, "size  = %zd\n", strlen("True"));
	    exit(EXIT_FAILURE);
		}
		
		//Write normally
		(void) fi;
		fd = open(total_path, O_WRONLY);
		if (fd == -1)
			return -errno;
		res = pwrite(fd, buf, size, offset);
		if (res == -1)
			res = -errno;
		
		// Set encval for later comparison
		encval = malloc(sizeof(char)*(strlen("False")+1));
		strcpy(encval, "False");
	}
	
	// Otherwise
	else {
		// Set encval to the true or false value of the xattr
		encval = malloc(sizeof(char)*(valsize + 1));
		encval[valsize] = '\0';
		valsize = getxattr(total_path, "user.pa5-encfs.encrypted", encval, valsize);
		fprintf(stderr, "encval: %s\n", encval);	
	}
	
	// If xattr = True, the file is encrypted
	if(!strcmp(encval, "True")) {
		
		//decrypt
		de_crypt(total_path, pass);
		
		//write normally
		(void) fi;
		fd = open(total_path, O_WRONLY);
		if (fd == -1)
			return -errno;
		res = pwrite(fd, buf, size, offset);
		if (res == -1)
			res = -errno;
			
		//re-encrypt
		en_crypt(total_path, pass);
		
	}
	
	// Otherwise if xattr = False, the file is not encrypted
	else if(!strcmp(encval, "False")) {
		//write normally
		(void) fi;
		fd = open(total_path, O_WRONLY);
		if (fd == -1)
			return -errno;
		res = pwrite(fd, buf, size, offset);
		if (res == -1)
			res = -errno;
	}

	close(fd);
	free(encval);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
	
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	res = statvfs(total_path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;
    
    // Append the current path to the path you want to mount the directory
    char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);
	
	// get the encryption key from the struct
	char* pass = ((arg*)(fuse_get_context()->private_data))->password;
	
	// create a file
	int res;
    res = creat(total_path, mode);
    if(res == -1)
	return -errno;
	
	// Encrypt the file
	en_crypt(total_path, pass);
	
	// Set the xattr to True
	if(setxattr(total_path, "user.pa5-encfs.encrypted", "True", strlen("True"), 0)){
	    perror("setxattr error");
	    fprintf(stderr, "path  = %s\n",total_path);
	    fprintf(stderr, "name  = user.pa5-encfs.encrypted\n");
	    fprintf(stderr, "value = True\n");
	    fprintf(stderr, "size  = %zd\n", strlen("True"));
	    exit(EXIT_FAILURE);
	}

    close(res);

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	   
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	(void) total_path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);

	(void) total_path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char* total_path = NULL;
	strcat(total_path, pre_path);
	strcat(total_path, path);
	
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);
	
	int res = lgetxattr(total_path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);
	
	int res = llistxattr(total_path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	char* pre_path = ((arg*)(fuse_get_context()->private_data))->prepend_path;
	char total_path[PATH_MAX];
	strcpy(total_path, pre_path);
	strcat(total_path, path);
	
	int res = lremovexattr(total_path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create         = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	
	// instantiate a struct
	arg* struct_ptr = malloc(sizeof(arg));
	struct_ptr->prepend_path = argv[argc-2];
	struct_ptr->password = argv[argc-3];
	
	// Re-index args
	argv[argc-3] = argv[argc-1];
	
	// Call Fuse
	return fuse_main(argc-2, argv, &xmp_oper, struct_ptr);
	free(struct_ptr);
}
