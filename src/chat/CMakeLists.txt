aux_source_directory(../. SRC_LIST)
aux_source_directory(.././model MODEL_LIST)

aux_source_directory(. CHAT_LIST)
add_executable(ChatServer ${SRC_LIST} ${MODEL_LIST} ${CHAT_LIST})
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient)