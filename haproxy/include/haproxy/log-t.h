/*
 * include/haproxy/log-t.h
 * This file contains definitions of log-related structures and macros.
 *
 * Copyright (C) 2000-2020 Willy Tarreau - w@1wt.eu
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, version 2.1
 * exclusively.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _HAPROXY_LOG_T_H
#define _HAPROXY_LOG_T_H

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include <haproxy/api-t.h>
#include <haproxy/ring-t.h>
#include <haproxy/thread-t.h>


#define NB_LOG_FACILITIES       24
#define NB_LOG_LEVELS           8
#define NB_LOG_HDR_MAX_ELEMENTS 15
#define SYSLOG_PORT             514
#define UNIQUEID_LEN            128

/* flags used in logformat_node->options */
#define LOG_OPT_NONE            0x00000000
#define LOG_OPT_HEXA            0x00000001
#define LOG_OPT_MANDATORY       0x00000002
#define LOG_OPT_QUOTE           0x00000004
#define LOG_OPT_REQ_CAP         0x00000008
#define LOG_OPT_RES_CAP         0x00000010
#define LOG_OPT_HTTP            0x00000020
#define LOG_OPT_ESC             0x00000040
#define LOG_OPT_MERGE_SPACES    0x00000080
#define LOG_OPT_BIN             0x00000100
/* unused: 0x00000200 ... 0x00000800 */
#define LOG_OPT_ENCODE_JSON     0x00001000
#define LOG_OPT_ENCODE_CBOR     0x00002000
#define LOG_OPT_ENCODE          0x00003000


/* Fields that need to be extracted from the incoming connection or request for
 * logging or for sending specific header information. They're set in px->to_log
 * and appear as flags in session->logs.logwait, which are removed once the
 * required information has been collected.
 */
#define LW_INIT             1        /* anything */
#define LW_CLIP             2        /* CLient IP */
#define LW_SVIP             4        /* SerVer IP */
#define LW_SVID             8        /* server ID */
#define LW_REQ             16        /* http REQuest */
#define LW_RESP            32        /* http RESPonse */
#define LW_BYTES          256        /* bytes read from server */
#define LW_COOKIE         512        /* captured cookie */
#define LW_REQHDR        1024        /* request header(s) */
#define LW_RSPHDR        2048        /* response header(s) */
#define LW_BCKIP         4096        /* backend IP */
#define LW_FRTIP         8192        /* frontend IP */
#define LW_XPRT         16384        /* transport layer information (eg: SSL) */

#define LOG_LEGACYTIME_LEN 15
#define LOG_ISOTIME_MINLEN 20
#define LOG_ISOTIME_MAXLEN 32

/* enum for log format */
enum log_fmt {
	LOG_FORMAT_UNSPEC = 0,
	LOG_FORMAT_LOCAL,
	LOG_FORMAT_RFC3164,
	LOG_FORMAT_RFC5424,
	LOG_FORMAT_PRIO,
	LOG_FORMAT_SHORT,
	LOG_FORMAT_TIMED,
	LOG_FORMAT_ISO,
	LOG_FORMAT_RAW,
	LOG_FORMATS           /* number of supported log formats, must always be last */
};

/* enum log header meta data */
enum log_meta {
        LOG_META_PRIO,
        LOG_META_TIME,
        LOG_META_HOST,
        LOG_META_TAG,
        LOG_META_PID,
        LOG_META_MSGID,
        LOG_META_STDATA,
        LOG_META_FIELDS  /* must always be the last */
};

/* log header data */
struct log_header {
	enum log_fmt format;  /* how to format the header */
	int level, facility;  /* used by several formats */
	struct ist *metadata; /* optional metadata - per-format */
};

#define LOG_HEADER_NONE (struct log_header){                              \
                                             .format = LOG_FORMAT_UNSPEC, \
                                             .level = 0,                  \
                                             .facility = 0,               \
                                             .metadata = NULL             \
                                           }

/* log target types */
enum log_tgt {
	LOG_TARGET_DGRAM = 0, // datagram address (udp, unix socket)
	LOG_TARGET_FD,        // file descriptor
	LOG_TARGET_BUFFER,    // ring buffer
	LOG_TARGET_BACKEND,   // backend with SYSLOG mode
};

/* lists of fields that can be logged, for logformat_node->type */
enum {

	LOG_FMT_TEXT = 0,  /* raw text */
	LOG_FMT_EXPR,      /* sample expression */
	LOG_FMT_SEPARATOR, /* separator replaced by one space */
	LOG_FMT_ALIAS,     /* reference to logformat_alias */
};

/* enum for parse_logformat_string */
enum {
	LF_INIT = 0,   // before first character
	LF_TEXT,       // normal text
	LF_SEPARATOR,  // a single separator
	LF_ALIAS,      // alias name, after '%' or '%{..}'
	LF_STARTALIAS, // % in text
	LF_STONAME,    // after '%(' and before ')'
	LF_STOTYPE,    // after ':' while in STONAME
	LF_EDONAME,    // ')' after '%('
	LF_STARG,      // after '%{' and berore '}'
	LF_EDARG,      // '}' after '%{'
	LF_STEXPR,     // after '%[' or '%{..}[' and berore ']'
	LF_EDEXPR,     // ']' after '%['
	LF_END,        // \0 found
};

/* log_format aliases (ie: %alias), see logformat_aliases table in log.c for
 * available aliases definitions
 */
struct logformat_node; // forward-declaration
struct logformat_alias {
	char *name;
	int type;
	int mode;
	int lw; /* logwait bitsfield */
	int (*config_callback)(struct logformat_node *node, struct proxy *curproxy);
};

struct logformat_node {
	struct list list;
	int type;      // LOG_FMT_*
	int options;   // LOG_OPT_*
	int typecast;  // explicit typecasting for printing purposes (SMP_T_{SAME,BOOL,STR,SINT})
	char *name;    // printable name for output types that require named fields (ie: json)
	char *arg;     // text for LOG_FMT_TEXT, arg for others
	void *expr;    // for use with LOG_FMT_EXPR
	const struct logformat_alias *alias; // set if ->type == LOG_FMT_ALIAS
};

enum lf_expr_flags {
	LF_FL_NONE     = 0x00,
	LF_FL_COMPILED = 0x01
};

/* a full logformat expr made of one or multiple logformat nodes */
struct lf_expr {
	struct list list;                 /* to store lf_expr inside a list */
	union {
		struct {
			struct list list; /* logformat_node list */
			int options;      /* global '%o' options (common to all nodes) */
		} nodes;
		char *str;                /* original string prior to parsing (NULL once compiled) */
	};
	struct {
		char *file;               /* file where the lft appears */
		int line;                 /* line where the lft appears */
	} conf; // parsing hints
	uint8_t flags;             /* LF_FL_* flags */
};

/* Range of indexes for log sampling. */
struct smp_log_range {
	unsigned int low;        /* Low limit of the indexes of this range. */
	unsigned int high;       /* High limit of the indexes of this range. */
	size_t sz;               /* The size of this range, or number of indexes in
	                          * this range.
	                          */
};

/* Log sampling information. */
struct smp_info {
	struct smp_log_range *smp_rgs; /* Array of ranges for log sampling. */
	size_t smp_rgs_sz;             /* The size of <smp_rgs> array. */
	size_t smp_sz;             /* The total number of logs to be sampled. */
	ullong curr_rg_idx;        /* 63:32 = current range; 31:0 = current index */
};

enum log_target_flags {
	LOG_TARGET_FL_NONE     = 0x00,
	LOG_TARGET_FL_RESOLVED = 0x01
};

struct log_target {
	struct sockaddr_storage *addr;
	union {
		char *ring_name;   /* type = BUFFER  - preparsing */
		struct sink *sink; /* type = BUFFER  - postparsing */
		char *be_name;     /* type = BACKEND - preparsing */
		struct proxy *be;  /* type = BACKEND - postparsing */
		char *resolv_name; /* generic        - preparsing */
	};
	enum log_tgt type;
	uint16_t flags;
};

struct logger {
	struct list list;
	struct log_target target;
	struct smp_info lb;
	enum log_fmt format;
	int facility;
	int level;
	int minlvl;
	int maxlen;
	struct logger *ref;
	struct {
                char *file;                     /* file where the logger appears */
                int line;                       /* line where the logger appears */
        } conf;
};

#endif /* _HAPROXY_LOG_T_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 */
