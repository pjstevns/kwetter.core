

#ifndef _KW_H
#define _KW_H

#include <zdb.h>
#include "helpers.h"

#define CONNECTION_URL "mysql://kwetter:kwetter@localhost/kwetter"
#define MAX_MESSAGE_SIZE 140

struct database_connector {
	ConnectionPool_T pool;
	URL_T url;
};

typedef struct database_connector DB_T;

struct session_handle {
	DB_T *db;
	void *socket;
};

typedef struct session_handle KW_T;

// commands
int handle_reg(KW_T *, json_object *);
int handle_unreg(KW_T *, json_object *);
int handle_rereg(KW_T *, json_object *);
int handle_info(KW_T *, json_object *);
int handle_follow(KW_T *, json_object *);
int handle_unfollow(KW_T *, json_object *);
int handle_post(KW_T *, json_object *);
int handle_search(KW_T *, json_object *);
int handle_timeline(KW_T *, json_object *);

#endif
