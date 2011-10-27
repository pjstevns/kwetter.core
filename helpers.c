
#include <malloc.h>
#include <string.h>
#include <zmq.h>
#include <json.h>

#include "helpers.h"

char * s_recv (void *socket) 
{
	zmq_msg_t message;
	zmq_msg_init (&message);
	if (zmq_recv (socket, &message, 0))
		return NULL;
	int size = zmq_msg_size (&message);
	char *string = malloc (size + 1);
	memcpy (string, zmq_msg_data (&message), size);
	zmq_msg_close (&message);
	string [size] = 0;
	//printf("in: [%s]\n", string);
	return string;
}

int s_send (void *socket, const char *string) 
{
	int rc;
	zmq_msg_t message;
	//printf("out: [%s]\n", string);
	zmq_msg_init_size (&message, strlen (string));
	memcpy (zmq_msg_data (&message), string, strlen (string));
	rc = zmq_send (socket, &message, 0);
	zmq_msg_close (&message);
	return rc;
}

inline int qmatch(json_object *string, const char *match)
{
	return (string && match && (strncasecmp(json_object_get_string(string), match, strlen(match)) == 0));
}

