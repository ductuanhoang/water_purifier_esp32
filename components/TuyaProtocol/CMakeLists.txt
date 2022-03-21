
set(pri_req UserBle UserJson)
file(GLOB_RECURSE SOURCES *.cpp)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES ${pri_req})
