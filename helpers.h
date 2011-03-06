#ifndef _KW_HELPERS_H
#define _KW_HELPERS_H

char * s_recv(void *);
int s_send(void *, const char *);
inline int qmatch(json_object *, const char *);

#endif
