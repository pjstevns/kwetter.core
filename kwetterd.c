
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <json.h>
#include <malloc.h>

#include "kwetter.h"


int handle(KW_T *K, const char *in)
{
	json_object *obj, *cmd;

	obj = json_tokener_parse(in);

	cmd = json_object_object_get(obj, "command");
	
	if (qmatch(cmd,"reg"))
		handle_reg(K, obj);
	else if (qmatch(cmd, "unreg"))
		handle_unreg(K, obj);
	else if (qmatch(cmd, "rereg"))
		handle_rereg(K, obj);
	else if (qmatch(cmd, "info"))
		handle_info(K, obj);
	else if (qmatch(cmd, "follow"))
		handle_follow(K, obj);
	else if (qmatch(cmd, "unfollow"))
		handle_unfollow(K, obj);
	else if (qmatch(cmd, "post"))
		handle_post(K, obj);
	else if (qmatch(cmd, "search"))
		handle_search(K, obj);
	else if (qmatch(cmd, "timeline"))
		handle_timeline(K, obj);

	// cleanup
	json_object_put(cmd);
	json_object_put(obj);

	// done
	return 0;
}

void server_start(KW_T *K)
{
	// bind REP socket to tcp://*:5555
	// listen to queries and send back replies
	char *in;
	void *context = zmq_init(1);
	void *socket = zmq_socket(context, ZMQ_REP);
	zmq_bind(socket, "tcp://*:5555");
	K->socket = socket;

	while (1) {
		in = s_recv(K->socket);
		if (in) {
			handle(K, in);
			free(in);
		} else {
			s_send(K->socket, "ERR");
		}
	}
	zmq_close(socket);
	zmq_term(context);
}

int database_connect(KW_T *K)
{
	K->db->url = URL_new(CONNECTION_URL);
	assert(K->db->url);
	K->db->pool = ConnectionPool_new(K->db->url);
	assert(K->db->pool);
	Connection_T conn = ConnectionPool_getConnection(K->db->pool);
	assert(conn);
	Connection_close(conn);
	return 0;
}

int main()
{
	KW_T K;
	database_connect(&K);
	server_start(&K);
	return 0;
}


