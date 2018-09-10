#include <string>
#include <vector>
#include <curl/curl.h>
#include <boost/regex.hpp>
//#include <string_regex.hpp>
#include <boost/algorithm/string.hpp>
//ptr:libcurl接受的响应html页面的存储缓冲区
//本函数的功能，就是把ptr里的数据拷贝到用户自己准备的缓冲区里
using namespace std;
size_t WriteData(char *ptr,size_t size,size_t nmemb,void *userdata)
{
	string *output=(string *)userdata;
	output->append(ptr,size*nmemb);
	return size*nmemb;
}

class ScopedHandler{
public:
	ScopedHandler(CURL* h):handler(h){}
	~ScopedHandler(){
		curl_easy_cleanup(handler);
	}
private:
	CURL* handler;
};

bool OpenPage(const string&url,string *html){
	//初始化句柄
	CURL* handler=curl_easy_init();
	ScopedHandler scopedhandler(handler);
	//构造HTTP请求，最核心的是设置url
	//HTTP请求中的其他的header等libcurl也是支持的
	curl_easy_setopt(handler,CURLOPT_URL,url.c_str());

	//设置响应应该如何处理
	curl_easy_setopt(handler,CURLOPT_WRITEFUNCTION,WriteData);
	curl_easy_setopt(handler,CURLOPT_WRITEDATA,html);
	//发送请求
	CURLcode ret=curl_easy_perform(handler);
	if(ret!=CURLE_OK){
		fprintf(stderr,"curl_easy_perfrom failed!\n");
		return false;
	}
	return true;
}

//2.把网页中的章节url解析出来
//完全就是字符串解析的过程
//借助正则表达式——>基于boost（在这里借助这个库）
//C++11里面有正则表达式
//

void ParseMainPage(const string &html,vector<string>*url_list){
	boost::regex reg("/read/\\S+html");
	//借助reg对象查找
	string::const_iterator cur=html.begin();
	string::const_iterator end=html.end();
	boost::smatch result;			//保存筛选结果的变量
	while(boost::regex_search(cur,end,result,reg)){
		//result[0]对象boost中内置的对象，可以隐式转换成string
		//从这个对象中渠道second属性，就对应了接下来要查找的位置信息
		url_list->push_back("http://www.shengxu6.com"+result[0]);
		cur=result[0].second;
	}
}
//
////3.获取到小说的详情页面：这一步和第一步完全相同
////4.解析小说详情页html，获取到章节详细内容
void ParseDetailPage(const string&html,string*content){
//基于字符串查找的方式来进行解析
//主要是基于正则表达式不是很方便
//找到正文的开始位置和结束位置，然后去取字符串字串即可
	string beg_flag="<div class=\"panel-body content-body content-ext\">";
	size_t beg=html.find(beg_flag);
	if(beg==string::npos){
		fprintf(stderr,"找不到开始标记!\n");
		return ;
	}
	beg+=beg_flag.size();

	string end_flag="<script>_drgd200();</script>";
	size_t end=html.find(end_flag);
	if(end==string::npos){
			fprintf(stderr,"找不到结束标记!\n");
			return ;
	}
	if(beg>=end){
		fprintf(stderr,"开始结束标志有问题！beg = %lu，end = %lu\n",beg,end);
		return ;
	}
	*content=html.substr(beg,end-beg);
	//替换掉转义字符
	boost::algorithm::replace_all(*content,"&nbsp;"," ");
	boost::algorithm::replace_all(*content,"<br />","\n");
}
//实现一个整体的入口函数将所有的流程穿起来
void Run(){
	//1.获取到主页的html
	string html;
	OpenPage("http://www.shengxu6.com/book/1.html",&html);
	vector<string> url_list;
	ParseMainPage(html,&url_list);
	for(size_t i=0;i<url_list.size();++i){
		fprintf(stderr,"%s\n",url_list[i].c_str());
		OpenPage(url_list[i],&html);
		string content;
		ParseDetailPage(html,&content);
		printf("%s\n",content.c_str());
	}
}
//解析出所有章节的url列表
//遍历url列表，一次获取到每个章节的html
//解析每个章节的正文内容
//
int main()
{
	Run();
}
