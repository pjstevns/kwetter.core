
#include <malloc.h>
#include <string.h>
#include <zmq.h>
#include <json.h>

#include "helpers.h"

char * s_recv (void *socket) 
{
	zmq_msg_t message;
	zmq_msg_init (&message);
	if (zmq_recv (socket, &message, 0)) {
		return (NULL);
	}
	int size = zmq_msg_size (&message);
	char *string = malloc (size + 1);
	memcpy (string, zmq_msg_data (&message), size);
	zmq_msg_close (&message);
	string [size] = 0;
	return (string);
}

int s_send (void *socket, const char *string) 
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
	int result;
	result = (strncasecmp(json_object_get_string(string), match, strlen(match)) == 0);
	printf("[%s]<->[%s]:[%d]\n", json_object_get_string(string), match, result);
	return result;
}

