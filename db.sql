
create table avatar (
	handle varchar(32) not null,
	fullname varchar(128) not null,
	primary key (handle)
) engine=innodb default charset=utf8;

create table follow (
	lhandle varchar(32) not null,
	rhandle varchar(32) not null,
	since datetime not null,
	unique key following (lhandle, rhandle),
	key since_index (since),
	constraint `follow_fk_1` foreign key (lhandle) references avatar(handle) on delete cascade on update cascade
) engine=innodb default charset=utf8;
	
create table message (
	owner varchar(32) not null,
	message varchar(180) not null,
	created datetime not null,
	key owner_index (owner),
	key message_index (message),
	constraint `message_fk_1` foreign key (owner) references avatar(handle) on delete cascade on update cascade
) engine=innodb default charset=utf8;
