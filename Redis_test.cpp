#include "RedisClient.h"
#include <time.h>"

int main(int argc, char const *argv[])
{
	CRedisClient test;
	if(test.InitConnection("127.0.0.1", 6795))
	{
		cout << "redis connect error:" << std::endl;
		return -1;
	}
	int a=21598725
	string s;
	s.assign((char*)&a,sizeof(a));
	s[0]=' '; s[1]='\n';s[2]='*';s[3]='$';
	if(test.Set("Set %b %b", s.data(),s.size(),s.data(),s.size() ))
	{
		cout << "set reply error" << endl;
		//return -2;
	}	
    test.Expire("Expire %b %d", s.data(),s.size(),16577);
    unsigned int nExpire;
    test.Ttl(nExpire, "TTL %b", s.data(), s.size());
    cout << "expire time:" << nExpire <<endl; 

	if(test.Get(s, "Get %d", a ))
	{
		cout << "get reply error" << endl;
		//return -2;
	}
	cout << "reply:" << s << "size:" << s.size() << endl;

	if(test.Set("Set %b %b", s.data(),s.size(),s.data(),s.size() ))
	{
		cout << "set reply error" << endl;
		//return -2;
	}	

	//sleep(10);
	if(test.Get(s, "Get %b", s.data(),s.size() ))
	{
		cout << "get reply error" << endl;
		//return -2;
	}
	cout << "reply:" << s << "size:" << s.size() << endl;
    return 0;
}
