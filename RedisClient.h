#ifndef __REDIS_CLIENT_H__
#define __REDIS_CLIENT_H__

#include <iostream>
#include <hiredis/hiredis.h>
#include <vector>
#include <map>
#include <string.h>

#define FREE_REPLY_RETURN(x) do{\
	freeReplyObject(m_reply);\
	m_reply = NULL;\
	return x;\
}while(0)

#define VA_COMMAND_REPLY() do{\
		if (m_redis == NULL)\
		{\
			InitConnection(m_strIP, m_nPort, m_strPassword,false);/*[autoconf]*/\
			return -1;\
		}\
		va_list ap;\
		va_start(ap,format);\
		m_reply = reinterpret_cast<redisReply*>(redisvCommand(m_redis,format,ap));\
		va_end(ap);\
		if(m_reply == NULL){\
			InitConnection(m_strIP,m_nPort,m_strPassword,false);/*[autoconf]*/\
			return -1;\
		}\
}while(0)

#define RECORD_NOT_EXIST -100



using namespace std;
class CRedisClient
{
public:
    CRedisClient()
    {
        m_redis=NULL;
        m_reply=NULL;
        m_error=0;
        memset(m_strIP,0,sizeof(m_strIP));
        memset(m_strPassword,0,sizeof(m_strPassword));
        // 启用异步redis
    };
    ~CRedisClient()
    {
        if(m_reply!=NULL)
            freeReplyObject(m_reply);
        if(m_redis!=NULL)
            redisFree(m_redis);
    }

    void Attach(redisContext *redis, const char* ip, short port, const char *password = NULL)
    {
        m_redis = redis;
        strncpy(m_strIP, ip, sizeof(m_strIP)-1);
        if(NULL != password)
        {
            strncpy(m_strPassword, password, sizeof(m_strPassword)-1);
        }
        m_nPort = port;
    }

    void Detach()
    {
        m_redis=NULL;
        m_reply = NULL;
        memset(m_strIP,0,sizeof(m_strIP));
        memset(m_strPassword,0,sizeof(m_strPassword));
        m_nPort = 0;
    }

    bool IsConnect()
    {
        return (m_redis!=NULL);
    }


    //[autoconf]
    int InitConnection(const char* strIP, unsigned short nPort, const char* strPassword = NULL, bool bInit = true)
    {
        if(m_redis != NULL)
        {
            redisFree(m_redis);
            m_redis = NULL;
        }
        memcpy(m_strIP,strIP,strlen(strIP));
        if(NULL != strPassword)
        {
            memcpy(m_strPassword, strPassword, strlen(strPassword));
        }
        m_nPort = nPort;
        if (m_nPort == 0 || m_strIP[0] == 0)
        {
            return -1;
        }

        m_redis = redisConnect(strIP, nPort);

        if (m_redis->err)	
        {
            return -1;
        }
        if(m_strPassword[0] != '\0')    //有密码  验证密码
        {
            if(VerifyIdentity() < 0)    //密码验证
            {
                return -1;
            }
        }
        return 0;
    }

    int VerifyIdentity()
    {
        string strReply;
        const int iMax = 2048;
        char acCmd[iMax];
        sprintf(acCmd, "AUTH %s", m_strPassword);
        int ret =  GetSyn(strReply, acCmd);
        if(ret > 0 && strcmp(strReply.c_str(), "OK")==0)
        {
            return ret;
        }
        return -1;
    }

    // <0 err, =0  NULL,  >0 len 
    int Get(string& strReply, const char* format,...)
    {
        strReply = "";
        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);

        return GetSyn(strReply, acCmd);
    }

    int GetSyn(string& strReply, const char* format, ...)
    {
        strReply = "";
        VA_COMMAND_REPLY();
        if (m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(0);

        if (m_reply->type == REDIS_REPLY_STRING && m_reply->str != NULL)
        {
            strReply.assign(m_reply->str, m_reply->len);
            int len = m_reply->len;
            FREE_REPLY_RETURN(len);
        }
        if (m_reply->type == REDIS_REPLY_STATUS  && m_reply->str != NULL)
        {
            strReply.assign(m_reply->str, m_reply->len);
            int len = m_reply->len;
            FREE_REPLY_RETURN(len);
        }

        if (m_reply != NULL)
        {
        }
            FREE_REPLY_RETURN(-1);
    }

    int GetInteger(int  &replyvalue, const char * format,...)
    {
        replyvalue=0;

        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);

        int iRet = GetIntegerSyn(replyvalue,  acCmd);

        if (iRet < 0)
        {
            if (m_reply)
            {
            }
            else
            {
            }

        }

        return iRet;


        return 0;
    }

    int GetIntegerSyn(int  &replyvalue, const char * format, ...)
    {
        replyvalue = 0;
        VA_COMMAND_REPLY();
        if (m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(0);
        if (m_reply->type == REDIS_REPLY_INTEGER)
        {
            replyvalue = m_reply->integer;
            FREE_REPLY_RETURN(0);
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("GetInteger,type:%d,integer:%d"), m_reply->type, m_reply->integer));
        if (m_reply != NULL)
        {
        }
            FREE_REPLY_RETURN(-1);
    }

    // <0 err,  =0  NULL,  >0 len 
    int Get(char* buf,unsigned int buflen,unsigned int& strlength, const char* format,...)
    {
        memset(buf,0,buflen);

        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);

        return GetSyn(buf, buflen, strlength,  acCmd);

    }

    int GetSyn(char* buf, unsigned int buflen, unsigned int& strlength, const char* format, ...)
    {
        memset(buf, 0, buflen);
        VA_COMMAND_REPLY();
        if (m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(0);

        if (m_reply->type == REDIS_REPLY_STRING && m_reply->str != NULL)
        {
            if (buflen<(unsigned)m_reply->len)
            {
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("get binary reply info, buf len over flow ,buflen=%d,replay_len=%d"), buflen, m_reply->len));
                FREE_REPLY_RETURN(-1);
            }
            memcpy(buf, m_reply->str, m_reply->len);
            strlength = m_reply->len;
            FREE_REPLY_RETURN(strlength);
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("get reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if (m_reply != NULL)
        {
        }
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));

            FREE_REPLY_RETURN(-1);
    }
    //call Set SMEMBERS
    int SGet(vector<string>& vctReply, const char* format, ...)
    {
        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);


        return SGetSyn(vctReply,  acCmd);

    }

    int SGetSyn(vector<string>& vctReply, const char* format, ...)
    {
        VA_COMMAND_REPLY();
        if (m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(0);

        if (m_reply->type == REDIS_REPLY_ARRAY)
        {
            string strTmp;
            for (unsigned i = 0; i<m_reply->elements; i++)
            {
                if ((m_reply->element[i]->type != REDIS_REPLY_NIL) && (m_reply->element[i]->len > 0))
                    strTmp.assign(m_reply->element[i]->str, m_reply->element[i]->len);
                else
                    strTmp = "";

                vctReply.push_back(strTmp);
            }
            int ret = m_reply->elements;
            FREE_REPLY_RETURN(ret);
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("SGet reply info,type:%d,integer:%d"), m_reply->type, m_reply->integer));
        if (m_reply != NULL)
        {}
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));

        FREE_REPLY_RETURN(-1);
    }


    int HGet(string& strReply, const char* format, ...)
    {
        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);

        return Get(strReply, "%s", acCmd);


    }

    int HGetAll(map<string, string> &ret, const char* format, ...)
    {
        ret.clear();

        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);


        return HGetAllSyn(ret,  acCmd);

        return 0;

    }

    int HGetAllSyn(map<string, string> &ret, const char* format, ...)
    {
        ret.clear();

        VA_COMMAND_REPLY();


        if (m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(-1);
        if (m_reply->type == REDIS_REPLY_ARRAY) {
            for (unsigned i = 0; i < m_reply->elements; ++i) {
                char *k = m_reply->element[i]->str;
                ++i;
                char *v = m_reply->element[i]->str;
                ret[k] = v;
            }

        }

        if (m_reply != NULL)
        {

            if (m_reply->type == REDIS_REPLY_ERROR)
            {
            }
        }


        FREE_REPLY_RETURN(0);
    }

    int HMGet(char* buf, int buflen, const char* format, ...)
    {
        memset(buf, 0, buflen);

        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);

        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);


        return HMGetSyn(buf, buflen, acCmd);


    }

    int HMGetSyn(char* buf, int buflen, const char* format, ...)
    {
        memset(buf, 0, buflen);
        VA_COMMAND_REPLY();
        if (m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(0);

        if (m_reply->type == REDIS_REPLY_ARRAY)
        {
            if (buflen<m_reply->len)
            {
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("get binary reply info, buf len over flow ,buflen=%d,replay_len=%d"), buflen, m_reply->len));
                FREE_REPLY_RETURN(-1);
            }
            memcpy(buf, m_reply->str, m_reply->len);
            int len = m_reply->len;
            FREE_REPLY_RETURN(len);  //返回value长度
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("HMGet reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if (m_reply != NULL)
        {
            if (m_reply->type == REDIS_REPLY_ERROR)
            {
            }
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));
            else
            {
            }
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);
    }

    bool SplitKey(char *pcCmd, string &strKey, string &strValue)
    {
        int i = 0;
        int iLen = strlen(pcCmd);
        int iFind = 0;
        while (pcCmd[i] != 0)
        {
            if (pcCmd[i] == ' ')
            {
                iFind = i+1;
                break;
            }
            ++i;
        }


        return true;

    }

    int HMGet(std::vector<string>& vctReply, const char* format, ...)
    {

        const int iMax = 2048;
        char acCmd[iMax];
        va_list ap;
        va_start(ap, format);
        vsnprintf(acCmd, iMax - 1, format, ap);
        va_end(ap);


        return HMGetSyn(vctReply, acCmd);

    }

    int HMGetSyn(std::vector<string>& vctReply, const char* format, ...)
    {

        VA_COMMAND_REPLY();
        if (m_reply->type == REDIS_REPLY_NIL)
        {
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("[%D]HMGet:return value = NULL \n")));
            FREE_REPLY_RETURN(0);
        }

        if (m_reply->type == REDIS_REPLY_ARRAY)
        {
            string strTmp;
            for (unsigned i = 0; i<m_reply->elements; i++)
            {
                if ((m_reply->element[i]->type != REDIS_REPLY_NIL) && (m_reply->element[i]->len > 0))
                    strTmp.assign(m_reply->element[i]->str, m_reply->element[i]->len);
                else
                    strTmp = "";

                vctReply.push_back(strTmp);
            }
            int ret = m_reply->elements;
            FREE_REPLY_RETURN(ret);
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("HMGet reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if (m_reply != NULL)
        {
            if (m_reply->type == REDIS_REPLY_ERROR)
            {
            }
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));
            else
            {
            }
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);

    }



    int Set(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_STATUS && (strncmp(m_reply->str,"OK",2) == 0))
        {
            FREE_REPLY_RETURN(0);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("set reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {
            }
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {
            }
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }
        FREE_REPLY_RETURN(-1);			
    }





    int SADD(const char* format,...)
    {

        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER)
        {
            int ret = m_reply->integer;
            FREE_REPLY_RETURN(ret);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("SADD reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);	
    }

    int SREM(const char* format,...)
    {

        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER)
        {
            int ret = m_reply->integer;
            FREE_REPLY_RETURN(ret);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("SREM reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);	
    }

    int HSet(const char* format,...)
    {

        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER  && (m_reply->integer==0 || m_reply->integer==1) )
        {
            FREE_REPLY_RETURN(0);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("HSet reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);	
    }



    int  HMSET(const char* format,...)
    {

        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER )
        {
            FREE_REPLY_RETURN(0);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("HMSET:type=%d,int=%d"), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }	
        FREE_REPLY_RETURN(-1);	
    }

    int Ttl(unsigned& nExpire, const char* format,...)
    {

        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER)
        {
            nExpire = m_reply->integer;
            FREE_REPLY_RETURN(0);			
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("ttl reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));

        if(m_reply != NULL)
            {}
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));
            FREE_REPLY_RETURN(-1);			
    }


    int Expire(const char* format,...)
    {

        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER && m_reply->integer == 1)
        {
            FREE_REPLY_RETURN(0);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("expire,type=%d,integer=%d"), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
            {}
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d"), m_reply->len));
            FREE_REPLY_RETURN(-1);			
    }



    // no-use
    int Del(const char* format, ...)
    {
        VA_COMMAND_REPLY();

        if (m_reply->type == REDIS_REPLY_INTEGER && (m_reply->integer == 1 || m_reply->integer == 0))
        {
            FREE_REPLY_RETURN(0);
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("del reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if (m_reply != NULL)
        {
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));
        }
        FREE_REPLY_RETURN(-1);
    }

    // no-use
    int Exists(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER && m_reply->integer == 1)
        {
            FREE_REPLY_RETURN(0);
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("Exists reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {}
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));
            FREE_REPLY_RETURN(-1);
    }
    // no-use
    int HExists(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER && m_reply->integer == 1)
        {
            FREE_REPLY_RETURN(0);
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("Exists reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
            {}
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));
            FREE_REPLY_RETURN(-1);
    }
    // no-use
    int HIncrby(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER)
        {
            int ret = m_reply->integer;
            FREE_REPLY_RETURN(ret);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("HIncrby reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);	
    }
    // no-use
    int LLen(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER)
        {
            int ret = m_reply->integer;
            FREE_REPLY_RETURN(ret); 		
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("LLen reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);	
    }
    // no-use
    int Ltrim(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER )
        {
            int ret = m_reply->integer;
            FREE_REPLY_RETURN(ret);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("Ltrim reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));
        }
        FREE_REPLY_RETURN(-1);			
    }
    // no-use
    int LIndex(char* buf,unsigned int buflen,const char* format,...)
    {
        memset(buf,0,buflen);
        VA_COMMAND_REPLY(); 	
        if(m_reply->type == REDIS_REPLY_NIL)
            FREE_REPLY_RETURN(0);

        if(m_reply->type == REDIS_REPLY_STRING && m_reply->str != NULL)
        {
            if(buflen<(unsigned)m_reply->len)
            {
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("get binary reply info, buf len over flow ,buflen=%d,replay_len=%d"), buflen,m_reply->len));
                FREE_REPLY_RETURN(-1);
            }
            memcpy(buf,m_reply->str,m_reply->len);
            int strlength=m_reply->len;
            FREE_REPLY_RETURN(strlength);
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("get reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {}
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d\n"), m_reply->len));

            FREE_REPLY_RETURN(-1);
    }
    // no-use
    int LPush(const char* format,...)
    {
        VA_COMMAND_REPLY();

        if(m_reply->type == REDIS_REPLY_INTEGER)
        {
            FREE_REPLY_RETURN(0);			
        }
        //ACE_DEBUG((LM_ERROR, ACE_TEXT("set reply info, type:%d, integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));	
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }	
        FREE_REPLY_RETURN(-1);			
    }

    // no-use
    int LRange(std::vector<string>& vctReply, const char* format, ...)
    {
        VA_COMMAND_REPLY();
        if(m_reply->type == REDIS_REPLY_NIL)
        {
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("[%D]LRange:return value = NULL \n")));
            FREE_REPLY_RETURN(0);
        }

        if(m_reply->type == REDIS_REPLY_ARRAY)
        {
            string strTmp;
            for (unsigned i=0; i<m_reply->elements; i++)
            {
                if((m_reply->element[i]->type != REDIS_REPLY_NIL)  && (m_reply->element[i]->len > 0))
                    strTmp.assign(m_reply->element[i]->str, m_reply->element[i]->len);
                else
                    strTmp = "";

                vctReply.push_back(strTmp);
            }
            int ret = m_reply->elements;
            //ACE_DEBUG((LM_ERROR, ACE_TEXT("[%D]LRange:return value = %d and ret = %d\n"), m_reply->type,ret));
            FREE_REPLY_RETURN(ret);
        }

        //ACE_DEBUG((LM_ERROR, ACE_TEXT("LRange reply info,type:%d,integer:%d, "), m_reply->type, m_reply->integer));
        if(m_reply != NULL)
        {
            if(m_reply->type == REDIS_REPLY_ERROR)
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("err_code:%s \n"), m_reply->str));
            else
            {}
                //ACE_DEBUG((LM_ERROR, ACE_TEXT("strlen:%d \n"), m_reply->len));
        }

        FREE_REPLY_RETURN(-1);
    }

private:
    char m_strIP[128];
    unsigned short m_nPort;
    char m_strPassword[128];
    redisContext *m_redis;				
    redisReply *m_reply;		
    unsigned int m_error;

};

#if 0
int main()
{
    CRedisClient test;
    if(test.InitConnection("127.0.0.1", 6795))
    {
        cout << "redis connect error:" << std::endl;
        return -1;
    }
    int a=21598725;
    string s;
    s.assign((char*)&a,sizeof(a));
    s[0]=' '; s[1]='\n';s[2]='*';s[3]='$';
    if(test.Set("Set %b %b", s.data(),s.size(),s.data(),s.size() ))
    {
        cout << "set reply error" << endl;
        //return -2;
    }	
    test.Expire("Expire %b %d", s.data(),s.size(),16577);
    sleep(2);
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
#endif
#endif

