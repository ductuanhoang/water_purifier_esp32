
set(pri_req bt)
file(GLOB_RECURSE SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES ${pri_req})
