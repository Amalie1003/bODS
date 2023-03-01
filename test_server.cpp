#define GOOGLE_GLOG_DLL_DECL
#include "orion/server.hpp"
#include "orion/precompute.hpp"
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
		FLAGS_alsologtostderr = false;
		// FLAGS_colorlogtostderr = true;
	#endif
	FLAGS_log_dir = "log";

	google::InitGoogleLogging("server");                // 全局初始化glog，argv[0]是程序名
    google::SetStderrLogging(google::GLOG_INFO);       // 设置glog的输出级别，这里的含义是输出INFO级别以上的信息
    google::SetLogDestination(google::GLOG_INFO, "log/info_");
    google::SetLogDestination(google::GLOG_WARNING, "log/warn_");
    google::SetLogDestination(google::GLOG_ERROR, "log/errr_");

    std::vector<int> params = param_bODS(10);
    P1 = params[0];
    P2 = params[1];
    C = params[2];
    server *my_s = new server();
    my_s->init();
	
	google::ShutdownGoogleLogging(); 

    return 0;
}
