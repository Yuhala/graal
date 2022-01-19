#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


typedef struct ms_ecall_graal_main_args_t {
	int ms_id;
	int ms_arg1;
} ms_ecall_graal_main_args_t;

typedef struct ms_ecall_graal_main_t {
	int ms_id;
} ms_ecall_graal_main_t;

typedef struct ms_ecall_set_environ_t {
	void** ms_env_ptr;
} ms_ecall_set_environ_t;

typedef struct ms_ecall_execute_job_t {
	pthread_t ms_pthread_self_id;
	unsigned long int ms_job_id;
} ms_ecall_execute_job_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_fsync_t {
	int ms_retval;
	int ms_fd;
} ms_ocall_fsync_t;

typedef struct ms_ocall_dup2_t {
	int ms_retval;
	int ms_oldfd;
	int ms_newfd;
} ms_ocall_dup2_t;

typedef struct ms_ocall_open_t {
	int ms_retval;
	const char* ms_path;
	int ms_oflag;
	int ms_arg;
} ms_ocall_open_t;

typedef struct ms_ocall_open64_t {
	int ms_retval;
	const char* ms_path;
	int ms_oflag;
	int ms_arg;
} ms_ocall_open64_t;

typedef struct ms_ocall_xclose_t {
	int ms_retval;
	int ms_fd;
} ms_ocall_xclose_t;

typedef struct ms_ocall_lseek_t {
	off_t ms_retval;
	int ms_fd;
	off_t ms_offset;
	int ms_whence;
} ms_ocall_lseek_t;

typedef struct ms_ocall_lseek64_t {
	off64_t ms_retval;
	int ms_fd;
	off64_t ms_offset;
	int ms_whence;
} ms_ocall_lseek64_t;

typedef struct ms_ocall_fflush_t {
	int ms_retval;
	SGX_FILE* ms_stream;
} ms_ocall_fflush_t;

typedef struct ms_ocall_pread_t {
	ssize_t ms_retval;
	int ms_fd;
	void* ms_buf;
	size_t ms_count;
	off_t ms_offset;
} ms_ocall_pread_t;

typedef struct ms_ocall_pread64_t {
	ssize_t ms_retval;
	int ms_fd;
	void* ms_buf;
	size_t ms_count;
	off64_t ms_offset;
} ms_ocall_pread64_t;

typedef struct ms_ocall_pwrite_t {
	ssize_t ms_retval;
	int ms_fd;
	const void* ms_buf;
	size_t ms_count;
	off_t ms_offset;
} ms_ocall_pwrite_t;

typedef struct ms_ocall_fopen_t {
	SGX_FILE ms_retval;
	const char* ms_filename;
	const char* ms_mode;
} ms_ocall_fopen_t;

typedef struct ms_ocall_fdopen_t {
	SGX_FILE ms_retval;
	int ms_fd;
	const char* ms_mode;
} ms_ocall_fdopen_t;

typedef struct ms_ocall_fclose_t {
	int ms_retval;
	SGX_FILE ms_stream;
} ms_ocall_fclose_t;

typedef struct ms_ocall_fwrite_t {
	size_t ms_retval;
	const void* ms_ptr;
	size_t ms_size;
	size_t ms_nmemb;
	SGX_FILE ms_stream;
} ms_ocall_fwrite_t;

typedef struct ms_ocall_read_t {
	ssize_t ms_retval;
	int ms_fd;
	void* ms_buf;
	size_t ms_count;
} ms_ocall_read_t;

typedef struct ms_ocall_write_t {
	ssize_t ms_retval;
	int ms_fd;
	const void* ms_buf;
	size_t ms_count;
} ms_ocall_write_t;

typedef struct ms_ocall_fscanf_t {
	int ms_retval;
	SGX_FILE ms_stream;
	const char* ms_format;
} ms_ocall_fscanf_t;

typedef struct ms_ocall_fprintf_t {
	int ms_retval;
	SGX_FILE ms_stream;
	const char* ms_str;
} ms_ocall_fprintf_t;

typedef struct ms_ocall_fgets_t {
	char* ms_str;
	int ms_n;
	SGX_FILE ms_stream;
} ms_ocall_fgets_t;

typedef struct ms_ocall_stderr_t {
	SGX_FILE ms_retval;
} ms_ocall_stderr_t;

typedef struct ms_ocall_puts_t {
	int ms_retval;
	const char* ms_str;
} ms_ocall_puts_t;

typedef struct ms_ocall_mkdir_t {
	int ms_retval;
	const char* ms_pathname;
	mode_t ms_mode;
} ms_ocall_mkdir_t;

typedef struct ms_ocall_truncate_t {
	int ms_retval;
	const char* ms_path;
	off_t ms_length;
} ms_ocall_truncate_t;

typedef struct ms_ocall_ftruncate64_t {
	int ms_retval;
	int ms_fd;
	off_t ms_length;
} ms_ocall_ftruncate64_t;

typedef struct ms_ocall_mmap64_t {
	void* ms_retval;
	void* ms_addr;
	size_t ms_len;
	int ms_prot;
	int ms_flags;
	int ms_fildes;
	off_t ms_off;
} ms_ocall_mmap64_t;

typedef struct ms_ocall_pwrite64_t {
	ssize_t ms_retval;
	int ms_fd;
	const void* ms_buf;
	size_t ms_nbyte;
	off_t ms_offset;
} ms_ocall_pwrite64_t;

typedef struct ms_ocall_fdatasync_t {
	int ms_retval;
	int ms_fd;
} ms_ocall_fdatasync_t;

typedef struct ms_ocall_rename_t {
	int ms_retval;
	const char* ms_oldpath;
	const char* ms_newpath;
} ms_ocall_rename_t;

typedef struct ms_ocall_unlink_t {
	int ms_retval;
	const char* ms_pathname;
} ms_ocall_unlink_t;

typedef struct ms_ocall_rmdir_t {
	int ms_retval;
	const char* ms_pathname;
} ms_ocall_rmdir_t;

typedef struct ms_ocall_times_t {
	clock_t ms_retval;
} ms_ocall_times_t;

typedef struct ms_ocall_chown_t {
	int ms_retval;
	const char* ms_pathname;
	uid_t ms_owner;
	gid_t ms_group;
} ms_ocall_chown_t;

typedef struct ms_ocall_fchown_t {
	int ms_retval;
	int ms_fd;
	uid_t ms_owner;
	gid_t ms_group;
} ms_ocall_fchown_t;

typedef struct ms_ocall_lchown_t {
	int ms_retval;
	const char* ms_pathname;
	uid_t ms_owner;
	gid_t ms_group;
} ms_ocall_lchown_t;

typedef struct ms_ocall_chmod_t {
	int ms_retval;
	const char* ms_pathname;
	mode_t ms_mode;
} ms_ocall_chmod_t;

typedef struct ms_ocall_fchmod_t {
	int ms_retval;
	int ms_fd;
	mode_t ms_mode;
} ms_ocall_fchmod_t;

typedef struct ms_ocall_lxstat64_t {
	int ms_retval;
	int ms_ver;
	const char* ms_path;
	struct stat* ms_stat_buf;
} ms_ocall_lxstat64_t;

typedef struct ms_ocall_fcntl_t {
	int ms_retval;
	int ocall_errno;
	int ms_fildes;
	int ms_cmd;
	int ms_arg;
} ms_ocall_fcntl_t;

typedef struct ms_ocall_ioctl_t {
	int ms_retval;
	int ms_fd;
	unsigned long int ms_request;
	int ms_arg;
} ms_ocall_ioctl_t;

typedef struct ms_ocall_xstat64_t {
	int ms_retval;
	int ms_ver;
	const char* ms_path;
	struct stat* ms_stat_buf;
} ms_ocall_xstat64_t;

typedef struct ms_ocall_fstat64_t {
	int ms_retval;
	int ms_fd;
	struct stat* ms_buf;
} ms_ocall_fstat64_t;

typedef struct ms_ocall_fxstat64_t {
	int ms_retval;
	int ms_ver;
	int ms_fildes;
	struct stat* ms_stat_buf;
} ms_ocall_fxstat64_t;

typedef struct ms_ocall_fxstat_t {
	int ms_retval;
	int ms_ver;
	int ms_fd;
	struct stat* ms_stat_buf;
} ms_ocall_fxstat_t;

typedef struct ms_ocall_lxstat_t {
	int ms_retval;
	int ms_ver;
	const char* ms_path;
	struct stat* ms_stat_buf;
} ms_ocall_lxstat_t;

typedef struct ms_ocall_xstat_t {
	int ms_retval;
	int ms_ver;
	const char* ms_path;
	struct stat* ms_stat_buf;
} ms_ocall_xstat_t;

typedef struct ms_ocall_pathconf_t {
	long int ms_retval;
	const char* ms_path;
	int ms_name;
} ms_ocall_pathconf_t;

typedef struct ms_ocall_readlink_t {
	ssize_t ms_retval;
	const char* ms_pathname;
	char* ms_buf;
	size_t ms_bufsiz;
} ms_ocall_readlink_t;

typedef struct ms_ocall_readdir64_r_t {
	int ms_retval;
	void* ms_dirp;
	void* ms_entry;
	struct dirent** ms_result;
} ms_ocall_readdir64_r_t;

typedef struct ms_ocall_opendir_t {
	void* ms_retval;
	const char* ms_name;
} ms_ocall_opendir_t;

typedef struct ms_ocall_chdir_t {
	int ms_retval;
	const char* ms_path;
} ms_ocall_chdir_t;

typedef struct ms_ocall_closedir_t {
	int ms_retval;
	void* ms_dirp;
} ms_ocall_closedir_t;

typedef struct ms_ocall_xmknod_t {
	int ms_retval;
	int ms_vers;
	const char* ms_path;
	mode_t ms_mode;
	dev_t* ms_dev;
} ms_ocall_xmknod_t;

typedef struct ms_ocall_symlink_t {
	int ms_retval;
	const char* ms_target;
	const char* ms_linkpath;
} ms_ocall_symlink_t;

typedef struct ms_ocall_deflateEnd_t {
	int ms_retval;
	z_streamp ms_stream;
} ms_ocall_deflateEnd_t;

typedef struct ms_ocall_deflateParams_t {
	int ms_retval;
	z_streamp ms_stream;
	int ms_level;
	int ms_strategy;
} ms_ocall_deflateParams_t;

typedef struct ms_ocall_deflate_t {
	int ms_retval;
	z_streamp ms_stream;
	int ms_flush;
} ms_ocall_deflate_t;

typedef struct ms_ocall_deflateInit2_t {
	int ms_retval;
	z_streamp ms_stream;
	int ms_level;
	int ms_method;
	int ms_windowBits;
	int ms_memLevel;
	int ms_strategy;
} ms_ocall_deflateInit2_t;

typedef struct ms_ocall_inflateReset_t {
	int ms_retval;
	z_streamp ms_stream;
} ms_ocall_inflateReset_t;

typedef struct ms_ocall_sendfile64_t {
	ssize_t ms_retval;
	int ms_out_fd;
	int ms_in_fd;
	off_t* ms_offset;
	size_t ms_count;
} ms_ocall_sendfile64_t;

typedef struct ms_ocall_adler32_t {
	ulong ms_retval;
	ulong ms_adler;
	const Bytef* ms_buf;
	size_t ms_len;
} ms_ocall_adler32_t;

typedef struct ms_ocall_getenv_t {
	int ms_retval;
	const char* ms_env;
	int ms_envlen;
	char* ms_ret_str;
	int ms_ret_len;
} ms_ocall_getenv_t;

typedef struct ms_ocall_fileno_t {
	int ms_retval;
	SGX_FILE* ms_stream;
} ms_ocall_fileno_t;

typedef struct ms_ocall_isatty_t {
	int ms_retval;
	int ms_fd;
} ms_ocall_isatty_t;

typedef struct ms_ocall_umask_t {
	mode_t ms_retval;
	mode_t ms_mask;
} ms_ocall_umask_t;

typedef struct ms_ocall_socket_t {
	int ms_retval;
	int ms_domain;
	int ms_type;
	int ms_protocol;
} ms_ocall_socket_t;

typedef struct ms_ocall_getsockname_t {
	int ms_retval;
	int ms_sockfd;
	struct sockaddr* ms_addr;
	socklen_t* ms_addrlen;
} ms_ocall_getsockname_t;

typedef struct ms_ocall_getaddrinfo_t {
	int ms_retval;
	const char* ms_node;
	const char* ms_service;
	const struct addrinfo* ms_hints;
	struct addrinfo** ms_res;
} ms_ocall_getaddrinfo_t;

typedef struct ms_ocall_getnameinfo_t {
	int ms_retval;
	const struct sockaddr* ms_addr;
	socklen_t ms_addrlen;
	char* ms_host;
	socklen_t ms_hostlen;
	char* ms_serv;
	socklen_t ms_servlen;
	int ms_flags;
} ms_ocall_getnameinfo_t;

typedef struct ms_ocall_freeaddrinfo_t {
	struct addrinfo* ms_res;
} ms_ocall_freeaddrinfo_t;

typedef struct ms_ocall_gethostname_t {
	int ms_retval;
	char* ms_name;
	size_t ms_namelen;
} ms_ocall_gethostname_t;

typedef struct ms_ocall_sethostname_t {
	int ms_retval;
	const char* ms_name;
	size_t ms_len;
} ms_ocall_sethostname_t;

typedef struct ms_ocall_gettimeofday_t {
	int ms_retval;
	void* ms_tv;
	int ms_tv_size;
} ms_ocall_gettimeofday_t;

typedef struct ms_ocall_clock_gettime_t {
	int ms_retval;
	clockid_t ms_clk_id;
	void* ms_tp;
	int ms_ts_size;
} ms_ocall_clock_gettime_t;

typedef struct ms_ocall_inet_pton_t {
	int ms_retval;
	int ms_af;
	const char* ms_src;
	void* ms_dst;
} ms_ocall_inet_pton_t;

typedef struct ms_ocall_getpid_t {
	pid_t ms_retval;
} ms_ocall_getpid_t;

typedef struct ms_ocall_remove_t {
	int ms_retval;
	const char* ms_pathname;
} ms_ocall_remove_t;

typedef struct ms_ocall_shutdown_t {
	int ms_retval;
	int ms_sockfd;
	int ms_how;
} ms_ocall_shutdown_t;

typedef struct ms_ocall_getsockopt_t {
	int ms_retval;
	int ocall_errno;
	int ms_socket;
	int ms_level;
	int ms_option_name;
	void* ms_option_value;
	socklen_t* ms_option_len;
} ms_ocall_getsockopt_t;

typedef struct ms_ocall_setsockopt_t {
	int ms_retval;
	int ocall_errno;
	int ms_socket;
	int ms_level;
	int ms_option_name;
	const void* ms_option_value;
	socklen_t ms_option_len;
} ms_ocall_setsockopt_t;

typedef struct ms_ocall_socketpair_t {
	int ms_retval;
	int ms_domain;
	int ms_type;
	int ms_protocol;
	int* ms_sv;
} ms_ocall_socketpair_t;

typedef struct ms_ocall_bind_t {
	int ms_retval;
	int ocall_errno;
	int ms_socket;
	const void* ms_address;
	socklen_t ms_address_len;
} ms_ocall_bind_t;

typedef struct ms_ocall_epoll_wait_t {
	int ms_retval;
	int ms_epfd;
	struct epoll_event* ms_events;
	int ms_maxevents;
	int ms_timeout;
} ms_ocall_epoll_wait_t;

typedef struct ms_ocall_epoll_ctl_t {
	int ms_retval;
	int ms_epfd;
	int ms_op;
	int ms_fd;
	struct epoll_event* ms_event;
} ms_ocall_epoll_ctl_t;

typedef struct ms_ocall_readv_t {
	ssize_t ms_retval;
	int ms_fd;
	const struct iovec* ms_iov;
	int ms_iovcnt;
} ms_ocall_readv_t;

typedef struct ms_ocall_writev_t {
	ssize_t ms_retval;
	int ms_fd;
	const struct iovec* ms_iov;
	int ms_iovcnt;
} ms_ocall_writev_t;

typedef struct ms_ocall_pipe_t {
	int ms_retval;
	int* ms_pipefd;
} ms_ocall_pipe_t;

typedef struct ms_ocall_connect_t {
	int ms_retval;
	int ms_sockfd;
	const void* ms_addr;
	socklen_t ms_addrlen;
} ms_ocall_connect_t;

typedef struct ms_ocall_listen_t {
	int ms_retval;
	int ocall_errno;
	int ms_socket;
	int ms_backlog;
} ms_ocall_listen_t;

typedef struct ms_ocall_accept_t {
	int ms_retval;
	int ocall_errno;
	int ms_socket;
	struct sockaddr* ms_address;
	socklen_t* ms_address_len;
} ms_ocall_accept_t;

typedef struct ms_ocall_poll_t {
	int ms_retval;
	int ocall_errno;
	struct pollfd* ms_fds;
	nfds_t ms_nfds;
	int ms_timeout;
} ms_ocall_poll_t;

typedef struct ms_ocall_epoll_create_t {
	int ms_retval;
	int ms_size;
} ms_ocall_epoll_create_t;

typedef struct ms_ocall_recv_t {
	ssize_t ms_retval;
	int ms_sockfd;
	void* ms_buf;
	size_t ms_len;
	int ms_flags;
} ms_ocall_recv_t;

typedef struct ms_ocall_send_t {
	ssize_t ms_retval;
	int ms_sockfd;
	const void* ms_buf;
	size_t ms_len;
	int ms_flags;
} ms_ocall_send_t;

typedef struct ms_ocall_dlsym_t {
	void* ms_handle;
	const char* ms_symbol;
	void* ms_res;
} ms_ocall_dlsym_t;

typedef struct ms_ocall_dlopen_t {
	void* ms_retval;
	const char* ms_symbol;
	int ms_flag;
} ms_ocall_dlopen_t;

typedef struct ms_ocall_sysconf_t {
	long int ms_retval;
	int ms_name;
} ms_ocall_sysconf_t;

typedef struct ms_ocall_getuid_t {
	int ms_retval;
} ms_ocall_getuid_t;

typedef struct ms_ocall_getcwd_t {
	char* ms_buf;
	size_t ms_len;
} ms_ocall_getcwd_t;

typedef struct ms_ocall_getpwuid_t {
	uid_t ms_uid;
	struct passwd* ms_ret;
} ms_ocall_getpwuid_t;

typedef struct ms_ocall_exit_t {
	int ms_stat;
} ms_ocall_exit_t;

typedef struct ms_ocall_getrlimit_t {
	int ms_retval;
	int ms_res;
	struct rlimit* ms_rlim;
} ms_ocall_getrlimit_t;

typedef struct ms_ocall_setrlimit_t {
	int ms_retval;
	int ms_resource;
	struct rlimit* ms_rlim;
} ms_ocall_setrlimit_t;

typedef struct ms_ocall_uname_t {
	int ms_retval;
	struct utsname* ms_buf;
} ms_ocall_uname_t;

typedef struct ms_ocall_sleep_t {
	unsigned int ms_retval;
	unsigned int ms_secs;
} ms_ocall_sleep_t;

typedef struct ms_ocall_realpath_t {
	const char* ms_path;
	char* ms_res_path;
} ms_ocall_realpath_t;

typedef struct ms_ocall_xpg_strerror_r_t {
	int ms_errnum;
	char* ms_buf;
	size_t ms_buflen;
} ms_ocall_xpg_strerror_r_t;

typedef struct ms_ocall_signal_t {
	__sighandler_t ms_retval;
	int ms_signum;
	__sighandler_t ms_handler;
} ms_ocall_signal_t;

typedef struct ms_ocall_get_cpuid_max_t {
	unsigned int ms_retval;
	unsigned int ms_ext;
	unsigned int* ms_sig;
} ms_ocall_get_cpuid_max_t;

typedef struct ms_ocall_get_cpuid_count_t {
	int ms_retval;
	unsigned int ms_leaf;
	unsigned int ms_subleaf;
	unsigned int* ms_eax;
	unsigned int* ms_ebx;
	unsigned int* ms_ecx;
	unsigned int* ms_edx;
} ms_ocall_get_cpuid_count_t;

typedef struct ms_ocall_dladdr_t {
	int ms_retval;
	const void* ms_addr;
	void* ms_info;
} ms_ocall_dladdr_t;

typedef struct ms_ocall_kill_t {
	int ms_retval;
	int ms_pid;
	int ms_sig;
} ms_ocall_kill_t;

typedef struct ms_ocall_getgrgid_r_t {
	int ms_retval;
	unsigned int ms_gid;
	void* ms_grp;
	char* ms_buf;
	size_t ms_buflen;
	void** ms_result;
} ms_ocall_getgrgid_r_t;

typedef struct ms_ocall_getpwuid_r_t {
	int ms_retval;
	unsigned int ms_uid;
	void* ms_pwd;
	char* ms_buf;
	size_t ms_buflen;
	void** ms_result;
} ms_ocall_getpwuid_r_t;

typedef struct ms_ocall_deflateReset_t {
	int ms_retval;
	Z_STREAMP ms_stream;
} ms_ocall_deflateReset_t;

typedef struct ms_ocall_pthread_attr_init_t {
	int ms_retval;
} ms_ocall_pthread_attr_init_t;

typedef struct ms_ocall_pthread_create_t {
	int ms_retval;
	pthread_t* ms_new_thread;
	unsigned long int ms_job_id;
	sgx_enclave_id_t ms_eid;
} ms_ocall_pthread_create_t;

typedef struct ms_ocall_pthread_self_t {
	pthread_t ms_retval;
} ms_ocall_pthread_self_t;

typedef struct ms_ocall_pthread_join_t {
	int ms_retval;
	pthread_t ms_pt;
	void** ms_res;
} ms_ocall_pthread_join_t;

typedef struct ms_ocall_pthread_attr_getguardsize_t {
	int ms_retval;
	size_t* ms_guardsize;
} ms_ocall_pthread_attr_getguardsize_t;

typedef struct ms_ocall_pthread_attr_getguardsize__bypass_t {
	int ms_retval;
	void* ms_attr;
	size_t ms_attr_len;
	size_t* ms_guardsize;
} ms_ocall_pthread_attr_getguardsize__bypass_t;

typedef struct ms_ocall_pthread_attr_destroy_t {
	int ms_retval;
} ms_ocall_pthread_attr_destroy_t;

typedef struct ms_ocall_pthread_condattr_setclock_t {
	int ms_retval;
	void* ms_attr;
	clockid_t ms_clock_id;
	size_t ms_attr_len;
} ms_ocall_pthread_condattr_setclock_t;

typedef struct ms_ocall_pthread_attr_destroy__bypass_t {
	int ms_retval;
	void* ms_attr;
	size_t ms_attr_len;
} ms_ocall_pthread_attr_destroy__bypass_t;

typedef struct ms_ocall_pthread_attr_getstack_t {
	int ms_retval;
	void** ms_stk_addr;
	size_t* ms_stack_size;
} ms_ocall_pthread_attr_getstack_t;

typedef struct ms_ocall_pthread_attr_getstack__bypass_t {
	int ms_retval;
	void* ms_attr;
	size_t ms_attr_len;
	void** ms_stk_addr;
	size_t ms_len;
	size_t* ms_stack_size;
} ms_ocall_pthread_attr_getstack__bypass_t;

typedef struct ms_ocall_pthread_getattr_np_t {
	int ms_retval;
	pthread_t ms_tid;
} ms_ocall_pthread_getattr_np_t;

typedef struct ms_ocall_pthread_getattr_np__bypass_t {
	int ms_retval;
	pthread_t ms_tid;
	void* ms_attr;
	size_t ms_len;
} ms_ocall_pthread_getattr_np__bypass_t;

typedef struct ms_sgx_oc_cpuidex_t {
	int* ms_cpuinfo;
	int ms_leaf;
	int ms_subleaf;
} ms_sgx_oc_cpuidex_t;

typedef struct ms_sgx_thread_wait_untrusted_event_ocall_t {
	int ms_retval;
	const void* ms_self;
} ms_sgx_thread_wait_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_set_untrusted_event_ocall_t {
	int ms_retval;
	const void* ms_waiter;
} ms_sgx_thread_set_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_setwait_untrusted_events_ocall_t {
	int ms_retval;
	const void* ms_waiter;
	const void* ms_self;
} ms_sgx_thread_setwait_untrusted_events_ocall_t;

typedef struct ms_sgx_thread_set_multiple_untrusted_events_ocall_t {
	int ms_retval;
	const void** ms_waiters;
	size_t ms_total;
} ms_sgx_thread_set_multiple_untrusted_events_ocall_t;

typedef struct ms_pthread_wait_timeout_ocall_t {
	int ms_retval;
	unsigned long long ms_waiter;
	unsigned long long ms_timeout;
} ms_pthread_wait_timeout_ocall_t;

typedef struct ms_pthread_create_ocall_t {
	int ms_retval;
	unsigned long long ms_self;
} ms_pthread_create_ocall_t;

typedef struct ms_pthread_wakeup_ocall_t {
	int ms_retval;
	unsigned long long ms_waiter;
} ms_pthread_wakeup_ocall_t;

static sgx_status_t SGX_CDECL sgx_ecall_graal_main_args(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_graal_main_args_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_graal_main_args_t* ms = SGX_CAST(ms_ecall_graal_main_args_t*, pms);
	sgx_status_t status = SGX_SUCCESS;



	ecall_graal_main_args(ms->ms_id, ms->ms_arg1);


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_graal_main(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_graal_main_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_graal_main_t* ms = SGX_CAST(ms_ecall_graal_main_t*, pms);
	sgx_status_t status = SGX_SUCCESS;



	ecall_graal_main(ms->ms_id);


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_set_environ(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_set_environ_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_set_environ_t* ms = SGX_CAST(ms_ecall_set_environ_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	void** _tmp_env_ptr = ms->ms_env_ptr;



	ecall_set_environ(_tmp_env_ptr);


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_execute_job(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_execute_job_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_execute_job_t* ms = SGX_CAST(ms_ecall_execute_job_t*, pms);
	sgx_status_t status = SGX_SUCCESS;



	ecall_execute_job(ms->ms_pthread_self_id, ms->ms_job_id);


	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[4];
} g_ecall_table = {
	4,
	{
		{(void*)(uintptr_t)sgx_ecall_graal_main_args, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_graal_main, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_set_environ, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_execute_job, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[137][4];
} g_dyn_entry_table = {
	137,
	{
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 1, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
		{0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_string(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));
	ocalloc_size -= sizeof(ms_ocall_print_string_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fsync(int* retval, int fd)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fsync_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fsync_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fsync_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fsync_t));
	ocalloc_size -= sizeof(ms_ocall_fsync_t);

	ms->ms_fd = fd;
	status = sgx_ocall(1, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_dup2(int* retval, int oldfd, int newfd)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_dup2_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_dup2_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_dup2_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_dup2_t));
	ocalloc_size -= sizeof(ms_ocall_dup2_t);

	ms->ms_oldfd = oldfd;
	ms->ms_newfd = newfd;
	status = sgx_ocall(2, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_open(int* retval, const char* path, int oflag, int arg)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;

	ms_ocall_open_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_open_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(path, _len_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_open_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_open_t));
	ocalloc_size -= sizeof(ms_ocall_open_t);

	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	ms->ms_oflag = oflag;
	ms->ms_arg = arg;
	status = sgx_ocall(3, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_open64(int* retval, const char* path, int oflag, int arg)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;

	ms_ocall_open64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_open64_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(path, _len_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_open64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_open64_t));
	ocalloc_size -= sizeof(ms_ocall_open64_t);

	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	ms->ms_oflag = oflag;
	ms->ms_arg = arg;
	status = sgx_ocall(4, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_xclose(int* retval, int fd)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_xclose_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_xclose_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_xclose_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_xclose_t));
	ocalloc_size -= sizeof(ms_ocall_xclose_t);

	ms->ms_fd = fd;
	status = sgx_ocall(5, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_lseek(off_t* retval, int fd, off_t offset, int whence)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_lseek_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_lseek_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_lseek_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_lseek_t));
	ocalloc_size -= sizeof(ms_ocall_lseek_t);

	ms->ms_fd = fd;
	ms->ms_offset = offset;
	ms->ms_whence = whence;
	status = sgx_ocall(6, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_lseek64(off64_t* retval, int fd, off64_t offset, int whence)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_lseek64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_lseek64_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_lseek64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_lseek64_t));
	ocalloc_size -= sizeof(ms_ocall_lseek64_t);

	ms->ms_fd = fd;
	ms->ms_offset = offset;
	ms->ms_whence = whence;
	status = sgx_ocall(7, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fflush(int* retval, SGX_FILE* stream)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fflush_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fflush_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fflush_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fflush_t));
	ocalloc_size -= sizeof(ms_ocall_fflush_t);

	ms->ms_stream = stream;
	status = sgx_ocall(8, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pread(ssize_t* retval, int fd, void* buf, size_t count, off_t offset)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = count;

	ms_ocall_pread_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pread_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pread_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pread_t));
	ocalloc_size -= sizeof(ms_ocall_pread_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (void*)__tmp;
		__tmp_buf = __tmp;
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_count = count;
	ms->ms_offset = offset;
	status = sgx_ocall(9, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pread64(ssize_t* retval, int fd, void* buf, size_t count, off64_t offset)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = count;

	ms_ocall_pread64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pread64_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pread64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pread64_t));
	ocalloc_size -= sizeof(ms_ocall_pread64_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (void*)__tmp;
		__tmp_buf = __tmp;
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_count = count;
	ms->ms_offset = offset;
	status = sgx_ocall(10, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pwrite(ssize_t* retval, int fd, const void* buf, size_t count, off_t offset)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = count;

	ms_ocall_pwrite_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pwrite_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pwrite_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pwrite_t));
	ocalloc_size -= sizeof(ms_ocall_pwrite_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_count = count;
	ms->ms_offset = offset;
	status = sgx_ocall(11, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fopen(SGX_FILE* retval, const char* filename, const char* mode)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_filename = filename ? strlen(filename) + 1 : 0;
	size_t _len_mode = mode ? strlen(mode) + 1 : 0;

	ms_ocall_fopen_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fopen_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(filename, _len_filename);
	CHECK_ENCLAVE_POINTER(mode, _len_mode);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (filename != NULL) ? _len_filename : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (mode != NULL) ? _len_mode : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fopen_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fopen_t));
	ocalloc_size -= sizeof(ms_ocall_fopen_t);

	if (filename != NULL) {
		ms->ms_filename = (const char*)__tmp;
		if (_len_filename % sizeof(*filename) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, filename, _len_filename)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_filename);
		ocalloc_size -= _len_filename;
	} else {
		ms->ms_filename = NULL;
	}
	
	if (mode != NULL) {
		ms->ms_mode = (const char*)__tmp;
		if (_len_mode % sizeof(*mode) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, mode, _len_mode)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_mode);
		ocalloc_size -= _len_mode;
	} else {
		ms->ms_mode = NULL;
	}
	
	status = sgx_ocall(12, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fdopen(SGX_FILE* retval, int fd, const char* mode)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_mode = mode ? strlen(mode) + 1 : 0;

	ms_ocall_fdopen_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fdopen_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(mode, _len_mode);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (mode != NULL) ? _len_mode : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fdopen_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fdopen_t));
	ocalloc_size -= sizeof(ms_ocall_fdopen_t);

	ms->ms_fd = fd;
	if (mode != NULL) {
		ms->ms_mode = (const char*)__tmp;
		if (_len_mode % sizeof(*mode) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, mode, _len_mode)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_mode);
		ocalloc_size -= _len_mode;
	} else {
		ms->ms_mode = NULL;
	}
	
	status = sgx_ocall(13, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fclose(int* retval, SGX_FILE stream)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fclose_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fclose_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fclose_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fclose_t));
	ocalloc_size -= sizeof(ms_ocall_fclose_t);

	ms->ms_stream = stream;
	status = sgx_ocall(14, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fwrite(size_t* retval, const void* ptr, size_t size, size_t nmemb, SGX_FILE stream)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ptr = nmemb * size;

	ms_ocall_fwrite_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fwrite_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(ptr, _len_ptr);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ptr != NULL) ? _len_ptr : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fwrite_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fwrite_t));
	ocalloc_size -= sizeof(ms_ocall_fwrite_t);

	if (ptr != NULL) {
		ms->ms_ptr = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, ptr, _len_ptr)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_ptr);
		ocalloc_size -= _len_ptr;
	} else {
		ms->ms_ptr = NULL;
	}
	
	ms->ms_size = size;
	ms->ms_nmemb = nmemb;
	ms->ms_stream = stream;
	status = sgx_ocall(15, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_read(ssize_t* retval, int fd, void* buf, size_t count)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = count;

	ms_ocall_read_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_read_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_read_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_read_t));
	ocalloc_size -= sizeof(ms_ocall_read_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (void*)__tmp;
		__tmp_buf = __tmp;
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_count = count;
	status = sgx_ocall(16, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_write(ssize_t* retval, int fd, const void* buf, size_t count)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = count;

	ms_ocall_write_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_write_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_write_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_write_t));
	ocalloc_size -= sizeof(ms_ocall_write_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_count = count;
	status = sgx_ocall(17, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fscanf(int* retval, SGX_FILE stream, const char* format)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_format = format ? strlen(format) + 1 : 0;

	ms_ocall_fscanf_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fscanf_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(format, _len_format);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (format != NULL) ? _len_format : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fscanf_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fscanf_t));
	ocalloc_size -= sizeof(ms_ocall_fscanf_t);

	ms->ms_stream = stream;
	if (format != NULL) {
		ms->ms_format = (const char*)__tmp;
		if (_len_format % sizeof(*format) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, format, _len_format)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_format);
		ocalloc_size -= _len_format;
	} else {
		ms->ms_format = NULL;
	}
	
	status = sgx_ocall(18, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fprintf(int* retval, SGX_FILE stream, const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_fprintf_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fprintf_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fprintf_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fprintf_t));
	ocalloc_size -= sizeof(ms_ocall_fprintf_t);

	ms->ms_stream = stream;
	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(19, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fgets(char* str, int n, SGX_FILE stream)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = n;

	ms_ocall_fgets_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fgets_t);
	void *__tmp = NULL;

	void *__tmp_str = NULL;

	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fgets_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fgets_t));
	ocalloc_size -= sizeof(ms_ocall_fgets_t);

	if (str != NULL) {
		ms->ms_str = (char*)__tmp;
		__tmp_str = __tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_str, 0, _len_str);
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	ms->ms_n = n;
	ms->ms_stream = stream;
	status = sgx_ocall(20, ms);

	if (status == SGX_SUCCESS) {
		if (str) {
			if (memcpy_s((void*)str, _len_str, __tmp_str, _len_str)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_stderr(SGX_FILE* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_stderr_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_stderr_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_stderr_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_stderr_t));
	ocalloc_size -= sizeof(ms_ocall_stderr_t);

	status = sgx_ocall(21, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_puts(int* retval, const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_puts_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_puts_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_puts_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_puts_t));
	ocalloc_size -= sizeof(ms_ocall_puts_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(22, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mkdir(int* retval, const char* pathname, mode_t mode)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_mkdir_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mkdir_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mkdir_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mkdir_t));
	ocalloc_size -= sizeof(ms_ocall_mkdir_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	ms->ms_mode = mode;
	status = sgx_ocall(23, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_truncate(int* retval, const char* path, off_t length)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;

	ms_ocall_truncate_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_truncate_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(path, _len_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_truncate_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_truncate_t));
	ocalloc_size -= sizeof(ms_ocall_truncate_t);

	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	ms->ms_length = length;
	status = sgx_ocall(24, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_ftruncate64(int* retval, int fd, off_t length)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_ftruncate64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_ftruncate64_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_ftruncate64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_ftruncate64_t));
	ocalloc_size -= sizeof(ms_ocall_ftruncate64_t);

	ms->ms_fd = fd;
	ms->ms_length = length;
	status = sgx_ocall(25, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_mmap64(void** retval, void* addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_mmap64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_mmap64_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_mmap64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_mmap64_t));
	ocalloc_size -= sizeof(ms_ocall_mmap64_t);

	ms->ms_addr = addr;
	ms->ms_len = len;
	ms->ms_prot = prot;
	ms->ms_flags = flags;
	ms->ms_fildes = fildes;
	ms->ms_off = off;
	status = sgx_ocall(26, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pwrite64(ssize_t* retval, int fd, const void* buf, size_t nbyte, off_t offset)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = nbyte;

	ms_ocall_pwrite64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pwrite64_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pwrite64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pwrite64_t));
	ocalloc_size -= sizeof(ms_ocall_pwrite64_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_nbyte = nbyte;
	ms->ms_offset = offset;
	status = sgx_ocall(27, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fdatasync(int* retval, int fd)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fdatasync_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fdatasync_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fdatasync_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fdatasync_t));
	ocalloc_size -= sizeof(ms_ocall_fdatasync_t);

	ms->ms_fd = fd;
	status = sgx_ocall(28, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_rename(int* retval, const char* oldpath, const char* newpath)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_oldpath = oldpath ? strlen(oldpath) + 1 : 0;
	size_t _len_newpath = newpath ? strlen(newpath) + 1 : 0;

	ms_ocall_rename_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_rename_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(oldpath, _len_oldpath);
	CHECK_ENCLAVE_POINTER(newpath, _len_newpath);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (oldpath != NULL) ? _len_oldpath : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (newpath != NULL) ? _len_newpath : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_rename_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_rename_t));
	ocalloc_size -= sizeof(ms_ocall_rename_t);

	if (oldpath != NULL) {
		ms->ms_oldpath = (const char*)__tmp;
		if (_len_oldpath % sizeof(*oldpath) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, oldpath, _len_oldpath)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_oldpath);
		ocalloc_size -= _len_oldpath;
	} else {
		ms->ms_oldpath = NULL;
	}
	
	if (newpath != NULL) {
		ms->ms_newpath = (const char*)__tmp;
		if (_len_newpath % sizeof(*newpath) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, newpath, _len_newpath)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_newpath);
		ocalloc_size -= _len_newpath;
	} else {
		ms->ms_newpath = NULL;
	}
	
	status = sgx_ocall(29, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_unlink(int* retval, const char* pathname)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_unlink_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_unlink_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_unlink_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_unlink_t));
	ocalloc_size -= sizeof(ms_ocall_unlink_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	status = sgx_ocall(30, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_rmdir(int* retval, const char* pathname)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_rmdir_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_rmdir_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_rmdir_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_rmdir_t));
	ocalloc_size -= sizeof(ms_ocall_rmdir_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	status = sgx_ocall(31, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_times(clock_t* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_times_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_times_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_times_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_times_t));
	ocalloc_size -= sizeof(ms_ocall_times_t);

	status = sgx_ocall(32, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_chown(int* retval, const char* pathname, uid_t owner, gid_t group)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_chown_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_chown_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_chown_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_chown_t));
	ocalloc_size -= sizeof(ms_ocall_chown_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	ms->ms_owner = owner;
	ms->ms_group = group;
	status = sgx_ocall(33, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fchown(int* retval, int fd, uid_t owner, gid_t group)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fchown_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fchown_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fchown_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fchown_t));
	ocalloc_size -= sizeof(ms_ocall_fchown_t);

	ms->ms_fd = fd;
	ms->ms_owner = owner;
	ms->ms_group = group;
	status = sgx_ocall(34, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_lchown(int* retval, const char* pathname, uid_t owner, gid_t group)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_lchown_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_lchown_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_lchown_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_lchown_t));
	ocalloc_size -= sizeof(ms_ocall_lchown_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	ms->ms_owner = owner;
	ms->ms_group = group;
	status = sgx_ocall(35, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_chmod(int* retval, const char* pathname, mode_t mode)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_chmod_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_chmod_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_chmod_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_chmod_t));
	ocalloc_size -= sizeof(ms_ocall_chmod_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	ms->ms_mode = mode;
	status = sgx_ocall(36, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fchmod(int* retval, int fd, mode_t mode)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fchmod_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fchmod_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fchmod_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fchmod_t));
	ocalloc_size -= sizeof(ms_ocall_fchmod_t);

	ms->ms_fd = fd;
	ms->ms_mode = mode;
	status = sgx_ocall(37, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_lxstat64(int* retval, int ver, const char* path, struct stat* stat_buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;
	size_t _len_stat_buf = sizeof(struct stat);

	ms_ocall_lxstat64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_lxstat64_t);
	void *__tmp = NULL;

	void *__tmp_stat_buf = NULL;

	CHECK_ENCLAVE_POINTER(path, _len_path);
	CHECK_ENCLAVE_POINTER(stat_buf, _len_stat_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stat_buf != NULL) ? _len_stat_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_lxstat64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_lxstat64_t));
	ocalloc_size -= sizeof(ms_ocall_lxstat64_t);

	ms->ms_ver = ver;
	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	if (stat_buf != NULL) {
		ms->ms_stat_buf = (struct stat*)__tmp;
		__tmp_stat_buf = __tmp;
		memset(__tmp_stat_buf, 0, _len_stat_buf);
		__tmp = (void *)((size_t)__tmp + _len_stat_buf);
		ocalloc_size -= _len_stat_buf;
	} else {
		ms->ms_stat_buf = NULL;
	}
	
	status = sgx_ocall(38, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stat_buf) {
			if (memcpy_s((void*)stat_buf, _len_stat_buf, __tmp_stat_buf, _len_stat_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fcntl(int* retval, int fildes, int cmd, int arg)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_fcntl_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fcntl_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fcntl_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fcntl_t));
	ocalloc_size -= sizeof(ms_ocall_fcntl_t);

	ms->ms_fildes = fildes;
	ms->ms_cmd = cmd;
	ms->ms_arg = arg;
	status = sgx_ocall(39, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_ioctl(int* retval, int fd, unsigned long int request, int arg)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_ioctl_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_ioctl_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_ioctl_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_ioctl_t));
	ocalloc_size -= sizeof(ms_ocall_ioctl_t);

	ms->ms_fd = fd;
	ms->ms_request = request;
	ms->ms_arg = arg;
	status = sgx_ocall(40, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_xstat64(int* retval, int ver, const char* path, struct stat* stat_buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;
	size_t _len_stat_buf = sizeof(struct stat);

	ms_ocall_xstat64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_xstat64_t);
	void *__tmp = NULL;

	void *__tmp_stat_buf = NULL;

	CHECK_ENCLAVE_POINTER(path, _len_path);
	CHECK_ENCLAVE_POINTER(stat_buf, _len_stat_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stat_buf != NULL) ? _len_stat_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_xstat64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_xstat64_t));
	ocalloc_size -= sizeof(ms_ocall_xstat64_t);

	ms->ms_ver = ver;
	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	if (stat_buf != NULL) {
		ms->ms_stat_buf = (struct stat*)__tmp;
		__tmp_stat_buf = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, stat_buf, _len_stat_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_stat_buf);
		ocalloc_size -= _len_stat_buf;
	} else {
		ms->ms_stat_buf = NULL;
	}
	
	status = sgx_ocall(41, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stat_buf) {
			if (memcpy_s((void*)stat_buf, _len_stat_buf, __tmp_stat_buf, _len_stat_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fstat64(int* retval, int fd, struct stat* buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = sizeof(struct stat);

	ms_ocall_fstat64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fstat64_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fstat64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fstat64_t));
	ocalloc_size -= sizeof(ms_ocall_fstat64_t);

	ms->ms_fd = fd;
	if (buf != NULL) {
		ms->ms_buf = (struct stat*)__tmp;
		__tmp_buf = __tmp;
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	status = sgx_ocall(42, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fxstat64(int* retval, int ver, int fildes, struct stat* stat_buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_stat_buf = sizeof(struct stat);

	ms_ocall_fxstat64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fxstat64_t);
	void *__tmp = NULL;

	void *__tmp_stat_buf = NULL;

	CHECK_ENCLAVE_POINTER(stat_buf, _len_stat_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stat_buf != NULL) ? _len_stat_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fxstat64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fxstat64_t));
	ocalloc_size -= sizeof(ms_ocall_fxstat64_t);

	ms->ms_ver = ver;
	ms->ms_fildes = fildes;
	if (stat_buf != NULL) {
		ms->ms_stat_buf = (struct stat*)__tmp;
		__tmp_stat_buf = __tmp;
		memset(__tmp_stat_buf, 0, _len_stat_buf);
		__tmp = (void *)((size_t)__tmp + _len_stat_buf);
		ocalloc_size -= _len_stat_buf;
	} else {
		ms->ms_stat_buf = NULL;
	}
	
	status = sgx_ocall(43, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stat_buf) {
			if (memcpy_s((void*)stat_buf, _len_stat_buf, __tmp_stat_buf, _len_stat_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fxstat(int* retval, int ver, int fd, struct stat* stat_buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_stat_buf = sizeof(struct stat);

	ms_ocall_fxstat_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fxstat_t);
	void *__tmp = NULL;

	void *__tmp_stat_buf = NULL;

	CHECK_ENCLAVE_POINTER(stat_buf, _len_stat_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stat_buf != NULL) ? _len_stat_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fxstat_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fxstat_t));
	ocalloc_size -= sizeof(ms_ocall_fxstat_t);

	ms->ms_ver = ver;
	ms->ms_fd = fd;
	if (stat_buf != NULL) {
		ms->ms_stat_buf = (struct stat*)__tmp;
		__tmp_stat_buf = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, stat_buf, _len_stat_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_stat_buf);
		ocalloc_size -= _len_stat_buf;
	} else {
		ms->ms_stat_buf = NULL;
	}
	
	status = sgx_ocall(44, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stat_buf) {
			if (memcpy_s((void*)stat_buf, _len_stat_buf, __tmp_stat_buf, _len_stat_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_lxstat(int* retval, int ver, const char* path, struct stat* stat_buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;
	size_t _len_stat_buf = sizeof(struct stat);

	ms_ocall_lxstat_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_lxstat_t);
	void *__tmp = NULL;

	void *__tmp_stat_buf = NULL;

	CHECK_ENCLAVE_POINTER(path, _len_path);
	CHECK_ENCLAVE_POINTER(stat_buf, _len_stat_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stat_buf != NULL) ? _len_stat_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_lxstat_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_lxstat_t));
	ocalloc_size -= sizeof(ms_ocall_lxstat_t);

	ms->ms_ver = ver;
	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	if (stat_buf != NULL) {
		ms->ms_stat_buf = (struct stat*)__tmp;
		__tmp_stat_buf = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, stat_buf, _len_stat_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_stat_buf);
		ocalloc_size -= _len_stat_buf;
	} else {
		ms->ms_stat_buf = NULL;
	}
	
	status = sgx_ocall(45, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stat_buf) {
			if (memcpy_s((void*)stat_buf, _len_stat_buf, __tmp_stat_buf, _len_stat_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_xstat(int* retval, int ver, const char* path, struct stat* stat_buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;
	size_t _len_stat_buf = sizeof(struct stat);

	ms_ocall_xstat_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_xstat_t);
	void *__tmp = NULL;

	void *__tmp_stat_buf = NULL;

	CHECK_ENCLAVE_POINTER(path, _len_path);
	CHECK_ENCLAVE_POINTER(stat_buf, _len_stat_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stat_buf != NULL) ? _len_stat_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_xstat_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_xstat_t));
	ocalloc_size -= sizeof(ms_ocall_xstat_t);

	ms->ms_ver = ver;
	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	if (stat_buf != NULL) {
		ms->ms_stat_buf = (struct stat*)__tmp;
		__tmp_stat_buf = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, stat_buf, _len_stat_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_stat_buf);
		ocalloc_size -= _len_stat_buf;
	} else {
		ms->ms_stat_buf = NULL;
	}
	
	status = sgx_ocall(46, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stat_buf) {
			if (memcpy_s((void*)stat_buf, _len_stat_buf, __tmp_stat_buf, _len_stat_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pathconf(long int* retval, const char* path, int name)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;

	ms_ocall_pathconf_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pathconf_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(path, _len_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pathconf_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pathconf_t));
	ocalloc_size -= sizeof(ms_ocall_pathconf_t);

	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	ms->ms_name = name;
	status = sgx_ocall(47, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_readlink(ssize_t* retval, const char* pathname, char* buf, size_t bufsiz)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;
	size_t _len_buf = bufsiz;

	ms_ocall_readlink_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_readlink_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);
	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_readlink_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_readlink_t));
	ocalloc_size -= sizeof(ms_ocall_readlink_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	if (buf != NULL) {
		ms->ms_buf = (char*)__tmp;
		__tmp_buf = __tmp;
		if (_len_buf % sizeof(*buf) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_bufsiz = bufsiz;
	status = sgx_ocall(48, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_readdir64_r(int* retval, void* dirp, void* entry, struct dirent** result)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_readdir64_r_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_readdir64_r_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_readdir64_r_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_readdir64_r_t));
	ocalloc_size -= sizeof(ms_ocall_readdir64_r_t);

	ms->ms_dirp = dirp;
	ms->ms_entry = entry;
	ms->ms_result = result;
	status = sgx_ocall(49, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_opendir(void** retval, const char* name)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_name = name ? strlen(name) + 1 : 0;

	ms_ocall_opendir_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_opendir_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(name, _len_name);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (name != NULL) ? _len_name : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_opendir_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_opendir_t));
	ocalloc_size -= sizeof(ms_ocall_opendir_t);

	if (name != NULL) {
		ms->ms_name = (const char*)__tmp;
		if (_len_name % sizeof(*name) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, name, _len_name)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_name);
		ocalloc_size -= _len_name;
	} else {
		ms->ms_name = NULL;
	}
	
	status = sgx_ocall(50, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_chdir(int* retval, const char* path)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;

	ms_ocall_chdir_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_chdir_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(path, _len_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_chdir_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_chdir_t));
	ocalloc_size -= sizeof(ms_ocall_chdir_t);

	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	status = sgx_ocall(51, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_closedir(int* retval, void* dirp)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_closedir_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_closedir_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_closedir_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_closedir_t));
	ocalloc_size -= sizeof(ms_ocall_closedir_t);

	ms->ms_dirp = dirp;
	status = sgx_ocall(52, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_xmknod(int* retval, int vers, const char* path, mode_t mode, dev_t* dev)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;

	ms_ocall_xmknod_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_xmknod_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(path, _len_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_xmknod_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_xmknod_t));
	ocalloc_size -= sizeof(ms_ocall_xmknod_t);

	ms->ms_vers = vers;
	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	ms->ms_mode = mode;
	ms->ms_dev = dev;
	status = sgx_ocall(53, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_symlink(int* retval, const char* target, const char* linkpath)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_target = target ? strlen(target) + 1 : 0;
	size_t _len_linkpath = linkpath ? strlen(linkpath) + 1 : 0;

	ms_ocall_symlink_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_symlink_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(target, _len_target);
	CHECK_ENCLAVE_POINTER(linkpath, _len_linkpath);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (target != NULL) ? _len_target : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (linkpath != NULL) ? _len_linkpath : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_symlink_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_symlink_t));
	ocalloc_size -= sizeof(ms_ocall_symlink_t);

	if (target != NULL) {
		ms->ms_target = (const char*)__tmp;
		if (_len_target % sizeof(*target) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, target, _len_target)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_target);
		ocalloc_size -= _len_target;
	} else {
		ms->ms_target = NULL;
	}
	
	if (linkpath != NULL) {
		ms->ms_linkpath = (const char*)__tmp;
		if (_len_linkpath % sizeof(*linkpath) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, linkpath, _len_linkpath)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_linkpath);
		ocalloc_size -= _len_linkpath;
	} else {
		ms->ms_linkpath = NULL;
	}
	
	status = sgx_ocall(54, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_deflateEnd(int* retval, z_streamp stream)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_deflateEnd_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_deflateEnd_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_deflateEnd_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_deflateEnd_t));
	ocalloc_size -= sizeof(ms_ocall_deflateEnd_t);

	ms->ms_stream = stream;
	status = sgx_ocall(55, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_deflateParams(int* retval, z_streamp stream, int level, int strategy)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_deflateParams_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_deflateParams_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_deflateParams_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_deflateParams_t));
	ocalloc_size -= sizeof(ms_ocall_deflateParams_t);

	ms->ms_stream = stream;
	ms->ms_level = level;
	ms->ms_strategy = strategy;
	status = sgx_ocall(56, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_deflate(int* retval, z_streamp stream, int flush)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_deflate_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_deflate_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_deflate_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_deflate_t));
	ocalloc_size -= sizeof(ms_ocall_deflate_t);

	ms->ms_stream = stream;
	ms->ms_flush = flush;
	status = sgx_ocall(57, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_deflateInit2(int* retval, z_streamp stream, int level, int method, int windowBits, int memLevel, int strategy)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_deflateInit2_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_deflateInit2_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_deflateInit2_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_deflateInit2_t));
	ocalloc_size -= sizeof(ms_ocall_deflateInit2_t);

	ms->ms_stream = stream;
	ms->ms_level = level;
	ms->ms_method = method;
	ms->ms_windowBits = windowBits;
	ms->ms_memLevel = memLevel;
	ms->ms_strategy = strategy;
	status = sgx_ocall(58, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_inflateReset(int* retval, z_streamp stream)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_inflateReset_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_inflateReset_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_inflateReset_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_inflateReset_t));
	ocalloc_size -= sizeof(ms_ocall_inflateReset_t);

	ms->ms_stream = stream;
	status = sgx_ocall(59, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_sendfile64(ssize_t* retval, int out_fd, int in_fd, off_t* offset, size_t count)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_sendfile64_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_sendfile64_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_sendfile64_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_sendfile64_t));
	ocalloc_size -= sizeof(ms_ocall_sendfile64_t);

	ms->ms_out_fd = out_fd;
	ms->ms_in_fd = in_fd;
	ms->ms_offset = offset;
	ms->ms_count = count;
	status = sgx_ocall(60, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_adler32(ulong* retval, ulong adler, const Bytef* buf, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = len;

	ms_ocall_adler32_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_adler32_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_adler32_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_adler32_t));
	ocalloc_size -= sizeof(ms_ocall_adler32_t);

	ms->ms_adler = adler;
	if (buf != NULL) {
		ms->ms_buf = (const Bytef*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(61, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getenv(int* retval, const char* env, int envlen, char* ret_str, int ret_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_env = envlen;
	size_t _len_ret_str = ret_len;

	ms_ocall_getenv_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getenv_t);
	void *__tmp = NULL;

	void *__tmp_ret_str = NULL;

	CHECK_ENCLAVE_POINTER(env, _len_env);
	CHECK_ENCLAVE_POINTER(ret_str, _len_ret_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (env != NULL) ? _len_env : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ret_str != NULL) ? _len_ret_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getenv_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getenv_t));
	ocalloc_size -= sizeof(ms_ocall_getenv_t);

	if (env != NULL) {
		ms->ms_env = (const char*)__tmp;
		if (_len_env % sizeof(*env) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, env, _len_env)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_env);
		ocalloc_size -= _len_env;
	} else {
		ms->ms_env = NULL;
	}
	
	ms->ms_envlen = envlen;
	if (ret_str != NULL) {
		ms->ms_ret_str = (char*)__tmp;
		__tmp_ret_str = __tmp;
		if (_len_ret_str % sizeof(*ret_str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_ret_str, 0, _len_ret_str);
		__tmp = (void *)((size_t)__tmp + _len_ret_str);
		ocalloc_size -= _len_ret_str;
	} else {
		ms->ms_ret_str = NULL;
	}
	
	ms->ms_ret_len = ret_len;
	status = sgx_ocall(62, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (ret_str) {
			if (memcpy_s((void*)ret_str, _len_ret_str, __tmp_ret_str, _len_ret_str)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_fileno(int* retval, SGX_FILE* stream)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_stream = sizeof(SGX_FILE);

	ms_ocall_fileno_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_fileno_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(stream, _len_stream);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stream != NULL) ? _len_stream : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_fileno_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_fileno_t));
	ocalloc_size -= sizeof(ms_ocall_fileno_t);

	if (stream != NULL) {
		ms->ms_stream = (SGX_FILE*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, stream, _len_stream)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_stream);
		ocalloc_size -= _len_stream;
	} else {
		ms->ms_stream = NULL;
	}
	
	status = sgx_ocall(63, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_isatty(int* retval, int fd)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_isatty_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_isatty_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_isatty_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_isatty_t));
	ocalloc_size -= sizeof(ms_ocall_isatty_t);

	ms->ms_fd = fd;
	status = sgx_ocall(64, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_umask(mode_t* retval, mode_t mask)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_umask_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_umask_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_umask_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_umask_t));
	ocalloc_size -= sizeof(ms_ocall_umask_t);

	ms->ms_mask = mask;
	status = sgx_ocall(65, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_socket(int* retval, int domain, int type, int protocol)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_socket_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_socket_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_socket_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_socket_t));
	ocalloc_size -= sizeof(ms_ocall_socket_t);

	ms->ms_domain = domain;
	ms->ms_type = type;
	ms->ms_protocol = protocol;
	status = sgx_ocall(66, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getsockname(int* retval, int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_getsockname_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getsockname_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getsockname_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getsockname_t));
	ocalloc_size -= sizeof(ms_ocall_getsockname_t);

	ms->ms_sockfd = sockfd;
	ms->ms_addr = addr;
	ms->ms_addrlen = addrlen;
	status = sgx_ocall(67, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getaddrinfo(int* retval, const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_node = node ? strlen(node) + 1 : 0;
	size_t _len_service = service ? strlen(service) + 1 : 0;
	size_t _len_hints = sizeof(struct addrinfo);
	size_t _len_res = sizeof(struct addrinfo*);

	ms_ocall_getaddrinfo_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getaddrinfo_t);
	void *__tmp = NULL;

	void *__tmp_res = NULL;

	CHECK_ENCLAVE_POINTER(node, _len_node);
	CHECK_ENCLAVE_POINTER(service, _len_service);
	CHECK_ENCLAVE_POINTER(hints, _len_hints);
	CHECK_ENCLAVE_POINTER(res, _len_res);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (node != NULL) ? _len_node : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (service != NULL) ? _len_service : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (hints != NULL) ? _len_hints : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (res != NULL) ? _len_res : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getaddrinfo_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getaddrinfo_t));
	ocalloc_size -= sizeof(ms_ocall_getaddrinfo_t);

	if (node != NULL) {
		ms->ms_node = (const char*)__tmp;
		if (_len_node % sizeof(*node) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, node, _len_node)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_node);
		ocalloc_size -= _len_node;
	} else {
		ms->ms_node = NULL;
	}
	
	if (service != NULL) {
		ms->ms_service = (const char*)__tmp;
		if (_len_service % sizeof(*service) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, service, _len_service)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_service);
		ocalloc_size -= _len_service;
	} else {
		ms->ms_service = NULL;
	}
	
	if (hints != NULL) {
		ms->ms_hints = (const struct addrinfo*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, hints, _len_hints)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_hints);
		ocalloc_size -= _len_hints;
	} else {
		ms->ms_hints = NULL;
	}
	
	if (res != NULL) {
		ms->ms_res = (struct addrinfo**)__tmp;
		__tmp_res = __tmp;
		if (_len_res % sizeof(*res) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_res, 0, _len_res);
		__tmp = (void *)((size_t)__tmp + _len_res);
		ocalloc_size -= _len_res;
	} else {
		ms->ms_res = NULL;
	}
	
	status = sgx_ocall(68, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (res) {
			if (memcpy_s((void*)res, _len_res, __tmp_res, _len_res)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getnameinfo(int* retval, const struct sockaddr* addr, socklen_t addrlen, char* host, socklen_t hostlen, char* serv, socklen_t servlen, int flags)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_addr = addrlen;
	size_t _len_host = hostlen;
	size_t _len_serv = servlen;

	ms_ocall_getnameinfo_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getnameinfo_t);
	void *__tmp = NULL;

	void *__tmp_host = NULL;
	void *__tmp_serv = NULL;

	CHECK_ENCLAVE_POINTER(addr, _len_addr);
	CHECK_ENCLAVE_POINTER(host, _len_host);
	CHECK_ENCLAVE_POINTER(serv, _len_serv);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (host != NULL) ? _len_host : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (serv != NULL) ? _len_serv : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getnameinfo_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getnameinfo_t));
	ocalloc_size -= sizeof(ms_ocall_getnameinfo_t);

	if (addr != NULL) {
		ms->ms_addr = (const struct sockaddr*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, addr, _len_addr)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_addr);
		ocalloc_size -= _len_addr;
	} else {
		ms->ms_addr = NULL;
	}
	
	ms->ms_addrlen = addrlen;
	if (host != NULL) {
		ms->ms_host = (char*)__tmp;
		__tmp_host = __tmp;
		if (_len_host % sizeof(*host) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, host, _len_host)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_host);
		ocalloc_size -= _len_host;
	} else {
		ms->ms_host = NULL;
	}
	
	ms->ms_hostlen = hostlen;
	if (serv != NULL) {
		ms->ms_serv = (char*)__tmp;
		__tmp_serv = __tmp;
		if (_len_serv % sizeof(*serv) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, serv, _len_serv)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_serv);
		ocalloc_size -= _len_serv;
	} else {
		ms->ms_serv = NULL;
	}
	
	ms->ms_servlen = servlen;
	ms->ms_flags = flags;
	status = sgx_ocall(69, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (host) {
			if (memcpy_s((void*)host, _len_host, __tmp_host, _len_host)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (serv) {
			if (memcpy_s((void*)serv, _len_serv, __tmp_serv, _len_serv)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_freeaddrinfo(struct addrinfo* res)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_freeaddrinfo_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_freeaddrinfo_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_freeaddrinfo_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_freeaddrinfo_t));
	ocalloc_size -= sizeof(ms_ocall_freeaddrinfo_t);

	ms->ms_res = res;
	status = sgx_ocall(70, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_gethostname(int* retval, char* name, size_t namelen)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_name = namelen * sizeof(char);

	ms_ocall_gethostname_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_gethostname_t);
	void *__tmp = NULL;

	void *__tmp_name = NULL;

	CHECK_ENCLAVE_POINTER(name, _len_name);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (name != NULL) ? _len_name : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_gethostname_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_gethostname_t));
	ocalloc_size -= sizeof(ms_ocall_gethostname_t);

	if (name != NULL) {
		ms->ms_name = (char*)__tmp;
		__tmp_name = __tmp;
		if (_len_name % sizeof(*name) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_name, 0, _len_name);
		__tmp = (void *)((size_t)__tmp + _len_name);
		ocalloc_size -= _len_name;
	} else {
		ms->ms_name = NULL;
	}
	
	ms->ms_namelen = namelen;
	status = sgx_ocall(71, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (name) {
			if (memcpy_s((void*)name, _len_name, __tmp_name, _len_name)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_sethostname(int* retval, const char* name, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_name = len;

	ms_ocall_sethostname_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_sethostname_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(name, _len_name);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (name != NULL) ? _len_name : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_sethostname_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_sethostname_t));
	ocalloc_size -= sizeof(ms_ocall_sethostname_t);

	if (name != NULL) {
		ms->ms_name = (const char*)__tmp;
		if (_len_name % sizeof(*name) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, name, _len_name)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_name);
		ocalloc_size -= _len_name;
	} else {
		ms->ms_name = NULL;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(72, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_gettimeofday(int* retval, void* tv, int tv_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_tv = tv_size;

	ms_ocall_gettimeofday_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_gettimeofday_t);
	void *__tmp = NULL;

	void *__tmp_tv = NULL;

	CHECK_ENCLAVE_POINTER(tv, _len_tv);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (tv != NULL) ? _len_tv : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_gettimeofday_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_gettimeofday_t));
	ocalloc_size -= sizeof(ms_ocall_gettimeofday_t);

	if (tv != NULL) {
		ms->ms_tv = (void*)__tmp;
		__tmp_tv = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, tv, _len_tv)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_tv);
		ocalloc_size -= _len_tv;
	} else {
		ms->ms_tv = NULL;
	}
	
	ms->ms_tv_size = tv_size;
	status = sgx_ocall(73, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (tv) {
			if (memcpy_s((void*)tv, _len_tv, __tmp_tv, _len_tv)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_clock_gettime(int* retval, clockid_t clk_id, void* tp, int ts_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_tp = ts_size;

	ms_ocall_clock_gettime_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_clock_gettime_t);
	void *__tmp = NULL;

	void *__tmp_tp = NULL;

	CHECK_ENCLAVE_POINTER(tp, _len_tp);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (tp != NULL) ? _len_tp : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_clock_gettime_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_clock_gettime_t));
	ocalloc_size -= sizeof(ms_ocall_clock_gettime_t);

	ms->ms_clk_id = clk_id;
	if (tp != NULL) {
		ms->ms_tp = (void*)__tmp;
		__tmp_tp = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, tp, _len_tp)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_tp);
		ocalloc_size -= _len_tp;
	} else {
		ms->ms_tp = NULL;
	}
	
	ms->ms_ts_size = ts_size;
	status = sgx_ocall(74, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (tp) {
			if (memcpy_s((void*)tp, _len_tp, __tmp_tp, _len_tp)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_inet_pton(int* retval, int af, const char* src, void* dst)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_src = src ? strlen(src) + 1 : 0;
	size_t _len_dst = 4;

	ms_ocall_inet_pton_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_inet_pton_t);
	void *__tmp = NULL;

	void *__tmp_dst = NULL;

	CHECK_ENCLAVE_POINTER(src, _len_src);
	CHECK_ENCLAVE_POINTER(dst, _len_dst);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (src != NULL) ? _len_src : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (dst != NULL) ? _len_dst : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_inet_pton_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_inet_pton_t));
	ocalloc_size -= sizeof(ms_ocall_inet_pton_t);

	ms->ms_af = af;
	if (src != NULL) {
		ms->ms_src = (const char*)__tmp;
		if (_len_src % sizeof(*src) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, src, _len_src)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_src);
		ocalloc_size -= _len_src;
	} else {
		ms->ms_src = NULL;
	}
	
	if (dst != NULL) {
		ms->ms_dst = (void*)__tmp;
		__tmp_dst = __tmp;
		memset(__tmp_dst, 0, _len_dst);
		__tmp = (void *)((size_t)__tmp + _len_dst);
		ocalloc_size -= _len_dst;
	} else {
		ms->ms_dst = NULL;
	}
	
	status = sgx_ocall(75, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (dst) {
			if (memcpy_s((void*)dst, _len_dst, __tmp_dst, _len_dst)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getpid(pid_t* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_getpid_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getpid_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getpid_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getpid_t));
	ocalloc_size -= sizeof(ms_ocall_getpid_t);

	status = sgx_ocall(76, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_remove(int* retval, const char* pathname)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pathname = pathname ? strlen(pathname) + 1 : 0;

	ms_ocall_remove_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_remove_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(pathname, _len_pathname);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pathname != NULL) ? _len_pathname : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_remove_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_remove_t));
	ocalloc_size -= sizeof(ms_ocall_remove_t);

	if (pathname != NULL) {
		ms->ms_pathname = (const char*)__tmp;
		if (_len_pathname % sizeof(*pathname) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, pathname, _len_pathname)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_pathname);
		ocalloc_size -= _len_pathname;
	} else {
		ms->ms_pathname = NULL;
	}
	
	status = sgx_ocall(77, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_shutdown(int* retval, int sockfd, int how)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_shutdown_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_shutdown_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_shutdown_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_shutdown_t));
	ocalloc_size -= sizeof(ms_ocall_shutdown_t);

	ms->ms_sockfd = sockfd;
	ms->ms_how = how;
	status = sgx_ocall(78, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getsockopt(int* retval, int socket, int level, int option_name, void* option_value, socklen_t* option_len)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_getsockopt_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getsockopt_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getsockopt_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getsockopt_t));
	ocalloc_size -= sizeof(ms_ocall_getsockopt_t);

	ms->ms_socket = socket;
	ms->ms_level = level;
	ms->ms_option_name = option_name;
	ms->ms_option_value = option_value;
	ms->ms_option_len = option_len;
	status = sgx_ocall(79, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_setsockopt(int* retval, int socket, int level, int option_name, const void* option_value, socklen_t option_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_option_value = option_len;

	ms_ocall_setsockopt_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_setsockopt_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(option_value, _len_option_value);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (option_value != NULL) ? _len_option_value : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_setsockopt_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_setsockopt_t));
	ocalloc_size -= sizeof(ms_ocall_setsockopt_t);

	ms->ms_socket = socket;
	ms->ms_level = level;
	ms->ms_option_name = option_name;
	if (option_value != NULL) {
		ms->ms_option_value = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, option_value, _len_option_value)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_option_value);
		ocalloc_size -= _len_option_value;
	} else {
		ms->ms_option_value = NULL;
	}
	
	ms->ms_option_len = option_len;
	status = sgx_ocall(80, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_socketpair(int* retval, int domain, int type, int protocol, int* sv)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_sv = 2 * sizeof(int);

	ms_ocall_socketpair_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_socketpair_t);
	void *__tmp = NULL;

	void *__tmp_sv = NULL;

	CHECK_ENCLAVE_POINTER(sv, _len_sv);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (sv != NULL) ? _len_sv : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_socketpair_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_socketpair_t));
	ocalloc_size -= sizeof(ms_ocall_socketpair_t);

	ms->ms_domain = domain;
	ms->ms_type = type;
	ms->ms_protocol = protocol;
	if (sv != NULL) {
		ms->ms_sv = (int*)__tmp;
		__tmp_sv = __tmp;
		if (_len_sv % sizeof(*sv) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_sv, 0, _len_sv);
		__tmp = (void *)((size_t)__tmp + _len_sv);
		ocalloc_size -= _len_sv;
	} else {
		ms->ms_sv = NULL;
	}
	
	status = sgx_ocall(81, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (sv) {
			if (memcpy_s((void*)sv, _len_sv, __tmp_sv, _len_sv)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_bind(int* retval, int socket, const void* address, socklen_t address_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_address = address_len;

	ms_ocall_bind_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_bind_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(address, _len_address);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (address != NULL) ? _len_address : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_bind_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_bind_t));
	ocalloc_size -= sizeof(ms_ocall_bind_t);

	ms->ms_socket = socket;
	if (address != NULL) {
		ms->ms_address = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, address, _len_address)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_address);
		ocalloc_size -= _len_address;
	} else {
		ms->ms_address = NULL;
	}
	
	ms->ms_address_len = address_len;
	status = sgx_ocall(82, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_epoll_wait(int* retval, int epfd, struct epoll_event* events, int maxevents, int timeout)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_epoll_wait_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_epoll_wait_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_epoll_wait_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_epoll_wait_t));
	ocalloc_size -= sizeof(ms_ocall_epoll_wait_t);

	ms->ms_epfd = epfd;
	ms->ms_events = events;
	ms->ms_maxevents = maxevents;
	ms->ms_timeout = timeout;
	status = sgx_ocall(83, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_epoll_ctl(int* retval, int epfd, int op, int fd, struct epoll_event* event)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_epoll_ctl_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_epoll_ctl_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_epoll_ctl_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_epoll_ctl_t));
	ocalloc_size -= sizeof(ms_ocall_epoll_ctl_t);

	ms->ms_epfd = epfd;
	ms->ms_op = op;
	ms->ms_fd = fd;
	ms->ms_event = event;
	status = sgx_ocall(84, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_readv(ssize_t* retval, int fd, const struct iovec* iov, int iovcnt)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_readv_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_readv_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_readv_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_readv_t));
	ocalloc_size -= sizeof(ms_ocall_readv_t);

	ms->ms_fd = fd;
	ms->ms_iov = iov;
	ms->ms_iovcnt = iovcnt;
	status = sgx_ocall(85, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_writev(ssize_t* retval, int fd, const struct iovec* iov, int iovcnt)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_iov = sizeof(struct iovec);

	ms_ocall_writev_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_writev_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(iov, _len_iov);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (iov != NULL) ? _len_iov : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_writev_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_writev_t));
	ocalloc_size -= sizeof(ms_ocall_writev_t);

	ms->ms_fd = fd;
	if (iov != NULL) {
		ms->ms_iov = (const struct iovec*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, iov, _len_iov)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_iov);
		ocalloc_size -= _len_iov;
	} else {
		ms->ms_iov = NULL;
	}
	
	ms->ms_iovcnt = iovcnt;
	status = sgx_ocall(86, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pipe(int* retval, int* pipefd)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_pipefd = 2 * sizeof(int);

	ms_ocall_pipe_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pipe_t);
	void *__tmp = NULL;

	void *__tmp_pipefd = NULL;

	CHECK_ENCLAVE_POINTER(pipefd, _len_pipefd);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (pipefd != NULL) ? _len_pipefd : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pipe_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pipe_t));
	ocalloc_size -= sizeof(ms_ocall_pipe_t);

	if (pipefd != NULL) {
		ms->ms_pipefd = (int*)__tmp;
		__tmp_pipefd = __tmp;
		if (_len_pipefd % sizeof(*pipefd) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_pipefd, 0, _len_pipefd);
		__tmp = (void *)((size_t)__tmp + _len_pipefd);
		ocalloc_size -= _len_pipefd;
	} else {
		ms->ms_pipefd = NULL;
	}
	
	status = sgx_ocall(87, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (pipefd) {
			if (memcpy_s((void*)pipefd, _len_pipefd, __tmp_pipefd, _len_pipefd)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_connect(int* retval, int sockfd, const void* addr, socklen_t addrlen)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_connect_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_connect_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_connect_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_connect_t));
	ocalloc_size -= sizeof(ms_ocall_connect_t);

	ms->ms_sockfd = sockfd;
	ms->ms_addr = addr;
	ms->ms_addrlen = addrlen;
	status = sgx_ocall(88, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_listen(int* retval, int socket, int backlog)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_listen_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_listen_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_listen_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_listen_t));
	ocalloc_size -= sizeof(ms_ocall_listen_t);

	ms->ms_socket = socket;
	ms->ms_backlog = backlog;
	status = sgx_ocall(89, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_accept(int* retval, int socket, struct sockaddr* address, socklen_t* address_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_address_len = sizeof(socklen_t);

	ms_ocall_accept_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_accept_t);
	void *__tmp = NULL;

	void *__tmp_address_len = NULL;

	CHECK_ENCLAVE_POINTER(address_len, _len_address_len);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (address_len != NULL) ? _len_address_len : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_accept_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_accept_t));
	ocalloc_size -= sizeof(ms_ocall_accept_t);

	ms->ms_socket = socket;
	ms->ms_address = address;
	if (address_len != NULL) {
		ms->ms_address_len = (socklen_t*)__tmp;
		__tmp_address_len = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, address_len, _len_address_len)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_address_len);
		ocalloc_size -= _len_address_len;
	} else {
		ms->ms_address_len = NULL;
	}
	
	status = sgx_ocall(90, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (address_len) {
			if (memcpy_s((void*)address_len, _len_address_len, __tmp_address_len, _len_address_len)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_poll(int* retval, struct pollfd* fds, nfds_t nfds, int timeout)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_fds = nfds * sizeof(struct pollfd);

	ms_ocall_poll_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_poll_t);
	void *__tmp = NULL;

	void *__tmp_fds = NULL;

	CHECK_ENCLAVE_POINTER(fds, _len_fds);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (fds != NULL) ? _len_fds : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_poll_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_poll_t));
	ocalloc_size -= sizeof(ms_ocall_poll_t);

	if (fds != NULL) {
		ms->ms_fds = (struct pollfd*)__tmp;
		__tmp_fds = __tmp;
		if (memcpy_s(__tmp, ocalloc_size, fds, _len_fds)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_fds);
		ocalloc_size -= _len_fds;
	} else {
		ms->ms_fds = NULL;
	}
	
	ms->ms_nfds = nfds;
	ms->ms_timeout = timeout;
	status = sgx_ocall(91, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (fds) {
			if (memcpy_s((void*)fds, _len_fds, __tmp_fds, _len_fds)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		errno = ms->ocall_errno;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_epoll_create(int* retval, int size)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_epoll_create_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_epoll_create_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_epoll_create_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_epoll_create_t));
	ocalloc_size -= sizeof(ms_ocall_epoll_create_t);

	ms->ms_size = size;
	status = sgx_ocall(92, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_recv(ssize_t* retval, int sockfd, void* buf, size_t len, int flags)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = len;

	ms_ocall_recv_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_recv_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_recv_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_recv_t));
	ocalloc_size -= sizeof(ms_ocall_recv_t);

	ms->ms_sockfd = sockfd;
	if (buf != NULL) {
		ms->ms_buf = (void*)__tmp;
		__tmp_buf = __tmp;
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_len = len;
	ms->ms_flags = flags;
	status = sgx_ocall(93, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_send(ssize_t* retval, int sockfd, const void* buf, size_t len, int flags)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = len;

	ms_ocall_send_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_send_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_send_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_send_t));
	ocalloc_size -= sizeof(ms_ocall_send_t);

	ms->ms_sockfd = sockfd;
	if (buf != NULL) {
		ms->ms_buf = (const void*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_len = len;
	ms->ms_flags = flags;
	status = sgx_ocall(94, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_dlsym(void* handle, const char* symbol, void* res)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_symbol = symbol ? strlen(symbol) + 1 : 0;

	ms_ocall_dlsym_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_dlsym_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(symbol, _len_symbol);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (symbol != NULL) ? _len_symbol : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_dlsym_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_dlsym_t));
	ocalloc_size -= sizeof(ms_ocall_dlsym_t);

	ms->ms_handle = handle;
	if (symbol != NULL) {
		ms->ms_symbol = (const char*)__tmp;
		if (_len_symbol % sizeof(*symbol) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, symbol, _len_symbol)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_symbol);
		ocalloc_size -= _len_symbol;
	} else {
		ms->ms_symbol = NULL;
	}
	
	ms->ms_res = res;
	status = sgx_ocall(95, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_dlopen(void** retval, const char* symbol, int flag)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_symbol = symbol ? strlen(symbol) + 1 : 0;

	ms_ocall_dlopen_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_dlopen_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(symbol, _len_symbol);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (symbol != NULL) ? _len_symbol : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_dlopen_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_dlopen_t));
	ocalloc_size -= sizeof(ms_ocall_dlopen_t);

	if (symbol != NULL) {
		ms->ms_symbol = (const char*)__tmp;
		if (_len_symbol % sizeof(*symbol) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, symbol, _len_symbol)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_symbol);
		ocalloc_size -= _len_symbol;
	} else {
		ms->ms_symbol = NULL;
	}
	
	ms->ms_flag = flag;
	status = sgx_ocall(96, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_sysconf(long int* retval, int name)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_sysconf_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_sysconf_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_sysconf_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_sysconf_t));
	ocalloc_size -= sizeof(ms_ocall_sysconf_t);

	ms->ms_name = name;
	status = sgx_ocall(97, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getuid(int* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_getuid_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getuid_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getuid_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getuid_t));
	ocalloc_size -= sizeof(ms_ocall_getuid_t);

	status = sgx_ocall(98, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getcwd(char* buf, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = len * 1;

	ms_ocall_getcwd_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getcwd_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getcwd_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getcwd_t));
	ocalloc_size -= sizeof(ms_ocall_getcwd_t);

	if (buf != NULL) {
		ms->ms_buf = (char*)__tmp;
		__tmp_buf = __tmp;
		if (_len_buf % sizeof(*buf) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(99, ms);

	if (status == SGX_SUCCESS) {
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getpwuid(uid_t uid, struct passwd* ret)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ret = sizeof(struct passwd);

	ms_ocall_getpwuid_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getpwuid_t);
	void *__tmp = NULL;

	void *__tmp_ret = NULL;

	CHECK_ENCLAVE_POINTER(ret, _len_ret);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ret != NULL) ? _len_ret : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getpwuid_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getpwuid_t));
	ocalloc_size -= sizeof(ms_ocall_getpwuid_t);

	ms->ms_uid = uid;
	if (ret != NULL) {
		ms->ms_ret = (struct passwd*)__tmp;
		__tmp_ret = __tmp;
		memset(__tmp_ret, 0, _len_ret);
		__tmp = (void *)((size_t)__tmp + _len_ret);
		ocalloc_size -= _len_ret;
	} else {
		ms->ms_ret = NULL;
	}
	
	status = sgx_ocall(100, ms);

	if (status == SGX_SUCCESS) {
		if (ret) {
			if (memcpy_s((void*)ret, _len_ret, __tmp_ret, _len_ret)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_exit(int stat)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_exit_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_exit_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_exit_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_exit_t));
	ocalloc_size -= sizeof(ms_ocall_exit_t);

	ms->ms_stat = stat;
	status = sgx_ocall(101, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getrlimit(int* retval, int res, struct rlimit* rlim)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_rlim = sizeof(struct rlimit);

	ms_ocall_getrlimit_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getrlimit_t);
	void *__tmp = NULL;

	void *__tmp_rlim = NULL;

	CHECK_ENCLAVE_POINTER(rlim, _len_rlim);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (rlim != NULL) ? _len_rlim : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getrlimit_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getrlimit_t));
	ocalloc_size -= sizeof(ms_ocall_getrlimit_t);

	ms->ms_res = res;
	if (rlim != NULL) {
		ms->ms_rlim = (struct rlimit*)__tmp;
		__tmp_rlim = __tmp;
		memset(__tmp_rlim, 0, _len_rlim);
		__tmp = (void *)((size_t)__tmp + _len_rlim);
		ocalloc_size -= _len_rlim;
	} else {
		ms->ms_rlim = NULL;
	}
	
	status = sgx_ocall(102, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (rlim) {
			if (memcpy_s((void*)rlim, _len_rlim, __tmp_rlim, _len_rlim)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_setrlimit(int* retval, int resource, struct rlimit* rlim)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_rlim = sizeof(struct rlimit);

	ms_ocall_setrlimit_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_setrlimit_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(rlim, _len_rlim);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (rlim != NULL) ? _len_rlim : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_setrlimit_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_setrlimit_t));
	ocalloc_size -= sizeof(ms_ocall_setrlimit_t);

	ms->ms_resource = resource;
	if (rlim != NULL) {
		ms->ms_rlim = (struct rlimit*)__tmp;
		if (memcpy_s(__tmp, ocalloc_size, rlim, _len_rlim)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_rlim);
		ocalloc_size -= _len_rlim;
	} else {
		ms->ms_rlim = NULL;
	}
	
	status = sgx_ocall(103, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_uname(int* retval, struct utsname* buf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = sizeof(struct utsname);

	ms_ocall_uname_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_uname_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_uname_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_uname_t));
	ocalloc_size -= sizeof(ms_ocall_uname_t);

	if (buf != NULL) {
		ms->ms_buf = (struct utsname*)__tmp;
		__tmp_buf = __tmp;
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	status = sgx_ocall(104, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_sleep(unsigned int* retval, unsigned int secs)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_sleep_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_sleep_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_sleep_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_sleep_t));
	ocalloc_size -= sizeof(ms_ocall_sleep_t);

	ms->ms_secs = secs;
	status = sgx_ocall(105, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_realpath(const char* path, char* res_path)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_path = path ? strlen(path) + 1 : 0;
	size_t _len_res_path = sizeof(char);

	ms_ocall_realpath_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_realpath_t);
	void *__tmp = NULL;

	void *__tmp_res_path = NULL;

	CHECK_ENCLAVE_POINTER(path, _len_path);
	CHECK_ENCLAVE_POINTER(res_path, _len_res_path);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (path != NULL) ? _len_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (res_path != NULL) ? _len_res_path : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_realpath_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_realpath_t));
	ocalloc_size -= sizeof(ms_ocall_realpath_t);

	if (path != NULL) {
		ms->ms_path = (const char*)__tmp;
		if (_len_path % sizeof(*path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, path, _len_path)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_path);
		ocalloc_size -= _len_path;
	} else {
		ms->ms_path = NULL;
	}
	
	if (res_path != NULL) {
		ms->ms_res_path = (char*)__tmp;
		__tmp_res_path = __tmp;
		if (_len_res_path % sizeof(*res_path) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_res_path, 0, _len_res_path);
		__tmp = (void *)((size_t)__tmp + _len_res_path);
		ocalloc_size -= _len_res_path;
	} else {
		ms->ms_res_path = NULL;
	}
	
	status = sgx_ocall(106, ms);

	if (status == SGX_SUCCESS) {
		if (res_path) {
			if (memcpy_s((void*)res_path, _len_res_path, __tmp_res_path, _len_res_path)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_xpg_strerror_r(int errnum, char* buf, size_t buflen)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = 1 * buflen;

	ms_ocall_xpg_strerror_r_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_xpg_strerror_r_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_xpg_strerror_r_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_xpg_strerror_r_t));
	ocalloc_size -= sizeof(ms_ocall_xpg_strerror_r_t);

	ms->ms_errnum = errnum;
	if (buf != NULL) {
		ms->ms_buf = (char*)__tmp;
		__tmp_buf = __tmp;
		if (_len_buf % sizeof(*buf) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_buflen = buflen;
	status = sgx_ocall(107, ms);

	if (status == SGX_SUCCESS) {
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_signal(__sighandler_t* retval, int signum, __sighandler_t handler)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_signal_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_signal_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_signal_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_signal_t));
	ocalloc_size -= sizeof(ms_ocall_signal_t);

	ms->ms_signum = signum;
	ms->ms_handler = handler;
	status = sgx_ocall(108, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_get_cpuid_max(unsigned int* retval, unsigned int ext, unsigned int* sig)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_sig = sizeof(unsigned int);

	ms_ocall_get_cpuid_max_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_get_cpuid_max_t);
	void *__tmp = NULL;

	void *__tmp_sig = NULL;

	CHECK_ENCLAVE_POINTER(sig, _len_sig);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (sig != NULL) ? _len_sig : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_get_cpuid_max_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_get_cpuid_max_t));
	ocalloc_size -= sizeof(ms_ocall_get_cpuid_max_t);

	ms->ms_ext = ext;
	if (sig != NULL) {
		ms->ms_sig = (unsigned int*)__tmp;
		__tmp_sig = __tmp;
		if (_len_sig % sizeof(*sig) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_sig, 0, _len_sig);
		__tmp = (void *)((size_t)__tmp + _len_sig);
		ocalloc_size -= _len_sig;
	} else {
		ms->ms_sig = NULL;
	}
	
	status = sgx_ocall(109, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (sig) {
			if (memcpy_s((void*)sig, _len_sig, __tmp_sig, _len_sig)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_get_cpuid_count(int* retval, unsigned int leaf, unsigned int subleaf, unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_eax = sizeof(unsigned int);
	size_t _len_ebx = sizeof(unsigned int);
	size_t _len_ecx = sizeof(unsigned int);
	size_t _len_edx = sizeof(unsigned int);

	ms_ocall_get_cpuid_count_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_get_cpuid_count_t);
	void *__tmp = NULL;

	void *__tmp_eax = NULL;
	void *__tmp_ebx = NULL;
	void *__tmp_ecx = NULL;
	void *__tmp_edx = NULL;

	CHECK_ENCLAVE_POINTER(eax, _len_eax);
	CHECK_ENCLAVE_POINTER(ebx, _len_ebx);
	CHECK_ENCLAVE_POINTER(ecx, _len_ecx);
	CHECK_ENCLAVE_POINTER(edx, _len_edx);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (eax != NULL) ? _len_eax : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ebx != NULL) ? _len_ebx : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ecx != NULL) ? _len_ecx : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (edx != NULL) ? _len_edx : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_get_cpuid_count_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_get_cpuid_count_t));
	ocalloc_size -= sizeof(ms_ocall_get_cpuid_count_t);

	ms->ms_leaf = leaf;
	ms->ms_subleaf = subleaf;
	if (eax != NULL) {
		ms->ms_eax = (unsigned int*)__tmp;
		__tmp_eax = __tmp;
		if (_len_eax % sizeof(*eax) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_eax, 0, _len_eax);
		__tmp = (void *)((size_t)__tmp + _len_eax);
		ocalloc_size -= _len_eax;
	} else {
		ms->ms_eax = NULL;
	}
	
	if (ebx != NULL) {
		ms->ms_ebx = (unsigned int*)__tmp;
		__tmp_ebx = __tmp;
		if (_len_ebx % sizeof(*ebx) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_ebx, 0, _len_ebx);
		__tmp = (void *)((size_t)__tmp + _len_ebx);
		ocalloc_size -= _len_ebx;
	} else {
		ms->ms_ebx = NULL;
	}
	
	if (ecx != NULL) {
		ms->ms_ecx = (unsigned int*)__tmp;
		__tmp_ecx = __tmp;
		if (_len_ecx % sizeof(*ecx) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_ecx, 0, _len_ecx);
		__tmp = (void *)((size_t)__tmp + _len_ecx);
		ocalloc_size -= _len_ecx;
	} else {
		ms->ms_ecx = NULL;
	}
	
	if (edx != NULL) {
		ms->ms_edx = (unsigned int*)__tmp;
		__tmp_edx = __tmp;
		if (_len_edx % sizeof(*edx) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_edx, 0, _len_edx);
		__tmp = (void *)((size_t)__tmp + _len_edx);
		ocalloc_size -= _len_edx;
	} else {
		ms->ms_edx = NULL;
	}
	
	status = sgx_ocall(110, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (eax) {
			if (memcpy_s((void*)eax, _len_eax, __tmp_eax, _len_eax)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (ebx) {
			if (memcpy_s((void*)ebx, _len_ebx, __tmp_ebx, _len_ebx)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (ecx) {
			if (memcpy_s((void*)ecx, _len_ecx, __tmp_ecx, _len_ecx)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (edx) {
			if (memcpy_s((void*)edx, _len_edx, __tmp_edx, _len_edx)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_dladdr(int* retval, const void* addr, void* info)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_dladdr_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_dladdr_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_dladdr_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_dladdr_t));
	ocalloc_size -= sizeof(ms_ocall_dladdr_t);

	ms->ms_addr = addr;
	ms->ms_info = info;
	status = sgx_ocall(111, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_kill(int* retval, int pid, int sig)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_kill_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_kill_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_kill_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_kill_t));
	ocalloc_size -= sizeof(ms_ocall_kill_t);

	ms->ms_pid = pid;
	ms->ms_sig = sig;
	status = sgx_ocall(112, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getgrgid_r(int* retval, unsigned int gid, void* grp, char* buf, size_t buflen, void** result)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = buflen;

	ms_ocall_getgrgid_r_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getgrgid_r_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getgrgid_r_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getgrgid_r_t));
	ocalloc_size -= sizeof(ms_ocall_getgrgid_r_t);

	ms->ms_gid = gid;
	ms->ms_grp = grp;
	if (buf != NULL) {
		ms->ms_buf = (char*)__tmp;
		__tmp_buf = __tmp;
		if (_len_buf % sizeof(*buf) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_buflen = buflen;
	ms->ms_result = result;
	status = sgx_ocall(113, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getpwuid_r(int* retval, unsigned int uid, void* pwd, char* buf, size_t buflen, void** result)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = buflen;

	ms_ocall_getpwuid_r_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getpwuid_r_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getpwuid_r_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getpwuid_r_t));
	ocalloc_size -= sizeof(ms_ocall_getpwuid_r_t);

	ms->ms_uid = uid;
	ms->ms_pwd = pwd;
	if (buf != NULL) {
		ms->ms_buf = (char*)__tmp;
		__tmp_buf = __tmp;
		if (_len_buf % sizeof(*buf) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}
	
	ms->ms_buflen = buflen;
	ms->ms_result = result;
	status = sgx_ocall(114, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_deflateReset(int* retval, Z_STREAMP stream)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_deflateReset_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_deflateReset_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_deflateReset_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_deflateReset_t));
	ocalloc_size -= sizeof(ms_ocall_deflateReset_t);

	ms->ms_stream = stream;
	status = sgx_ocall(115, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_init(int* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_pthread_attr_init_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_init_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_init_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_init_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_init_t);

	status = sgx_ocall(116, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_create(int* retval, pthread_t* new_thread, unsigned long int job_id, sgx_enclave_id_t eid)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_new_thread = sizeof(pthread_t);

	ms_ocall_pthread_create_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_create_t);
	void *__tmp = NULL;

	void *__tmp_new_thread = NULL;

	CHECK_ENCLAVE_POINTER(new_thread, _len_new_thread);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (new_thread != NULL) ? _len_new_thread : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_create_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_create_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_create_t);

	if (new_thread != NULL) {
		ms->ms_new_thread = (pthread_t*)__tmp;
		__tmp_new_thread = __tmp;
		memset(__tmp_new_thread, 0, _len_new_thread);
		__tmp = (void *)((size_t)__tmp + _len_new_thread);
		ocalloc_size -= _len_new_thread;
	} else {
		ms->ms_new_thread = NULL;
	}
	
	ms->ms_job_id = job_id;
	ms->ms_eid = eid;
	status = sgx_ocall(117, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (new_thread) {
			if (memcpy_s((void*)new_thread, _len_new_thread, __tmp_new_thread, _len_new_thread)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_self(pthread_t* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_pthread_self_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_self_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_self_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_self_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_self_t);

	status = sgx_ocall(118, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_join(int* retval, pthread_t pt, void** res)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_pthread_join_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_join_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_join_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_join_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_join_t);

	ms->ms_pt = pt;
	ms->ms_res = res;
	status = sgx_ocall(119, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_getguardsize(int* retval, size_t* guardsize)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_guardsize = sizeof(size_t);

	ms_ocall_pthread_attr_getguardsize_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_getguardsize_t);
	void *__tmp = NULL;

	void *__tmp_guardsize = NULL;

	CHECK_ENCLAVE_POINTER(guardsize, _len_guardsize);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (guardsize != NULL) ? _len_guardsize : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_getguardsize_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_getguardsize_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_getguardsize_t);

	if (guardsize != NULL) {
		ms->ms_guardsize = (size_t*)__tmp;
		__tmp_guardsize = __tmp;
		if (_len_guardsize % sizeof(*guardsize) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_guardsize, 0, _len_guardsize);
		__tmp = (void *)((size_t)__tmp + _len_guardsize);
		ocalloc_size -= _len_guardsize;
	} else {
		ms->ms_guardsize = NULL;
	}
	
	status = sgx_ocall(120, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (guardsize) {
			if (memcpy_s((void*)guardsize, _len_guardsize, __tmp_guardsize, _len_guardsize)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_getguardsize__bypass(int* retval, void* attr, size_t attr_len, size_t* guardsize)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_attr = attr_len;
	size_t _len_guardsize = sizeof(size_t);

	ms_ocall_pthread_attr_getguardsize__bypass_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_getguardsize__bypass_t);
	void *__tmp = NULL;

	void *__tmp_attr = NULL;
	void *__tmp_guardsize = NULL;

	CHECK_ENCLAVE_POINTER(attr, _len_attr);
	CHECK_ENCLAVE_POINTER(guardsize, _len_guardsize);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (attr != NULL) ? _len_attr : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (guardsize != NULL) ? _len_guardsize : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_getguardsize__bypass_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_getguardsize__bypass_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_getguardsize__bypass_t);

	if (attr != NULL) {
		ms->ms_attr = (void*)__tmp;
		__tmp_attr = __tmp;
		memset(__tmp_attr, 0, _len_attr);
		__tmp = (void *)((size_t)__tmp + _len_attr);
		ocalloc_size -= _len_attr;
	} else {
		ms->ms_attr = NULL;
	}
	
	ms->ms_attr_len = attr_len;
	if (guardsize != NULL) {
		ms->ms_guardsize = (size_t*)__tmp;
		__tmp_guardsize = __tmp;
		if (_len_guardsize % sizeof(*guardsize) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_guardsize, 0, _len_guardsize);
		__tmp = (void *)((size_t)__tmp + _len_guardsize);
		ocalloc_size -= _len_guardsize;
	} else {
		ms->ms_guardsize = NULL;
	}
	
	status = sgx_ocall(121, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (attr) {
			if (memcpy_s((void*)attr, _len_attr, __tmp_attr, _len_attr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (guardsize) {
			if (memcpy_s((void*)guardsize, _len_guardsize, __tmp_guardsize, _len_guardsize)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_destroy(int* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_pthread_attr_destroy_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_destroy_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_destroy_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_destroy_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_destroy_t);

	status = sgx_ocall(122, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_condattr_setclock(int* retval, void* attr, clockid_t clock_id, size_t attr_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_attr = attr_len;

	ms_ocall_pthread_condattr_setclock_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_condattr_setclock_t);
	void *__tmp = NULL;

	void *__tmp_attr = NULL;

	CHECK_ENCLAVE_POINTER(attr, _len_attr);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (attr != NULL) ? _len_attr : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_condattr_setclock_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_condattr_setclock_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_condattr_setclock_t);

	if (attr != NULL) {
		ms->ms_attr = (void*)__tmp;
		__tmp_attr = __tmp;
		memset(__tmp_attr, 0, _len_attr);
		__tmp = (void *)((size_t)__tmp + _len_attr);
		ocalloc_size -= _len_attr;
	} else {
		ms->ms_attr = NULL;
	}
	
	ms->ms_clock_id = clock_id;
	ms->ms_attr_len = attr_len;
	status = sgx_ocall(123, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (attr) {
			if (memcpy_s((void*)attr, _len_attr, __tmp_attr, _len_attr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_destroy__bypass(int* retval, void* attr, size_t attr_len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_attr = attr_len;

	ms_ocall_pthread_attr_destroy__bypass_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_destroy__bypass_t);
	void *__tmp = NULL;

	void *__tmp_attr = NULL;

	CHECK_ENCLAVE_POINTER(attr, _len_attr);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (attr != NULL) ? _len_attr : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_destroy__bypass_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_destroy__bypass_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_destroy__bypass_t);

	if (attr != NULL) {
		ms->ms_attr = (void*)__tmp;
		__tmp_attr = __tmp;
		memset(__tmp_attr, 0, _len_attr);
		__tmp = (void *)((size_t)__tmp + _len_attr);
		ocalloc_size -= _len_attr;
	} else {
		ms->ms_attr = NULL;
	}
	
	ms->ms_attr_len = attr_len;
	status = sgx_ocall(124, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (attr) {
			if (memcpy_s((void*)attr, _len_attr, __tmp_attr, _len_attr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_getstack(int* retval, void** stk_addr, size_t* stack_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_stack_size = sizeof(size_t);

	ms_ocall_pthread_attr_getstack_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_getstack_t);
	void *__tmp = NULL;

	void *__tmp_stack_size = NULL;

	CHECK_ENCLAVE_POINTER(stack_size, _len_stack_size);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stack_size != NULL) ? _len_stack_size : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_getstack_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_getstack_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_getstack_t);

	ms->ms_stk_addr = stk_addr;
	if (stack_size != NULL) {
		ms->ms_stack_size = (size_t*)__tmp;
		__tmp_stack_size = __tmp;
		if (_len_stack_size % sizeof(*stack_size) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_stack_size, 0, _len_stack_size);
		__tmp = (void *)((size_t)__tmp + _len_stack_size);
		ocalloc_size -= _len_stack_size;
	} else {
		ms->ms_stack_size = NULL;
	}
	
	status = sgx_ocall(125, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (stack_size) {
			if (memcpy_s((void*)stack_size, _len_stack_size, __tmp_stack_size, _len_stack_size)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_attr_getstack__bypass(int* retval, void* attr, size_t attr_len, void** stk_addr, size_t len, size_t* stack_size)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_attr = attr_len;
	size_t _len_stk_addr = len;
	size_t _len_stack_size = sizeof(size_t);

	ms_ocall_pthread_attr_getstack__bypass_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_attr_getstack__bypass_t);
	void *__tmp = NULL;

	void *__tmp_attr = NULL;
	void *__tmp_stk_addr = NULL;
	void *__tmp_stack_size = NULL;

	CHECK_ENCLAVE_POINTER(attr, _len_attr);
	CHECK_ENCLAVE_POINTER(stk_addr, _len_stk_addr);
	CHECK_ENCLAVE_POINTER(stack_size, _len_stack_size);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (attr != NULL) ? _len_attr : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stk_addr != NULL) ? _len_stk_addr : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (stack_size != NULL) ? _len_stack_size : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_attr_getstack__bypass_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_attr_getstack__bypass_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_attr_getstack__bypass_t);

	if (attr != NULL) {
		ms->ms_attr = (void*)__tmp;
		__tmp_attr = __tmp;
		memset(__tmp_attr, 0, _len_attr);
		__tmp = (void *)((size_t)__tmp + _len_attr);
		ocalloc_size -= _len_attr;
	} else {
		ms->ms_attr = NULL;
	}
	
	ms->ms_attr_len = attr_len;
	if (stk_addr != NULL) {
		ms->ms_stk_addr = (void**)__tmp;
		__tmp_stk_addr = __tmp;
		if (_len_stk_addr % sizeof(*stk_addr) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_stk_addr, 0, _len_stk_addr);
		__tmp = (void *)((size_t)__tmp + _len_stk_addr);
		ocalloc_size -= _len_stk_addr;
	} else {
		ms->ms_stk_addr = NULL;
	}
	
	ms->ms_len = len;
	if (stack_size != NULL) {
		ms->ms_stack_size = (size_t*)__tmp;
		__tmp_stack_size = __tmp;
		if (_len_stack_size % sizeof(*stack_size) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_stack_size, 0, _len_stack_size);
		__tmp = (void *)((size_t)__tmp + _len_stack_size);
		ocalloc_size -= _len_stack_size;
	} else {
		ms->ms_stack_size = NULL;
	}
	
	status = sgx_ocall(126, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (attr) {
			if (memcpy_s((void*)attr, _len_attr, __tmp_attr, _len_attr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (stk_addr) {
			if (memcpy_s((void*)stk_addr, _len_stk_addr, __tmp_stk_addr, _len_stk_addr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (stack_size) {
			if (memcpy_s((void*)stack_size, _len_stack_size, __tmp_stack_size, _len_stack_size)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_getattr_np(int* retval, pthread_t tid)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_pthread_getattr_np_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_getattr_np_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_getattr_np_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_getattr_np_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_getattr_np_t);

	ms->ms_tid = tid;
	status = sgx_ocall(127, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_pthread_getattr_np__bypass(int* retval, pthread_t tid, void* attr, size_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_attr = len;

	ms_ocall_pthread_getattr_np__bypass_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_pthread_getattr_np__bypass_t);
	void *__tmp = NULL;

	void *__tmp_attr = NULL;

	CHECK_ENCLAVE_POINTER(attr, _len_attr);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (attr != NULL) ? _len_attr : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_pthread_getattr_np__bypass_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_pthread_getattr_np__bypass_t));
	ocalloc_size -= sizeof(ms_ocall_pthread_getattr_np__bypass_t);

	ms->ms_tid = tid;
	if (attr != NULL) {
		ms->ms_attr = (void*)__tmp;
		__tmp_attr = __tmp;
		memset(__tmp_attr, 0, _len_attr);
		__tmp = (void *)((size_t)__tmp + _len_attr);
		ocalloc_size -= _len_attr;
	} else {
		ms->ms_attr = NULL;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(128, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
		if (attr) {
			if (memcpy_s((void*)attr, _len_attr, __tmp_attr, _len_attr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_cpuinfo = 4 * sizeof(int);

	ms_sgx_oc_cpuidex_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_oc_cpuidex_t);
	void *__tmp = NULL;

	void *__tmp_cpuinfo = NULL;

	CHECK_ENCLAVE_POINTER(cpuinfo, _len_cpuinfo);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (cpuinfo != NULL) ? _len_cpuinfo : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_oc_cpuidex_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_oc_cpuidex_t));
	ocalloc_size -= sizeof(ms_sgx_oc_cpuidex_t);

	if (cpuinfo != NULL) {
		ms->ms_cpuinfo = (int*)__tmp;
		__tmp_cpuinfo = __tmp;
		if (_len_cpuinfo % sizeof(*cpuinfo) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset(__tmp_cpuinfo, 0, _len_cpuinfo);
		__tmp = (void *)((size_t)__tmp + _len_cpuinfo);
		ocalloc_size -= _len_cpuinfo;
	} else {
		ms->ms_cpuinfo = NULL;
	}
	
	ms->ms_leaf = leaf;
	ms->ms_subleaf = subleaf;
	status = sgx_ocall(129, ms);

	if (status == SGX_SUCCESS) {
		if (cpuinfo) {
			if (memcpy_s((void*)cpuinfo, _len_cpuinfo, __tmp_cpuinfo, _len_cpuinfo)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_wait_untrusted_event_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_wait_untrusted_event_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);

	ms->ms_self = self;
	status = sgx_ocall(130, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_set_untrusted_event_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_set_untrusted_event_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_set_untrusted_event_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);

	ms->ms_waiter = waiter;
	status = sgx_ocall(131, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(int* retval, const void* waiter, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_setwait_untrusted_events_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);

	ms->ms_waiter = waiter;
	ms->ms_self = self;
	status = sgx_ocall(132, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(int* retval, const void** waiters, size_t total)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_waiters = total * sizeof(void*);

	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(waiters, _len_waiters);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (waiters != NULL) ? _len_waiters : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_set_multiple_untrusted_events_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);

	if (waiters != NULL) {
		ms->ms_waiters = (const void**)__tmp;
		if (_len_waiters % sizeof(*waiters) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, waiters, _len_waiters)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_waiters);
		ocalloc_size -= _len_waiters;
	} else {
		ms->ms_waiters = NULL;
	}
	
	ms->ms_total = total;
	status = sgx_ocall(133, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL pthread_wait_timeout_ocall(int* retval, unsigned long long waiter, unsigned long long timeout)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_pthread_wait_timeout_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_pthread_wait_timeout_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_pthread_wait_timeout_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_pthread_wait_timeout_ocall_t));
	ocalloc_size -= sizeof(ms_pthread_wait_timeout_ocall_t);

	ms->ms_waiter = waiter;
	ms->ms_timeout = timeout;
	status = sgx_ocall(134, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL pthread_create_ocall(int* retval, unsigned long long self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_pthread_create_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_pthread_create_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_pthread_create_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_pthread_create_ocall_t));
	ocalloc_size -= sizeof(ms_pthread_create_ocall_t);

	ms->ms_self = self;
	status = sgx_ocall(135, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL pthread_wakeup_ocall(int* retval, unsigned long long waiter)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_pthread_wakeup_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_pthread_wakeup_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_pthread_wakeup_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_pthread_wakeup_ocall_t));
	ocalloc_size -= sizeof(ms_pthread_wakeup_ocall_t);

	ms->ms_waiter = waiter;
	status = sgx_ocall(136, ms);

	if (status == SGX_SUCCESS) {
		if (retval) *retval = ms->ms_retval;
	}
	sgx_ocfree();
	return status;
}

