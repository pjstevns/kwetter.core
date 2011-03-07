
#include <zdb.h>
#include <json.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "kwetter.h"

#define C Connection_T
#define S PreparedStatement_T
#define R ResultSet_T

int handle_reg(KW_T *K, json_object *in)
{
	C c; S s; volatile int result = 0;
	json_object *avatar, *fullname;

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");
	fullname = json_object_object_get(in, "fullname");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "insert into avatar (handle, fullname) values (?,?)");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));
	avatar = json_object_object_get(in, "avatar");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "delete from avatar where handle = ?");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");
	newavatar = json_object_object_get(in, "newavatar");
	newfullname = json_object_object_get(in, "newfullname");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "update avatar set handle=?, fullname=? where handle=?");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "select fullname from avatar where handle=?");
		assert(s);
		PreparedStatement_setString(s, 1, json_object_get_string(avatar));
		r = PreparedStatement_executeQuery(s);
		if (r && ResultSet_next(r)) {
			fullname = json_object_new_string(ResultSet_getString(r, 1));
		}

		s = Connection_prepareStatement(c, "select rhandle from follow where lhandle=?");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");
	follow = json_object_object_get(in, "follow");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "insert into follow (lhandle, rhandle) values (?,?)");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");
	follow = json_object_object_get(in, "follow");

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "delete from follow where lhandle = ? and rhandle = ?");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");
	message = json_object_object_get(in, "message");

	if (strlen(json_object_get_string(message)) > MAX_MESSAGE_SIZE) {
		s_send(K->socket, "NO");
		return result;
	}

	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "insert into message (owner, message, created) values (?,?,%s)", "NOW()");
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

	printf("%s: %s\n", __func__, json_object_to_json_string(in));

	avatar = json_object_object_get(in, "avatar");
	string = json_object_object_get(in, "string");
	since = json_object_object_get(in, "since");
	limit = json_object_object_get(in, "limit");


	c = ConnectionPool_getConnection(K->db->pool);
	TRY
		Connection_beginTransaction(c);
		s = Connection_prepareStatement(c, "select owner, message, created from message "
				"where message like ? and created >= ? order by created desc limit ?");
		assert(s);
		match = (char *)malloc(sizeof(char) * (strlen(json_object_get_string(string) + 3)));
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
		json_object_put(result);
		json_object_put(output);
	} else {
		s_send(K->socket, "NO");
	}

	json_object_put(string);
	json_object_put(since);
	json_object_put(limit);
	json_object_put(avatar);

	return 0;
}

int handle_timeline(KW_T *K, json_object *in)
{
	return 0;
}

