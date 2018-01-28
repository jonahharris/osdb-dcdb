/*	Header File:	index.h	*/

#ifndef	__INDEX_H__
#define	__INDEX_H__

#include <block.h>

#ifdef	__cplusplus
externC
#endif
/*
 * [BeginDoc]
 * 
 * \section{Index Module}
 * 
 * This is the index module.  This module is dependent on the 
 * following:
 * 
 * fsystem module
 * shell module
 * list module
 * block module
 * 
 * The index module is an attempt to create an indexing scheme for
 * an Indexed Sequential Access Method for table access.  The design
 * goals are as follows:
 * 
 * \begin{itemize}
 * \item The module should be robust.
 * \item  The module should be fast.
 * \item The module should use a block I/O scheme and queued LRU block
 * buffers.  LRU stands for ``Least Recently Used''.
 * \end{itemize}
 * 
 * The algorithm uses the list module and the shell extensively.  
 * A thorough
 * understanding of these modules is a must for understanding the
 * code for the index module.
 * 
 * An index is an ordered list of information.  There are two
 * primary structures the user should know about for every index.
 * When an index file is open, the actual index data is stored in
 * a ``block file'' (see the block module).  This file is
 * \index{block file}
 * buffered using an LRU cache of blocks for efficiency.  The second
 * \index{LRU}
 * structure is an in-memory ``index'' of the block file data.  This
 * uses the shell module to index the blocks that are stored in the
 * block file.  When the index is closed, the shell is stored to disk
 * in a file separate from the block file.  Once the shell is
 * retrieved from disk, the file used to store it is closed.  Hence,
 * the file the in-memory shell is stored in is only for persistence
 * between index opens.  The block file, on the other hand, is
 * open as long as the index is in use.
 * 
 * An index is created with a block size which will remain the
 * block size for that index for all future accesses.  If a block
 * \index{block size}
 * size of 0 is specified, a default block size is used.  Currently,
 * the default block size is 512, but this is alterable by changing
 * a defined constant in the index.h file.  It is important that the
 * block size be selected for an index emperically.  A good rule of
 * thumb is to select a block size that allows for 10-20 items stored
 * per block.  This seems to give the best performance.
 * 
 * When an index file is created or opened, a cache of blocks is
 * \index{block cache}
 * reserved in memory.  At this writing, the cache size is 6 and has been
 * determined by extensive testing for maximum efficiency.  This may
 * need to be altered on a system that does not provide the file buffering
 * that Linux does (all the testing was conducted on a Linux station).
 * Again, it is alterable by changing a defined constant in the 
 * index.h file.
 * 
 * Error conditions are communicated to the user by setting a global
 * variable called idxError.  idxError is of type idxErrorType, which
 * \index{idxError} \index{idxErrMsg}
 * is defined as follows:
 * 
 * [EndDoc]
 */
#define	IDX_MAGIC			0xfeedfaceUL
#define	DEFAULT_BLOCK_SIZE	512
#define	BLOCK_CACHE_SIZE	9
#define	MIN_ITEMS_PER_BLOCK	4
#define	TREE_NODES			24
#define	INDEX_NAME_WIDTH	34
#define	SHELL_THRESHOLD		250
#define	NOT_INSERTED		2
/*
 * [BeginDoc]
 * [Verbatim]	*/
typedef enum __idx_error_type {
  IDX_NOERR,                    /* no error */
  IDX_FSYSTEM,                  /* low level file error */
  IDX_NOMEMORY,                 /* fatal memory error */
  IDX_LIST,                     /* list error */
  IDX_SHELL,                    /* shell error */
  IDX_FTYPE,                    /* invalid file type */
  IDX_UNSTABLE,                 /* memory pointers corrupted */
  IDX_ITEMS,                    /* item size too large */
  IDX_UNIQUE,                   /* unique constraint violated */
  IDX_SEQUENCE,                 /* sequential access not */
                                /*   initiated */
  IDX_DUP,                      /* is/is not a dup index */
  IDX_NEXISTS,			/* index pair doesn't exist */
  /* put new ones here */
  IDX_UNSPECIFIED               /* generic error */
} idxErrorType;

extern idxErrorType idxError;

extern char *idxErrMsg[];

/* [EndDoc] */

/*
 * [BeginDoc]
 * 
 * On an error condition, idxErrMsg[idxError] will point to a string
 * in memory that describes the error.
 * 
 * =================================================================
 * 
 * There are several C structures which are used to manage index
 * information in memory.  The first is the indexHeader structure.
 * This is stamped into the control block of the block file when
 * it is created.  All updates that effect this information are
 * saved to the block file when it is closed.  The indexHeader
 * structure is defined as follows:
 * 
 * -----------------------------------------------------------------
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * [Verbatim]		*/

typedef struct __index_header {
  unsigned long magic;         /* magic number */
  char indexName[INDEX_NAME_WIDTH + 1];
  int blockSize;               /* block size of this file */
  off_t lastBlock;             /* last block/number of */
  /*   blocks */
  int isUnique;                 /* unique index? */
  int isCase;                   /* case sensitive index? */
  int itemSize;                 /* size of each key/item */
  unsigned long numItems;       /* total number of items */
  /*   added */
} indexHeader;

/* [EndDoc] */

/*
 * [BeginDoc]
 * -----------------------------------------------------------------
 * 
 * ``magic'' will contain the ``magic number'' that identifies this
 * as an index file.  ``indexName'' will contain the name of the
 * long name of the index.  ``blockSize'' is the block size of the
 * block file that contains the leaf nodes.  ``lastBlock'' is the
 * last block number in the block file.  This is used internally
 * by the index routines for appending new blocks to the block
 * file.  ``isUnique'' tells whether the index allows duplicates
 * or not.  If isUnique is FALSE, duplicates are allowd.  If it is
 * TRUE, duplicates are not allowed.  ``isCase'' indicates whether
 * the index is case sensitive (isCase is TRUE) or not (isCase is
 * FALSE).  ``itemSize'' is the size of each key/item pair in the
 * index.  An index file consists of the keys (the actual indexed
 * elements) along with associated 32 bit integer items.  These
 * items can be anything the user wants them to be, but are 
 * generally thought to be record numbers or offsets into a data
 * file.  The itemSize will be the size of the actual keys + the
 * size of a 32 bit integer (presumably 4 bytes).  ``numItems'' is
 * the number of items stored in the index.
 * 
 * The next structure used to manage index information is the
 * blockHeader structure.  This is stamped into the beginning of
 * each block in the block file.  The blockHeader structure is
 * defined as follows:
 * 
 * -----------------------------------------------------------------
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * [Verbatim]		*/

typedef struct __block_header {
  int numKeyItems;              /* number of key/item pairs */
  off_t prevBlock;              /* previous block (for */
  /*   backward sequential) */
  off_t nextBlock;              /* next block (forward */
                                /*   sequential) */
                                /* nextBlock and prevBlock */
                                /*   only applicable if this */
                                /*   is a leaf node. */
} blockHeader;

 /* [EndDoc] */

/*
 * [BeginDoc]
 * -----------------------------------------------------------------
 * 
 * ``numKeyItems'' is the number of key/item pairs stored in this
 * block.  ``prevBlock'' is the previous block sequentially in order
 * of the index data.  ``nextBlock'' is the next block sequentially.
 * These items are necessary for effecient sequential access.  Blocks
 * are not necessarily stored to the block file in sequential order,
 * so these data items are used for retrieving blocks when the user
 * is traversing the index sequentially.
 * 
 * The next structure used to manage indexes is the keyItem 
 * structure.  This is really somewhat of a kludge.  I've always
 * thought when I've seen stuff like this that I could do a better
 * job in this arena.  Oh, well... :o)
 * 
 * When an item is added to an index, a chunk of memory of size
 * itemSize, as contained in the indexHeader structure, is allocated
 * and a pointer to an item is set to point to it.  The item and
 * key are then copied into the item and key members, and this data
 * is what is stored in the index.  So, in essence, this is used
 * as a template for storing and manipulating indexes.  It relies
 * on the fact that ``an array decays to a pointer when the array
 * is referenced by name''.  The keyItem structure is defined as
 * follows:
 * 
 * -----------------------------------------------------------------
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * [Verbatim]		*/

typedef struct __key_item {
  unsigned long item;           /* item referenced */
  char key[1];                  /* associated key */
} keyItem;

/* [EndDoc] */

/*
 * [BeginDoc]
 * -----------------------------------------------------------------
 * 
 * The members of this structure are self explanatory.
 * 
 * The next structure used to manage indexes is the idxBlock
 * structure.  The idxBlock structure stores the information that
 * is used to keep blocks (as from a block file) in memory.  When a
 * block is read into memory, it's information is parsed and put
 * into a linked list.  The idxBlock structure is defined as
 * follows:
 * 
 * -----------------------------------------------------------------
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * [Verbatim]		*/

typedef struct __idx_block {
  off_t blockNumber;            /* number of this block */
  void *blk;                    /* associated block */
  blockHeader *bhd;             /* block header - points to blk */
  int isDirty;                  /* has this idxBlock changed? */
  int numMaxItems;              /* max number of items that can */
  /*   be added to this leaf */
  int itemSize;                 /* size of each key/item pair */
  int currentSize;              /* current total size of key/items */
  keyItem *firstItem;           /* pointer to first key/item pair */
  Link *current;                /* current link in the block */
#ifndef	BIG_BLOCKS
  ListHeader *keyItemList;      /* list of key/items */
#endif
#ifdef	BIG_BLOCKS
  shellHeader *keyItemList;
#endif
} idxBlock;

/* [EndDoc] */

/*
 * [BeginDoc]
 * -----------------------------------------------------------------
 * 
 * The ``blockNumber'' member will contain the number of the block.
 * The ``blk'' member will point to an allocated block of memory of
 * size blockSize as stored in the indexHeader.  The block read from
 * disk will be stored in this memory.  ``bhd'' points to an allocated
 * blockHeader as read from the block.  ``isDirty'' will determine if
 * the block has been changed and should be stored to disk.
 * ``numMaxItems'' is a calculated number based on the size of each
 * key/index item and determines the threshold at which the block
 * must be split.  ``itemSize'' is the size of each key/item pair.
 * It is the same as that stored in the indexHeader, but is stored
 * here to make it readily available as needed.  ``firstItem'' is a
 * pointer to the first key/item pair stored in this block.
 * ``current'' is used for sequential access to show which key/item
 * is the current key/item.  Finally, the ``keyItemList'' member
 * is the ListHeader which manages the linked list of key/items as
 * parsed from the disk block when the block is read from memory
 * (see the list module documentation for more information).
 * 
 * The final structure used to manage indexes is the dbIndex
 * structure.  This pulls all the other management structures
 * together.  The index functions all receive a pointer to a
 * dbIndex structure and make all changes to the data that it
 * refers to in doing their work.  A dbIndex structure is defined
 * as follows:
 * 
 * -----------------------------------------------------------------
 * [EndDoc]
 */

/*
 * [BeginDoc]
 * [Verbatim]		*/

typedef struct __db_index {
  int fd;                       /* file descriptor of open block file */
  indexHeader *hdr;             /* index header */
  void *ctrl;                   /* control block in memory */
  shellHeader *inx;             /* in-memory index for leafs */
  ListHeader *free;             /* free blocks */
  ListHeader *used;             /* used blocks */
  int bof;                      /* beginning of the index? */
  int eof;                      /* end of the index? */
  idxBlock *current;            /* current block */
  int isSequential;             /* are we accessing the index */
  /*   sequentially */
} dbIndex;

/* [EndDoc] */

/*
 * [BeginDoc]
 * -----------------------------------------------------------------
 * 
 * The ``fd'' member is the descriptor of the open block file.  ``hdr''
 * points to an allocated indexHeader as read from the control block
 * of the block file.  ``inx'' is the treeHeader for the in-memory
 * tree that indexes the blocks in the block file.  ``free'' is a
 * pointer to a ListHeader that manages the list of free blocks for
 * the index.  This is the cache of blocks that is maintained for the
 * open index.  The ``used'' member is a pointer to a ListHeader that
 * manages the queue of used blocks.  This is used to provide an
 * LRU (least recently used) cache of blocks when the free blocks are
 * exhausted.  ``bof'' and ``eof'' are indicators of whether we are 
 * at the end or beginning of the index in terms of sequential access
 * of the data.  ``current'' points to the current blockHeader when
 * the user is accessing the index data sequentially.  And finally,
 * the ``isSequential'' member is set to TRUE if sequential access
 * is under way.  Some functions should not be called unless
 * the index data is being accessed sequentially by the user.
 * 
 * =================================================================
 * [EndDoc]
 */

/*
 * Index functions
 * 
 * @DocInclude index.c
 */
int createIndexFile (char *name, int blockSize, int fldsize,
		     int isUnique, int isCase);
int caseCompare (const void *p1, const void *p2);
int noCaseCompare (const void *p1, const void *p2);
dbIndex *openIndex (char *idxName, char *inxName, int blockSize);
/*dbIndex *openDupIndex (char *idxName, char *inxName, int blockSize);*/
int closeIndexFile (dbIndex * idx, char *inxName);
int addIndexItem (dbIndex * idx, keyItem * ki);
keyItem *deleteIndexItem (dbIndex *idx, keyItem *ki);
/*int restructureIndexShell (dbIndex *idx);*/
Link *searchExactIndexItem (dbIndex * idx, Link * lnk);
Link *searchIndexItem (dbIndex * idx, Link * lnk);
keyItem *firstIndexItem (dbIndex * idx);
keyItem *lastIndexItem (dbIndex * idx);
keyItem *nextIndexItem (dbIndex * idx);
keyItem *prevIndexItem (dbIndex * idx);

/* BUGBUG
#ifdef	DEBUG
void DebugDumpBlock (idxBlock *blk);
#endif
#ifndef	DEBUG
#define	DebugDumpBlock(blk)
#endif
 BUGBUG */

/*
 * Block index file functions
 * 
 * @DocInclude idxblk.c
 */
idxBlock *getIndexBlock (dbIndex * idx, off_t blknum);
idxBlock *getNewBlock (dbIndex * idx, off_t blknum);
idxBlock *splitBlock (dbIndex * idx, idxBlock * blk);
int flushIndexBlocks (dbIndex * idx);

#ifdef	__cplusplus
endC
#endif
#endif /* __INDEX_H__ */
