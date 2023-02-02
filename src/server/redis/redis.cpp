#include "redis.hpp"
#include <iostream>
#include <string>

using namespace std;

// Redis的ip和端口号
string Ip = "127.0.0.1";
int Port = 6379;

Redis::Redis() : _publish_context(nullptr),_subscribe_context(nullptr)
{
}

Redis::~Redis(){
    if(_publish_context != nullptr){
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr){
        redisFree(_subscribe_context);
    }
}

//连接redis服务器
bool Redis::connect(){
    _publish_context = redisConnect(Ip.c_str(),Port);
    if(_publish_context == nullptr){
        cerr << "_publish_context connect redis fail!"<<endl;
        return false;
    }
    _subscribe_context = redisConnect(Ip.c_str(),Port);
    if(_subscribe_context == nullptr){
        cerr << "_subscribe_context connect redis fail!"<<endl;
        return false;
    }
    // 创建独立线程：监听通道信息，上报上层
    thread t([&](){
        observer_channel_message();
    });
    t.detach();    //线程分离  

    cout<<"Redis connect success!"<<endl;
    return true;
}

//向redis指定的通道channel发布消息
bool Redis::publish(int channel,string message){
    redisReply* rep = (redisReply*)redisCommand(_publish_context,"PUBLISH %d %s",channel,message.c_str());
    if(rep == nullptr){
        cerr << "Redis publish fail!"<<endl;
        return false;
    }
    freeReplyObject(rep);
    return true;
}

//向redis指定的通道subscrib订阅消息
bool Redis::subscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(_subscribe_context,"SUBSCRIBE %d",channel)){
        cerr << "Redis reidsAppendCommand fail!"<<endl;
        return false;
    }
    int done = 0;
    while(!done){
        // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
        if(REDIS_ERR ==  redisBufferWrite(_subscribe_context,&done)){
            cerr << "Redis redisBufferWrite fail!"<<endl;
            return false;
        }
    }
    
    return true;
}

//向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(_subscribe_context,"UNSUBSCRIBE %d",channel)){
        cerr << "Redis reidsAppendCommand fail!"<<endl;
        return false;
    }
    int done = 0;
    while(!done){
        // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
        if(REDIS_ERR ==  redisBufferWrite(_subscribe_context,&done)){
            cerr << "Redis redisBufferWrite fail!"<<endl;
            return false;
        }
    }
    return true;
}

//在独立线程中接受订阅通道中的消息
void Redis::observer_channel_message(){
    redisReply* reply = nullptr;
    while(REDIS_OK == redisGetReply(this->_subscribe_context,(void**)&reply))
    {
        //订阅收到的消息是一个带三元素的数组（三元组）
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            //给业务层上报通道上发生的消息
            this->_notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        
        freeReplyObject(reply);
    }
    // while不应该退出（要么执行要么阻塞）
    cerr << ">>>>> observer_channel_message quit <<<<<<" << endl;

}

//初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int,string)> fn){
    this->_notify_message_handler = fn;
}