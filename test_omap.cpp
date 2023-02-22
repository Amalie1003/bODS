#define GOOGLE_GLOG_DLL_DECL
#include "orion/client.hpp"
#include "glog/logging.h"
#include <iostream>
using namespace std;

#define NDEBUG 0

void logSet()
{
	FLAGS_logtostderr = false;
	FLAGS_log_prefix = true;
	FLAGS_logbufsecs = 0;
	FLAGS_max_log_size = 10;
	FLAGS_stop_logging_if_full_disk = true;
	FLAGS_stderrthreshold = 1;
	FLAGS_minloglevel = 0;
}

int main()
{
	logSet();
	#if NDEBUG
		std::cout << "Build-RELASE" << std::endl;
		FLAGS_alsologtostderr = false;
	#else
		std::cout << "Build-DEBUG" << std::endl;
		//设置日志消息除日志文件之外同时输出到标准输出
		FLAGS_alsologtostderr = true;
		//设置记录到标准输出的颜色消息（如果终端支持）
		FLAGS_colorlogtostderr = true;
	#endif
	FLAGS_log_dir = "log";

	google::InitGoogleLogging("client");                // 全局初始化glog，argv[0]是程序名
    google::SetStderrLogging(google::GLOG_INFO);       // 设置glog的输出级别，这里的含义是输出INFO级别以上的信息
    // 设置INFO级别以上的信息log文件的路径和前缀名
    google::SetLogDestination(google::GLOG_INFO, "log/info_");
 
    // 设置WARNING级别以上的信息log文件的路径和前缀名
    google::SetLogDestination(google::GLOG_WARNING, "log/warn_");
 
    // 设置ERROR级别以上的信息log文件的路径和前缀名
    google::SetLogDestination(google::GLOG_ERROR, "log/errr_");

	bytes<Key> key1{0};
	bytes<Key> key2{1};
    bytes<Key> key3{2};
	LOG(INFO) << "[client]init begin";
	//std::cout<<"init begin"<<std::endl;

	client* my_client = new client(10*4, key1, key2, key3);
	my_client->init();
	LOG(INFO) << "[client]init finished";
	//std::cout<<"init finished!"<<std::endl;

	string mapkey1 = "test1";
	my_client->insert_map(mapkey1, 10);
	LOG(INFO) << "[client]insert 1-----finished";
	//std::cout<<"insert 1"<<std::endl;

	//string mapkey2 = "test2";
	//my_omap->insert(mapkey2, "hello2");
	//std::cout<<"insert 2"<<std::endl;

	string mapkey3 = "test3";
	my_client->insert_map(mapkey3, 30);
	LOG(INFO) << "[client]insert 3-----finished";
	//std::cout<<"insert 3"<<std::endl;

	string mapkey4 = "test2";
	my_client->insert_map(mapkey4, 20);
	LOG(INFO) << "[client]insert 2-----finished";
	//std::cout<<"insert 2"<<std::endl;

	google::ShutdownGoogleLogging();  
	return 0;
}
