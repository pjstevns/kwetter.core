
#include <zdb.h>
#include <json.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "kwetter.h"

#define C Connection_T
#define S PreparedStatement_T
#define R ResultSet_T

#define REG_QUERY         "INSERT INTO avatar (handle,fullname) VALUES (?,?)"
#define UNREG_QUERY       "DELETE FROM avatar WHERE handle = ?"
#define REREG_QUERY       "UPDATE avatar SET handle=?,fullname=? WHERE handle=?"
#define INFO_REG_QUERY    "SELECT fullname FROM avatar WHERE handle=?"
#define INFO_FOLLOW_QUERY "SELECT rhandle FROM follow WHERE lhandle=?"
#define FOLLOW_QUERY      "INSERT INTO follow (lhandle,rhandle,since) VALUES (?,?,NOW())"
#define UNFOLLOW_QUERY    "DELETE FROM follow WHERE lhandle = ? AND rhandle = ?"
#define POST_QUERY        "INSERT INTO message (owner,message,created) VALUES (?,?,NOW())"
#define SEARCH_QUERY      "SELECT owner,message,created FROM message " \
			  "WHERE message LIKE ? AND created >= ? " \
			  "ORDER BY created DESC LIMIT ?"
#define TIMELINE_QUERY    "SELECT owner,message,created " \
       			  "FROM message m JOIN follow f ON m.owner=f.rhandle " \
			  "WHERE f.lhandle = ? AND m.created >= ? and m.created >= f.since " \
			  "UNION SELECT owner,message,created FROM message m WHERE m.owner=? AND m.created >= ? " \
			  "ORDER BY created DESC"

int handle_reg(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar, *fullname;

	avatar = json_object_object_get(in, "avatar");
	fullname = json_object_object_get(in, "fullname");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, REG_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		PreparedStatement_setString(s, 2, json_object_get_string(fullname));
		PreparedStatement_execute(s);
		Connection_commit(c);
		result = 1;
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result)
		s_send(K->socket, "OK");
	else
		s_send(K->socket, "NO");

	json_object_put(avatar);
	json_object_put(fullname);

	return result;
}

int handle_unreg(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar;

	avatar = json_object_object_get(in, "avatar");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, UNREG_QUERY);
		assert(s);
		PreparedStatement_setString(s,1, json_object_get_string(avatar));
		PreparedStatement_execute(s);
		Connection_commit(c);
		result = 1;
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	json_object_put(avatar);

	if (result)
		s_send(K->socket, "OK");
	else
		s_send(K->socket, "NO");

	return result;
}

int handle_rereg(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar, *newavatar, *newfullname;

	avatar = json_object_object_get(in, "avatar");
	newavatar = json_object_object_get(in, "newavatar");
	newfullname = json_object_object_get(in, "newfullname");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, REREG_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(newavatar));
		PreparedStatement_setString(s, 2, json_object_get_string(newfullname));
		PreparedStatement_setString(s, 3, json_object_get_string(avatar));
		PreparedStatement_execute(s);
		Connection_commit(c);
		result = 1;
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result)
		s_send(K->socket, "OK");
	else
		s_send(K->socket, "NO");

	return result;
}

int handle_info(KW_T *K, json_object *in)
{
	C c; S s; R r;
	json_object *avatar, *fullname = NULL, *follows=NULL;
	json_object *output;

	avatar = json_object_object_get(in, "avatar");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, INFO_REG_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		r = PreparedStatement_executeQuery(s);
		if (r && ResultSet_next(r)) {
			fullname = json_object_new_string(ResultSet_getString(r, 1));
		}

		s = Connection_prepareStatement(c, INFO_FOLLOW_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		r = PreparedStatement_executeQuery(s);
		while (r && ResultSet_next(r)) {
			if (! follows) follows = json_object_new_array();
			json_object_array_add(follows, json_object_new_string(ResultSet_getString(r, 1)));
		}

		Connection_commit(c);
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (fullname) {
		output = json_object_new_object();
		json_object_object_add(output, "avatar", avatar);
		json_object_object_add(output, "fullname", fullname);
		if (follows) {
			json_object_object_add(output, "follows", follows);
		}
		s_send(K->socket, json_object_to_json_string(output));
		if (follows) json_object_put(follows);
		json_object_put(output);
		json_object_put(fullname);
	} else {
		s_send(K->socket, "NO");
	}

	json_object_put(avatar);

	return 0;
}


int handle_follow(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar, *follow;

	avatar = json_object_object_get(in, "avatar");
	follow = json_object_object_get(in, "follow");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, FOLLOW_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		PreparedStatement_setString(s, 2, json_object_get_string(follow));
		PreparedStatement_execute(s);
		Connection_commit(c);
		result = 1;
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result)
		s_send(K->socket, "OK");
	else
		s_send(K->socket, "NO");

	return result;
}

int handle_unfollow(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar, *follow;

	avatar = json_object_object_get(in, "avatar");
	follow = json_object_object_get(in, "follow");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, UNFOLLOW_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		PreparedStatement_setString(s, 2, json_object_get_string(follow));
		PreparedStatement_execute(s);
		Connection_commit(c);
		result = 1;
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result)
		s_send(K->socket, "OK");
	else
		s_send(K->socket, "NO");

	return result;
}

int handle_post(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar, *message;

	avatar = json_object_object_get(in, "avatar");
	message = json_object_object_get(in, "message");

	if (strlen(json_object_get_string(message)) > MAX_MESSAGE_SIZE) {
		s_send(K->socket, "NO");
		return result;
	}

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, POST_QUERY);
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		PreparedStatement_setString(s, 2, json_object_get_string(message));
		PreparedStatement_execute(s);
		Connection_commit(c);
		result = 1;
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result)
		s_send(K->socket, "OK");
	else
		s_send(K->socket, "NO");

	return result;
}

int handle_search(KW_T *K, json_object *in)
{
	C c; S s; R r;
	json_object *avatar, *string = NULL, *since=NULL, *limit=NULL;
	json_object *result = NULL;
	char *match;

	avatar = json_object_object_get(in, "avatar");
	string = json_object_object_get(in, "string");
	since = json_object_object_get(in, "since");
	limit = json_object_object_get(in, "limit");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, SEARCH_QUERY);
		assert(s);
		match = (char *)malloc(sizeof(char) * (strlen(json_object_get_string(string)) + 3));
		sprintf(match, "%%%s%%", json_object_get_string(string));
		PreparedStatement_setString(s, 1, match);
		PreparedStatement_setString(s, 2, json_object_get_string(since));
		PreparedStatement_setInt(s, 3, json_object_get_int(limit));

		r = PreparedStatement_executeQuery(s);
		free(match);
		while (r && ResultSet_next(r)) {
			json_object *row = json_object_new_array();
			if (! result) result = json_object_new_array();

			json_object_array_add(row, json_object_new_string(ResultSet_getString(r, 1)));
			json_object_array_add(row, json_object_new_string(ResultSet_getString(r, 2)));
			json_object_array_add(row, json_object_new_string(ResultSet_getString(r, 3)));
			
			json_object_array_add(result, row);
		}

		Connection_commit(c);
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result) {
		json_object *output = json_object_new_object();
		json_object_object_add(output, "avatar", avatar);
		json_object_object_add(output, "string", string);
		json_object_object_add(output, "since", since);
		json_object_object_add(output, "limit", limit);
		json_object_object_add(output, "messages", result);

		s_send(K->socket, json_object_to_json_string(output));
		json_object_put(output);
	} else {
		s_send(K->socket, "NO");
	}

	return 0;
}

int handle_timeline(KW_T *K, json_object *in)
{
	C c; S s; R r;
	json_object *avatar, *since=NULL;
	json_object *result = NULL;

	avatar = json_object_object_get(in, "avatar");
	since = json_object_object_get(in, "since");
	
	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, TIMELINE_QUERY);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		PreparedStatement_setString(s, 2, json_object_get_string(since));
		PreparedStatement_setString(s, 3, json_object_get_string(avatar));
		PreparedStatement_setString(s, 4, json_object_get_string(since));

		r = PreparedStatement_executeQuery(s);
		result = json_object_new_array();
		while (r && ResultSet_next(r)) {
			json_object *row = json_object_new_array();
			json_object_array_add(row, json_object_new_string(ResultSet_getString(r, 1)));
			json_object_array_add(row, json_object_new_string(ResultSet_getString(r, 2)));
			json_object_array_add(row, json_object_new_string(ResultSet_getString(r, 3)));
			json_object_array_add(result, row);
		}
		Connection_commit(c);
	CATCH(SQLException)
		Connection_rollback(c);
	FINALLY
		Connection_close(c);
	END_TRY;

	if (result) {
		json_object *output = json_object_new_object();
		json_object_object_add(output, "avatar", avatar);
		json_object_object_add(output, "since", since);
		json_object_object_add(output, "messages", result);
		s_send(K->socket, json_object_to_json_string(output));
		json_object_put(output);
	} else {
		s_send(K->socket, "NO");
	}

	return 0;
}

