
set(pri_req UserBle UserJson)
file(GLOB_RECURSE SOURCES *.c *cpp)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES ${pri_req})
