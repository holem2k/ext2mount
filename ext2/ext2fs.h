#ifndef __EXT2FS_H__
#define __EXT2FS_H__

// Основные структуры файловой системы  ext2


/*
 * Constants relative to the data blocks
 */

const ULONG EXT2_NDIR_BLOCKS	= 12;
const ULONG EXT2_IND_BLOCK		= EXT2_NDIR_BLOCKS;
const ULONG EXT2_DIND_BLOCK		= (EXT2_IND_BLOCK + 1);
const ULONG EXT2_TIND_BLOCK		= (EXT2_DIND_BLOCK + 1);
const ULONG EXT2_N_BLOCKS		= (EXT2_TIND_BLOCK + 1);


/*
 * Inode flags
 */

const ULONG EXT2_SECRM_FL		= 0x00000001; /* Secure deletion */
const ULONG EXT2_UNRM_FL		= 0x00000002; /* Undelete */
const ULONG EXT2_COMPR_FL		= 0x00000004; /* Compress file */
const ULONG EXT2_SYNC_FL		= 0x00000008; /* Synchronous updates */
const ULONG EXT2_IMMUTABLE_FL	= 0x00000010; /* Immutable file */
const ULONG EXT2_APPEND_FL		= 0x00000020; /* writes to file may only append */
const ULONG EXT2_NODUMP_FL		= 0x00000040; /* do not dump file */
const ULONG EXT2_NOATIME_FL		= 0x00000080; /* do not update atime */
/* Reserved for compression usage... */
const ULONG EXT2_DIRTY_FL		= 0x00000100;
const ULONG EXT2_COMPRBLK_FL	 = 0x00000200; /* One or more compressed clusters */
const ULONG EXT2_NOCOMP_FL		= 0x00000400; /* Don't compress */
const ULONG EXT2_ECOMPR_FL		= 0x00000800; /* Compression error */
/* End compression flags --- maybe not all used */	
const ULONG EXT2_BTREE_FL		= 0x00001000; /* btree format dir */
const ULONG EXT2_RESERVED_FL	= 0x80000000; /* reserved for ext2 lib */

/*
 * Special inodes numbers
 */
const ULONG	EXT2_BAD_INO		= 1;		/* Bad blocks inode */
const ULONG	EXT2_ROOT_INO		= 2;		/* Root inode */
const ULONG	EXT2_ACL_IDX_INO	= 3;		/* ACL inode */
const ULONG	EXT2_ACL_DATA_INO	= 4;		/* ACL inode */
const ULONG	EXT2_BOOT_LOADER_INO = 5;	/* Boot loader inode */
const ULONG	EXT2_UNDEL_DIR_INO	=6;		/* Undelete directory inode */

const ULONG EXT2_DIR_PAD = 4;
const ULONG EXT2_DIR_ROUND = EXT2_DIR_PAD - 1;

#define EXT2_DIR_REC_LEN(name_len)	(((name_len) + 8 + EXT2_DIR_ROUND) & \
					 ~EXT2_DIR_ROUND)

#pragma pack(push, __ext2fs_h__)
#pragma pack(1)

typedef struct EXT2_INODE_REAL
{
	USHORT	i_mode;		/* File mode */
	USHORT	i_uid;		/* Owner Uid */
	ULONG	i_size;		/* Size in bytes */
	ULONG	i_atime;	/* Access time */
	ULONG	i_ctime;	/* Creation time */
	ULONG	i_mtime;	/* Modification time */
	ULONG	i_dtime;	/* Deletion Time */
	USHORT	i_gid;		/* Group Id */
	USHORT	i_links_count;	/* Links count */
	ULONG	i_blocks;	/* Blocks count */
	ULONG	i_flags;	/* File flags */
	union {
		struct {
			ULONG  l_i_reserved1;
		} linux1;
		struct {
			ULONG  h_i_translator;
		} hurd1;
		struct {
			ULONG  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	ULONG	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	ULONG	i_version;	/* File version (for NFS) */
	ULONG	i_file_acl;	/* File ACL */
	ULONG	i_dir_acl;	/* Directory ACL */
	ULONG	i_faddr;	/* Fragment address */
	union {
		struct {
			UCHAR	l_i_frag;	/* Fragment number */
			UCHAR	l_i_fsize;	/* Fragment size */
			USHORT	i_pad1;
			ULONG	l_i_reserved2[2];
		} linux2;
		struct {
			UCHAR	h_i_frag;	/* Fragment number */
			UCHAR	h_i_fsize;	/* Fragment size */
			USHORT	h_i_mode_high;
			USHORT	h_i_uid_high;
			USHORT	h_i_gid_high;
			ULONG	h_i_author;
		} hurd2;
		struct {
			UCHAR	m_i_frag;	/* Fragment number */
			UCHAR	m_i_fsize;	/* Fragment size */
			USHORT	m_pad1;
			ULONG	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
} EXT2_INODE_REAL, *PEXT2_INODE_REAL;

 


typedef struct EXT2_GROUP_DESC_REAL
{
	ULONG	bg_block_bitmap;		/* Blocks bitmap block */
	ULONG	bg_inode_bitmap;		/* Inodes bitmap block */
	ULONG	bg_inode_table;		    /* Inodes table block  */
	USHORT	bg_free_blocks_count;	/* Free blocks count   */
	USHORT	bg_free_inodes_count;	/* Free inodes count   */
	USHORT	bg_used_dirs_count;	    /* Directories count   */
	USHORT	bg_pad;
	ULONG	bg_reserved[3];
} EXT2_GROUP_DESC_REAL, *PEXT2_GROUP_DESC_REAL;


const USHORT EXT2_SUPER_MAGIC	= 0xEF53;


typedef struct EXT2_SUPERBLOCK_REAL
{
	ULONG	s_inodes_count;		/* Inodes count */
	ULONG	s_blocks_count;		/* Blocks count */
	ULONG	s_r_blocks_count;	/* Reserved blocks count */
	ULONG	s_free_blocks_count;	/* Free blocks count */
	ULONG	s_free_inodes_count;	/* Free inodes count */
	ULONG	s_first_data_block;	/* First Data Block */
	ULONG	s_log_block_size;	/* Block size */
	int	s_log_frag_size;	/* Fragment size */
	ULONG	s_blocks_per_group;	/* # Blocks per group */
	ULONG	s_frags_per_group;	/* # Fragments per group */
	ULONG	s_inodes_per_group;	/* # Inodes per group */
	ULONG	s_mtime;		/* Mount time */
	ULONG	s_wtime;		/* Write time */
	USHORT	s_mnt_count;		/* Mount count */
	short	s_max_mnt_count;	/* Maximal mount count */
	USHORT	s_magic;		/* Magic signature */
	USHORT	s_state;		/* File system state */
	USHORT	s_errors;		/* Behaviour when detecting errors */
	USHORT	s_minor_rev_level; 	/* minor revision level */
	ULONG	s_lastcheck;		/* time of last check */
	ULONG	s_checkinterval;	/* max. time between checks */
	ULONG	s_creator_os;		/* OS */
	ULONG	s_rev_level;		/* Revision level */
	USHORT	s_def_resuid;		/* Default uid for reserved blocks */
	USHORT	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */	
	ULONG	s_first_ino; 		/* First non-reserved inode */
	USHORT   s_inode_size; 		/* size of inode structure */
	USHORT	s_block_group_nr; 	/* block group # of this superblock */
	ULONG	s_feature_compat; 	/* compatible feature set */
	ULONG	s_feature_incompat; 	/* incompatible feature set */
	ULONG	s_feature_ro_compat; 	/* readonly-compatible feature set */
	UCHAR	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	ULONG	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	UCHAR	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	UCHAR	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	USHORT	s_padding1;
	ULONG	s_reserved[204];	/* Padding to the end of the block */
} EXT2_SUPERBLOCK_REAL, *PEXT2_SUPERBLOCK_REAL;


const ULONG SUPERBLOCK_OFFSET = 1024;

const ULONG EXT2_NAME_LEN  = 255;

const ULONG S_IFMT		= 0170000;
const ULONG S_IFSOCK	= 0140000;
const ULONG S_IFLNK		= 0120000;
const ULONG S_IFREG		= 0100000;
const ULONG S_IFBLK		= 0060000;
const ULONG S_IFDIR		= 0040000;
const ULONG S_IFCHR		= 0020000;
const ULONG S_IFIFO		= 0010000;

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define i_size_high i_dir_acl

const ULONG  EXT2_GOOD_OLD_REV = 0;		/* The good old (original) format */
const ULONG  EXT2_DYNAMIC_REV  = 1; 	/* V2 format w/ dynamic inode sizes */
const ULONG  EXT2_GOOD_OLD_INODE_SIZE = 128;
const ULONG  EXT2_GOOD_OLD_FIRST_INO  = 11;

const ULONG EXT2_FEATURE_COMPAT_DIR_PREALLOC	= 0x0001;
const ULONG EXT2_FEATURE_COMPAT_IMAGIC_INODES	= 0x0002;
const ULONG EXT2_FEATURE_COMPAT_HAS_JOURNAL		= 0x0004;
const ULONG EXT2_FEATURE_COMPAT_EXT_ATTR		= 0x0008;
const ULONG EXT2_FEATURE_COMPAT_RESIZE_INODE	= 0x0010;
const ULONG EXT2_FEATURE_COMPAT_DIR_INDEX		= 0x0020;

const ULONG EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	= 0x0001;
const ULONG EXT2_FEATURE_RO_COMPAT_LARGE_FILE	= 0x0002;
const ULONG EXT2_FEATURE_RO_COMPAT_BTREE_DIR	= 0x0004;

const ULONG EXT2_FEATURE_INCOMPAT_COMPRESSION	= 0x0001;
const ULONG EXT2_FEATURE_INCOMPAT_FILETYPE		= 0x0002;
const ULONG EXT2_FEATURE_INCOMPAT_RECOVER		= 0x0004; /* Needs recovery */
const ULONG EXT2_FEATURE_INCOMPAT_JOURNAL_DEV	= 0x0008; /* Journal device */

const ULONG EXT2_FEATURE_COMPAT_SUPP = 0;
const ULONG EXT2_FEATURE_INCOMPAT_SUPP = EXT2_FEATURE_INCOMPAT_FILETYPE | 
					 EXT2_FEATURE_INCOMPAT_RECOVER;
const ULONG EXT2_FEATURE_RO_COMPAT_SUPP	= EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER|
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE |
					 EXT2_FEATURE_RO_COMPAT_BTREE_DIR;


typedef struct EXT2_DIR_ENTRY_REAL
{
	ULONG	inode;					/* Inode number */
	USHORT	rec_len;				/* Directory entry length */
	UCHAR	name_len;				/* Name length */
//	char	name[EXT2_NAME_LEN];	/* File name */
} EXT2_DIR_ENTRY_REAL, *PEXT2_DIR_ENTRY_REAL;

#pragma pack(pop, __ext2fs_h__)

#endif //__EXT2FS_H__