
#include <zmq.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zdb.h>
#include <json.h>

#define CONNECTION_URL "mysql://kwetter:kwetter@localhost/kwetter"

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

static char * s_recv (void *socket) 
{
	zmq_msg_t message;
	zmq_msg_init (&message);
	if (zmq_recv (socket, &message, 0))
		return (NULL);
	int size = zmq_msg_size (&message);
	char *string = malloc (size + 1);
	memcpy (string, zmq_msg_data (&message), size);
	zmq_msg_close (&message);
	string [size] = 0;
	return (string);
}

static int s_send (void *socket, const char *string) 
{
	int rc;
	zmq_msg_t message;
	zmq_msg_init_size (&message, strlen (string));
	memcpy (zmq_msg_data (&message), string, strlen (string));
	rc = zmq_send (socket, &message, 0);
	zmq_msg_close (&message);
	return (rc);
}

inline int qmatch(json_object *string, const char *match)
{
	return strncasecmp(json_object_to_json_string(string), match, strlen(match));
}

int handle_reg(void *socket, json_object *in)
{
	return 0;
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
		handle_reg(socket, obj);

	json_object_put(cmd);
	json_object_put(obj);

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


