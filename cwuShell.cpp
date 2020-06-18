#include <iostream>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <sys/syscall.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fstream>
#include <queue>
#include <sstream>
#include <set>

using namespace std;

void cpuinfo_manual(){
    cout << "----- cpuInfo help manual: How to use cpuInfo -----\n" <<
         "Type the command cpuInfo followed by one or more of the following tags:\n" <<
         "-c   --->   print the cpu clock of your system.\n" <<
         "-t   --->   print the cpu type on your system.\n" <<
         "-n   --->   print the number of cores that are on your system. \n" <<
         "-h or -help  --->   print the manual for cpuInfo. Or call without an argument.\n\n";
}
void meminfo_manual(){
    cout << "----- memInfo help manual: How to use memInfo -----\n" <<
         "Type the command memInfo followed by one or more of the following tags:\n" <<
         "-t   --->   print the total RAM memory available in your system.\n" <<
         "-u   --->   print the used RAM memory in your system.\n" <<
         "-c   --->   print the size of the L2 cache/core in your system. \n" <<
         "-h or -help  --->   print the manual for memInfo. Or call without an argument.\n\n";
}
void exit_manual(){
    cout << "----- exit command help manual: How to use exit -----\n" <<
    "Type the command 'exit' to exit the shell. The exit command takes a \n" <<
    "single integer argument or no argument at all. If an argument is provided, \n" <<
    "it will be the exit status of the program. If no argument is provided, the exit value \n" <<
    "will be the value returned by the last executed command. \n" <<
    "Add -h or -help as a tag to display the exit manual.\n\n";
}
void prompt_manual(){
    cout << "----- prompt command help manual: How to use prompt -----\n" <<
         "Type the command 'prompt' followed by any string. Spaces are not allowed.\n" <<
         "This string will become the new name of the prompter in this shell.\n" <<
         "If you would like to return to the original prompt, simply type 'prompt' with no argument.\n" <<
         "Add -h or -help as a tag after typing the 'prompt' command to promptly display the prompt manual\n\n";
}
queue<string> parse_input(const string& user_input) {

    queue<string> parse;
    stringstream s(user_input);
    string word;

    while(s >> word) {
        parse.push(word);
    }
    return parse;
}

bool check_num(const string& num) {
    for(char i : num) {
        if(!isdigit(i)) return false;
    }
    return true;
}

//only for exit command
bool validate_argument(queue<string> args) {

    if(args.size() > 1) return false;

    else {

        if(args.front().at(0) == '-') {

            if(args.front().size() == 1) {
                return false;
            }
            string number = args.front().substr(1, args.front().size());
            return check_num(number);

        }
        else {

            string number = args.front().substr(0, args.front().size());
            return check_num(number);
        }
    }
}

bool validate_tags(queue<string> command, set<string> tags) {

    while(!command.empty()) {

        auto itr = tags.find(command.front());

        if(itr == tags.end()) {
            return false;
        }
        command.pop();
    }
    return true;
}

string get_mem_info(string line, ifstream& stream, const string& info_type = "") {

    if(info_type == "total") {

        getline(stream, line);

        queue<string> parsed_line = parse_input(line);
        parsed_line.pop();
        int data = stoi(parsed_line.front());
        data *= 1024; // data in /proc/meminfo actually shows kibibytes instead of kilobyte. 1 kibibyte = 1024 Bytes

        return to_string(data);
    }
    else if(info_type == "used") {

        getline(stream, line);

        queue<string> parsed_line = parse_input(line);
        parsed_line.pop();
        int mem_total = stoi(parsed_line.front());

        getline(stream, line);
        parsed_line = parse_input(line);
        parsed_line.pop();
        int mem_free = stoi(parsed_line.front());

        int mem_used = mem_total - mem_free;
        mem_used *= 1024;

        return  to_string(mem_used);
    }
    else {

        getline(stream, line);
        string cache;
        for(char x : line) {
            if(isdigit(x)) {
                cache.append(to_string(int(x) - 48));
            }
        }
        int cache_mem = stoi(cache);
        cache_mem *= 1000;
        cache_mem *= 1000;
        return to_string(cache_mem);
    }
}

string get_cpu_info(string line, ifstream& stream, const string& info_type = "") {

    if(info_type == "clock") {

        for(int i = 0; i < 5; i++) {
            getline(stream, line);
        }
        int i = line.size() - 1;
        while (line.at(i) != ' ') {
            i--;
        }
        return line.substr(i + 1, line.size() - i);
    }
    else if(info_type == "type") {

        for(int i = 0; i < 5; i++) {
            getline(stream, line);
        }

        queue<string> parsed_line = parse_input(line);

        for(int i = 0; i < 3; i++) {
            parsed_line.pop();
        }

        string type = parsed_line.front();
        parsed_line.pop();
        type.append(parsed_line.front());

        return type;
    }
    else {
        // i NEEDS TO BE 12 in LINUX
        for(int i = 0; i < 12; i++) {
            getline(stream, line);
        }

        queue<string> parsed_line = parse_input(line);

        for(int i = 0; i < 3; i++) {
            parsed_line.pop();
        }

        return parsed_line.front();
    }
}

int random_num() {
    return 0 + (rand() % ( 255 - 0 + 1)) ;
}

void perform_custom_commands(queue<string> command, set<set<string>> tags, string& prompt, int& exit_status) {

    string str_comm = command.front();
    command.pop();

    if(str_comm == "cpuinfo") {

        if(command.empty()) {
            cpuinfo_manual();
        }

        else if(validate_tags(command, *tags.begin())) {

            ifstream cpuInfo("/proc/cpuinfo");
            string info;

            while(!command.empty()) {

                cpuInfo.seekg(0, ios::beg);

                if(command.front() == "-h" or command.front() == "-help") {
                    cpuinfo_manual();
                    exit_status = 0;
                }
                else if(command.front() == "-c") {
                    cout << "CPU Clock ---> " << get_cpu_info(info, cpuInfo, "clock") << "\n";
                }
                else if(command.front() == "-t") {
                    cout << "CPU Type ---> " << get_cpu_info(info, cpuInfo, "type") << "\n";
                }
                else {
                    cout << "Number of cores: ---> " << get_cpu_info(info, cpuInfo) << "\n";
                }
                command.pop();
            }
            cpuInfo.close();
            exit_status = 0;
        }
        else {
            cout << "cwuShell: " << str_comm << ": Illegal tag found\n";
            exit_status = random_num();
        }
    }
    else if(str_comm == "meminfo") {

        auto mem_info_tags = tags.begin();
        mem_info_tags++;

        if(command.empty()) {
            meminfo_manual();
        }
        else if(validate_tags(command, *mem_info_tags)) {

            ifstream memInfo("/proc/meminfo");
            string info;

            while(!command.empty()) {

                memInfo.seekg(0, ios::beg);

                if(command.front() == "-h" or command.front() == "-help") {
                    meminfo_manual();
                    exit_status = 0;
                }
                else if(command.front() == "-t") {
                    cout << "Total RAM ---> " << get_mem_info(info, memInfo, "total") << " Bytes\n";
                }
                else if(command.front() == "-u") {
                    cout << "RAM used ---> " << get_mem_info(info, memInfo, "used") << " Bytes\n";
                }
                else {
                    // LINUX: "/sys/devices/system/cpu/cpu0/cache/index2/size"
                    ifstream cacheInfo("/sys/devices/system/cpu/cpu0/cache/index2/size");
                    cout << "L2 cache/core size ---> " << get_mem_info(info, cacheInfo) << " Bytes\n";
                }
                command.pop();
            }
            memInfo.close();
            exit_status = 0;
        }
        else {
            cout << "cwuShell: " << str_comm << ": Illegal tag found\n";
            exit_status = random_num();
        }
    }
    else if(str_comm == "exit") {

        if(command.size() > 1) {
            cout << str_comm << ": Too many arguments\n";
            exit_status = random_num();
        }
        else if(command.size() == 0) {
            cout << "Process finished with exit code " << to_string(exit_status);
            exit(exit_status);
        }
        else if(command.front() == "-h" or command.front() == "-help") {
            exit_manual();
            exit_status = 0;
        }
        else if(validate_argument(command)) {
                string code = command.front().substr(0, command.front().size());
                cout << "Process finished with exit code " << code;
                exit(stoi(code));
        }
        else {
            cout << str_comm << ": Illegal argument found \n";
            exit_status = random_num();
        }
    }
    //command must be prompt
    else {
        if(command.empty()) {
            exit_status = 0;
            prompt = "cwuShell>";
        }

        else if(command.size() > 1) {
            cout << str_comm << ": Too many tags or arguments\n";
            exit_status = random_num();
        }
        else if(command.front() == "-h" or command.front() == "-help") {
            prompt_manual();
            exit_status = 0;
        }
        else {
            exit_status = 0;
            prompt = command.front();
            prompt.append(">");
        }
    }
}

int get_exit_code(string command) {

    FILE * data_stream;
    string code;
    const int max_buffer = 256;
    char buff[max_buffer];
    command.append("; echo $?");

    data_stream = popen(command.c_str(), "r");

    if (data_stream) {

        while (!feof(data_stream)) {
            if (fgets(buff, max_buffer, data_stream) != NULL) code.append(buff);
        }

        pclose(data_stream);
    }
    int pos = -1;

    for(char x : code) {

        pos++;
        if(isdigit(x)) continue;

        if(x == '\n') {
            code.erase(code.begin() + pos);
        }
        else{
            break;
        }
    }
    //special case error
    if(code == "0 .\n0\n") {
        return 1;
    }

    if(!check_num(code)) return false;
    return stoi(code);
}

int main() {

    string prompt = "cwushell>";
    string input;
    set<string> custom_commands = {"cpuinfo", "exit", "meminfo", "prompt", "manual"};
    set<set<string>> tags = {{"-c", "-t", "-n", "-h", "-help"}, {"-t", "-u", "-c", "-h", "-help"}};
    int exit_code = 0;

    cout << "Welcome to cwushell. Type 'maunal' to see what you can do.\n";
    cout << prompt;

    while(true) {
        getline(cin, input);

        if(input.empty()) {
            cout << prompt;
            continue;
        }

        queue<string> parsed_input;
        parsed_input = parse_input(input);

        auto itr = custom_commands.find(parsed_input.front());

        if (itr == custom_commands.end()) {

            exit_code = get_exit_code(input);

            if(exit_code == 0) {
                system(input.c_str());
                cout << prompt;
            }
            else {
                cout << "\n";
                cout << prompt;
            }
        }
        else {

            if(parsed_input.front() == "manual") {
                if(parsed_input.size() > 1) {
                    cout << "manual" << ": Too many tags or arguments\n";
                    exit_code = random_num();
                    cout << prompt;
                    continue;
                }
                cout << "All regular internal and external shell commands can be executed. In addition, there are a few \n" <<
                "custom commands that can be executed. These capabilities of these commands will be detailed here.\n" <<
                "----- Here are all the custom internal commands that can be performed -----\n\n";
                cpuinfo_manual();
                meminfo_manual();
                exit_manual();
                prompt_manual();
            }

            perform_custom_commands(parsed_input, tags, prompt, exit_code);
            cout << prompt;

        }
    }
}
