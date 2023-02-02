target = ChatServer
obj = main.cpp ./db/db.cpp chatserver.cpp chatservice.cpp ./model/FriendModel.cpp ./model/GroupModel.cpp ./model/OfflineMsgModel.cpp ./redis/redis.cpp ./model/UserModel.cpp

PATH = ../include

$(target):$(obj)
	g++ $(obj) -o $(target) -lhiredis -lmuduo_net -lmuduo_base -pthread -lmysqlclient -std=c++11

clean:
	rm -r $(target)