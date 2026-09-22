-- Rung 1 of spec/14-target-ladder.md, read by the sqlite shell inside the distribution image.
-- The first three answers are the ones tamnd/rucc's own rung 1 check asks for, so a pass here and
-- a pass there mean the same thing. The rest are the places the library reaches into libc: the
-- date functions are strftime and a 64-bit time_t, printf is the library's own and is here because
-- the answer has to be the same everywhere, randomblob goes to the kernel for its bytes, and the
-- database is a file on disk rather than in memory so that the VFS opens, locks, writes and stats
-- something real.
.open rung1.db
.mode list
.separator |
.headers off
create table t(a integer primary key, b text, c real);
insert into t values (1, 'one', 1.5), (2, 'two', 2.5), (3, 'three', 3.5);
select count(*) || '|' || sum(a) || '|' || group_concat(b) from t;
select round(sum(c), 2) from t;
select b from t where a = 2;
select printf('%d %s %.3f', 42, 'x', 1.5);
select date(1000000000, 'unixepoch') || ' ' || time(1000000000, 'unixepoch');
select length(hex(randomblob(16)));
select case when (select file from pragma_database_list where name = 'main') like '%rung1.db'
	then 'on disk' else 'in memory' end;
pragma integrity_check;
select sqlite_version();
