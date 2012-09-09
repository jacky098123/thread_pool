#ifndef __KXUTIL3_SYS_CONF_V2_H__
#define __KXUTIL3_SYS_CONF_V2_H__

#include <string>
#include <vector>
#include <map>

using namespace std;
class Config {

#define SYS_CONFIG_STRIPSTR        " \t\r\n"
  public:
    int LoadFile(const string& conf_file, const string &trim_chars="");

    int GetConfInt(const string &section, const string &name, int* value, int def=-1);
    int GetConfStr(const string &section, const string &name, string &value, const string &def="");
    int GetSections(vector<string> &sections);
    void DumpInfo();

    int WriteConfStr(const string &section, const string &name, const string &value);
    int WriteConfInt(const string &section, const string &name, int value);
    int WriteFile();

    static Config* Instance();

  protected:
    Config() : conf_file_("") { section_map_.clear(); }
    Config(const Config&) {}
    void operator=(const Config&) {}
    string TrimChar(const string &buf);

  protected:
    map<string, map<string,string> > section_map_;
    string                           conf_file_;
    static Config*                instance_;
    static string                    trim_chars_;
};

#endif // __KXUTIL3_SYS_CONF_V2_H__
