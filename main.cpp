#include <iostream>
#include <windows.h>
#include <string>
#include <time.h>
#include <conio.h>
#include <fstream>
#include <vector>

using namespace std;

class Data{
public:
    float windSpeed;
    float windDestination;
    string tester;
    string time;
    Data(string);
    void print();
};

void charAddToString(string& str, char* buff);

string findData(string& str);

int writeToFile(string str, int& index);

void readFromFile();

int main()
{
    char ch;
    cout << "Write(w) or read(r) data from tester?\n";
    while(true){
        ch = _getch();
        if(ch == 'w' || ch == 'r') break;
    }
    if(ch == 'w'){
        int index = 0;
        const size_t n = 1;
        string data = "";
        string jsonString = "";
        HANDLE hSerial;
        hSerial = CreateFile("COM2", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if(hSerial==INVALID_HANDLE_VALUE){
            if(GetLastError()==ERROR_FILE_NOT_FOUND){
                cout << "Com port error\n";
                return 1;
            }
            return 999;
        }
        DCB dcbSerial{0};
        dcbSerial.DCBlength = sizeof(dcbSerial);

        if(!GetCommState(hSerial, &dcbSerial)){
            cout << "Get state error\n";
            return 2;
        }

        dcbSerial.BaudRate = CBR_2400;
        dcbSerial.ByteSize = 8;
        dcbSerial.StopBits = ONESTOPBIT;
        dcbSerial.Parity = NOPARITY;

        if(!SetCommState(hSerial, &dcbSerial)){
            cout << "Set state error\n";
            return 3;
        }

        COMMTIMEOUTS timeouts={0};

        timeouts.ReadIntervalTimeout=50;
        timeouts.ReadTotalTimeoutConstant=50;
        timeouts.ReadTotalTimeoutMultiplier=10;

        if(!SetCommTimeouts(hSerial, &timeouts)){
            cout << "Set timeouts error\n";
            return 6;
        }

        while(true){
            char szBuff[n+1] = {0};
            DWORD dwBytesRead = 0;
            if(!ReadFile(hSerial, szBuff, n, &dwBytesRead, NULL)){
                cout << "Read error\n";
                return 7;
            }
            charAddToString(data, szBuff);
            if(data.size() >= 15){
                jsonString = findData(data);
                if(jsonString.size()){
                    if(writeToFile(jsonString, index)){
                        return 0;
                    }
                    jsonString = "";
                }
            }
        }
    }
    if(ch == 'r'){
        readFromFile();
        cout << "End of reading\n";
        return 0;
    }
}

Data::Data(string str){
    int countOfVariables = 4;
    std::vector<string> values;
    bool skip = 1;
    size_t start = str.find_first_of('"', 0);
    while(start != std::string::npos){
        size_t end = str.find_first_of('"', start + 1);
        if(!skip){
            values.push_back(str.substr(start+1, end - start - 1));
        }
        skip = !skip;
        start = str.find_first_of('"', end + 1);
    }
    this->windSpeed = stof(values[0]);
    this->windDestination = stof(values[1]);
    this->time = values[2];
    this->tester = values[3];
}

void Data::print(){
    cout <<
    "Tester: " + this->tester + "\n"
    "Wind Speed: " + to_string(this->windSpeed) + "\n"
    "Wind Destination: " + to_string(this->windDestination) + "\n"
    "Time: " + this->time + "\n";
}

void charAddToString(string& str, char* buff){
    str += buff[0];
}

string findData(string& str){
    string windDestination = "", windSpeed = "", timeString = "";
    time_t timeNow = time(0);
    char timeBuffer[9];
    tm* localTime = localtime(&timeNow);
    for(int i = 0; i < str.size() - 15; i++){
        if(str[i] == '$'){
            windSpeed = str.substr(i+1, 5);
            windDestination = str.substr(i+7, 6);
            str = "";
            break;
        }
    }
    if(windSpeed.empty() && windDestination.empty()){
        return "";
    }
    strftime(timeBuffer, 9, "%T", localTime);
    for(int i = 0; i < 8; i++){
        timeString += timeBuffer[i];
    }
    return  "{"
            "\"windSpeed\": \"" + windSpeed + "\", "
            "\"windDestination\": \"" + windDestination + "\", "
            "\"time\": \"" + timeString + "\", "
            "\"Tester\": \"" + "WMT700" + "\""
            "}\n";
}

int writeToFile(string str, int& index){
    ofstream out;
    if(index == 0){
        out.open("data.json", ios::out);
    }
    else{
        out.open("data.json", ios::app | ios::ate);
    }
    if(out.is_open()){
        out << str;
        cout << "Message #" + to_string(index+1) + " was writed\n";
    }
    else{
        cout << "Write error\n";
    }
    index += 1;
    if(index == 10){
        cout << "End of writing\n";
        return 1;
    }
    out.close();
    return 0;
}

void readFromFile(){
    ifstream in;
    in.open("data.json", ios::in);
    string str;
    if(in.is_open()){
        while(getline(in, str)){
            if(str.empty()) break;
            Data data(str);
            data.print();
        }
    }
    in.close();
}
