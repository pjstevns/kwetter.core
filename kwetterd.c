
#include <zmq.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zdb.h>

#define CONNECTION_URL "mysql://kwetter:kwetter@localhost/kwetter"

struct database_connector {
	ConnectionPool_T pool;
	URL_T url;
};

typedef struct database_connector DB_T;

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

int handle(void *socket, const char *in)
{
	printf("received message: %s\n", in);
	s_send(socket, "OK");
	return 0;
}

void server_start(DB_T *db)
{
	// bind REP socket to tcp://*:5555
	// listen to queries and send back replies
	void *context = zmq_init(1);
	void *socket = zmq_socket(context, ZMQ_REP);
	zmq_bind(socket, "tcp://*:5555");
	char *in;

	while (1) {
		in = s_recv(socket);
		handle(socket, in);
		free(in);
	}
	zmq_close(socket);
	zmq_term(context);
}

int database_connect(DB_T *database)
{
	database->url = URL_new(CONNECTION_URL);
	assert(database->url);
	database->pool = ConnectionPool_new(database->url);
	assert(database->pool);
	Connection_T conn = ConnectionPool_getConnection(database->pool);
	assert(conn);
	Connection_close(conn);
	return 0;
}

int main()
{
	DB_T db;
	database_connect(&db);
	server_start(&db);
	return 0;
}


