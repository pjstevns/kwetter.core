
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <json.h>
#include <malloc.h>

#include "kwetter.h"

inline int qmatch(json_object *string, const char *match)
{
	return strncasecmp(json_object_to_json_string(string), match, strlen(match));
}

int handle(KW_T *K, const char *in)
{
	void *socket = K->socket;
	json_object *obj, *cmd;

	obj = json_tokener_parse(in);
	printf("received message: %s\n", json_object_to_json_string(obj));

	cmd = json_object_object_get(obj, "command");
	printf("received command: %s\n", json_object_to_json_string(cmd));
	
	if (qmatch(cmd,"reg"))
		handle_reg(K, obj);

	// cleanup
	json_object_put(cmd);
	json_object_put(obj);

	// done
	s_send(socket, "OK");
	return 0;
}

void server_start(KW_T *K)
{
	// bind REP socket to tcp://*:5555
	// listen to queries and send back replies
	void *context = zmq_init(1);
	void *socket = zmq_socket(context, ZMQ_REP);
	zmq_bind(socket, "tcp://*:5555");
	char *in;
	K->socket = socket;

	while (1) {
		in = s_recv(K->socket);
		handle(K, in);
		free(in);
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


