aux_source_directory(../. SRC_LIST)

aux_source_directory(. FILE_LIST)
add_executable(FileServer ${SRC_LIST} ${FILE_LIST})
target_link_libraries(FileServer muduo_net muduo_base mysqlclient)