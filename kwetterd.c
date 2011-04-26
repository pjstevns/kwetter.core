
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <json.h>
#include <malloc.h>

#include "kwetter.h"

void config_read(KW_T *K, const char * filename)
{
	int fd, result;
	char data[2048];
	char buf = 0;
	int off = 0, inquote=0, incomment=0;
       	if ((fd = open(filename, O_RDONLY)) < 0) {
		perror("opening config file failed");
		_exit(1);
	}
	memset(data,0,sizeof(data));
	while (off < sizeof(data)) {
		if ( (result = read(fd, &buf, sizeof(buf))) < 0) {
			perror("reading config file failed");
			_exit(1);
		}
		if (result == 0) break;

		// strip comments
		if (buf == '\n') {
			incomment=0;
			continue;
		} else if (incomment == 1) {
			continue;
		} else if (buf == '"' && inquote == 0) {
			inquote=1;
		} else if (buf == '"' && inquote == 1) {
			inquote=0;
		} else if (buf == '#' && inquote == 0) {
			incomment=1;
			continue;
		} else if ((inquote == 0) && (buf == '\t' || buf == ' ')) {
			continue;
		}

		data[off++] = buf;
	}

	//printf("data:[%s]\n", data);
	K->config = json_tokener_parse(data);
}

int handle(KW_T *K, const char *in)
{
	json_object *obj, *cmd;

	printf("in: [%s]\n", in);
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
	json_object *channel, *command;
	const char *cmdchan;

	channel = json_object_object_get(K->config, "channel");
	command = json_object_object_get(channel, "command");
	cmdchan = json_object_get_string(command);

	printf("channel[command]: %s\n", cmdchan);

	zmq_bind(socket, cmdchan);
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
	json_object *database = json_object_object_get(K->config, "database");
	json_object *url = json_object_object_get(database, "url");
	const char *dburl = json_object_get_string(url);
	if (! Connection_isSupported(dburl)) {
		printf("database not supported: %s\n", dburl);
		_exit(1);
	}

	printf("database[url]: %s\n", dburl);

	K->db = (DB_T *)malloc(sizeof(DB_T));

	K->db->url = URL_new(dburl);
	assert(K->db->url);
	K->db->pool = ConnectionPool_new(K->db->url);
	assert(K->db->pool);
	Connection_T conn = ConnectionPool_getConnection(K->db->pool);
	assert(conn);
	Connection_close(conn);
	return 0;
}

int main(int nargs, char **argv)
{
	KW_T *K = (KW_T *)malloc(sizeof (KW_T));

	if (nargs == 2)
		config_read(K, argv[1]);
	else {
		printf("missing config file: kwetterd <config-file>\n");
		_exit(1);
	}

	database_connect(K);
	server_start(K);
	free(K->db);
	free(K);

	return 0;
}


