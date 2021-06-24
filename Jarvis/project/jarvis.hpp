#ifndef _JARVIS_HPP_
#define _JARVIS_HPP_


#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<json/json.h>
#include<memory>
#include"speech/speech.h"
#include"speech/base/http.h"
#include<stdio.h>
#include<map>
#include<unordered_map>
#include"log.hpp"
#define VOICE_PATH "voice"
#define SPEECH_ASR "asr.wav"
#define SPEECH_TTL "ttl.mp3"
#define CMD_ETC "command.etc"
#define SIZE 1024
using namespace std;



class TuLing
{

    public:
      TuLing()
      {}
      ~TuLing()
      {}

      string ResponsePickup(std::string &str)
      {
          JSONCPP_STRING errs;
          Json::Value root;
          Json::CharReaderBuilder rb;
          std::unique_ptr<Json::CharReader> const jsonReader(rb.newCharReader());
          bool res = jsonReader->parse(str.data(), str.data() + str.size(), &root, &errs);
          if(!res || !errs.empty())
          {
              LOG(Warning, "jsoncpp parse error!");
              return errs;
          }

          Json::Value results = root["results"];
          Json::Value values = results[0]["values"];
          return values["text"].asString();
      }

      string Chat(string message)
      {
        //序列化过程
        Json::Value root;
        root["reqType"] = 0;

        Json::Value inputText;
        Json::Value text;
        text["text"] = message;
        inputText["inputText"] = text;

        root["perception"] = inputText;

        Json::Value user;
        user["apiKey"] = apiKey;
        user["userID"] = userID;

        root["userInfo"] = user;

        Json::StreamWriterBuilder wb;
        std::ostringstream os;
        std::unique_ptr<Json::StreamWriter> jsonwriter(wb.newStreamWriter());
        jsonwriter->write(root, &os);

        std::string body = os.str(); //有了json串，接下来进行http请求
        
        //进行http请求(调的是百度语音识别库里的http.h)
        string response;
        int code = client.post(url, nullptr, body, nullptr, &response);
        if(code != CURLcode::CURLE_OK)
        {
            LOG(Warning, "http request error!");
            return "";
        }
        return ResponsePickup(response);
        //cout << "response#" << response << endl;
      }
    private:
      string apiKey = "27be85301eb14e4fa6cb63023884de7c";
      string userID = "1";
      string url = "http://openapi.tuling123.com/openapi/api/v2";
      aip::HttpClient client;
};


class Jarvis 
{
    public:
        Jarvis():client(nullptr)
        {
    
        }
        
        //配置文件加载，去读配置文件
        void LoadCommandEtc()
        {
            LOG(Normal, "commadn etc load begin!");
            string name = CMD_ETC;
            ifstream in(name);
            if(in.is_open())
            {
                LOG(Warning, "Load command etc error");
                exit(1);
            }
            //走到这里说明打开成功
            char line[SIZE];
            string sep = " : ";
            while(in.getline(line, sizeof(line)))
            {
                string str = line;
                size_t pos = str.find(sep);
                if(pos == string::npos)
                {
                    LOG(Warning, "command etc format error");
                    break;
                }
                string key = str.substr(0, pos);
                string value = str.substr(pos + sep.size());
                key += " 。";//为了语音识别能够对比成功

                record_set.insert({key, value});
            }
            in.close();
            LOG(Normal, "command etc load success!");
        }

        void Init()
        {
            client = new aip::Speech(appid, apikey, secretkey);
            LoadCommandEtc();
            record = "arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
            record += VOICE_PATH;
            record += "/";
            record += SPEECH_ASR;
            record += ">/dev/null 2>&1";
            play = "cvlc --play--and-exit";
            play += VOICE_PATH;
            play += "/";
            play += SPEECH_TTL;
            play += ">/dev/null/ 2>&1"; 
        }


        bool Exec(string command, bool is_print)
        {
            FILE *fp = popen(command.c_str(), "r");
          if(nullptr == fp)
          {
              cout << "popen error" << endl;
              return false;
          }

          
          if(is_print)
          {
              char c;
              size_t s = 0;
              while((s = fread(&c, 1, 1, fp)) > 0)
              {
                  cout << c;
              }
          }


          pclose(fp);
          return true;
        }


        string RecognizePickup(Json::Value &root)
        {
              int err_no = root["err_no"].asInt(); 
              if(err_no != 0)
              {
                    cout << root["err_msg"] << " : " << err_no << endl;
                    return "unknown";
              }
              return root["result"][0].asString();
        }
        //语音识别
        string ASR(aip::Speech *client)
        {
              string asr_file = VOICE_PATH;
              asr_file += "/";
              asr_file += SPEECH_ASR;

              //string message;
              std::map<string, string> options;
              string file_content;
              aip::get_file_content(asr_file.c_str(), &file_content);

              Json::Value root = client->recognize(file_content, "wav", 16000, options); 
              return RecognizePickup(root);
        }

        //语音合成
        bool TTL(aip::Speech *client, std::string &str)
        {
            ofstrean ofile;
            string ttl = VOICE_PATH;
            ttl += "/";
            ttl += SPEECH_TTL;
            ofile.open(ttl.c_str(), ios::out | ios::binary);

            string file_ret;
            map<string, string> options;

            options["spd"] = "5";
            options["per"] = "4";
            options["vol"] = "8";

            Json::Value result = client->text2audio(str, options, file_ret);
            if(!file_ret.empty())
            {
                ofile << file_ret;
            }
            else 
            {
                cout << result.toStyledString() << endl;
            }
            ofile.close();
        }
        
        //判断是不是命令
        bool IsCommand(std::string &message)
        {
            return record_set.find(message) != record_set.end() ? true : false;
        }

        void Run()
        {
            string record = "arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE";
            record += VOICE_PATH;
            record += "/";
            record += SPEECH_ASR;
            record += ">/dev/null 2>&1";
            for(;;)
            { 
                LOG(Normal, "....................讲话中");
                fflush(stdout);
                if(Exec(record, false))
                {
                    //如果录音成功进行语音识别
                    LOG(Normal,"....................识别中");
                    string message = ASR(client);
                    cout << endl;
                    
                    LOG(normal, message);

                    cout << "debug#" << message << endl;
                    if(IsCommand(message))
                    {
                        //是命令
                        LOG(Normal, "Exec a command!");
                        Exec(record_set[message], true);
                        continue;
                    }
                    else 
                    {
                        //不是命令
                        LOG(Normal, "run a normal chat!");
                        //cout << "我#" << message << endl;
                        string echo = tl.Chat(message);
                        LOG(client, echo);
                        //cout << "贾维斯#" << echo << endl;
                        TTL(client, echo);
                        Exec(play, false); 
                        //text转mp3 
                    }
                }
            }   
        }

    private:
        TuLing tl;
        aip::Speech *client;
        string appid = "24427776";
        string apikey = "CGuBbYmKAil3PfQrTzG7zfEu";
        string secretkey = "6YBlNTpjIWuUAzrLmHPTjpYEoXWghbFM";
        unordered_map<string,string> record_set;//贾维斯可识别的命令都存在这里
        string record;
        string play;
};

#endif 
