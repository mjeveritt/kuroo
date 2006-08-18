INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(sqlite_STAT_SRCS
alter.c
attach.c
auth.c
btree.c
build.c
callback.c
date.c
delete.c
expr.c
func.c
hash.c
insert.c
legacy.c
main.c
opcodes.c
os_unix.c
os_win.c
pager.c
parse.c
pragma.c
prepare.c
printf.c
random.c
select.c
shell.c
table.c
tokenize.c
trigger.c
update.c
utf.c
util.c
vacuum.c
vdbe.c
vdbeapi.c
vdbeaux.c
vdbemem.c
where.c
)

ADD_LIBRARY(sqlite STATIC ${sqlite_STAT_SRCS})
