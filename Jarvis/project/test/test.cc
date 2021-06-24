#include<iostream>
#include<json/json.h>
#include<sstream>
#include<memory>
#include<string>

using namespace std;


int main()
{



    std::string str =  "{\"Age\" : 26,\"Lang\" : \"c++\",\"Name\" : \"zhangsan\"}";
    cout << str << endl;

    JSONCPP_STRING errs;
    Json::Value root;
    Json::CharReaderBuilder rb;
    std::unique_ptr<Json::CharReader> const jsonReader(rb.newCharReader());

    bool res = jsonReader->parse(str.data(), str.data() + str.size(), &root, &errs);
    if(!res || !errs.empty())
    {
        cout << "json parse error" << endl;
        return 1;
    }
    cout << root["Age"].asInt() << endl;
    cout << root["Lang"].asString() << endl;
    cout << root["Name"].asString() << endl;

    /*Json::Value root;
    Json::StreamWriterBuilder wb;
    std::ostringstream os;


    root["name"] = "zhangsan";
    root["age"] = 19;
    root["lang"] = "chinese";

    std::unique_ptr<Json::StreamWriter> jw(wb.newStreamWriter());

    jw->write(root, &os);//os就是保存的序列化之后的结果

    std::string result = os.str();

    std:: cout << result << std::endl;
    return 0;*/
}
