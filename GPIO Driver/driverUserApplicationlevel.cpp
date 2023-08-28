#include <iostream>
#include <fstream>
using namespace std;

class Driver
{
    private:
        const string path="/dev/test_file";
        fstream m_fd;


    public:
        void WriteFile(string message);
        string readfile();
};

void Driver::WriteFile(string message)
{
    m_fd.open(path,ios::out);
    m_fd.write(message.c_str(),message.size());
    m_fd.close();
}
string Driver::readfile()
{
    string result;
    m_fd.open(path);
    getline(m_fd,result);
    m_fd.close();
    return result;
}


class Display {
    private:
        static bool flag;
    public:
        static void DisplayOnChange(string value)
        {
            if(value.find("1")!=string::npos && flag==false)
            {
                cout<<"LED is now ON"<<endl;
                flag=true;
            }
            else if(value.find("0")!=string::npos && flag==true)
            {
                cout<<"LED is now OFF"<<endl;
                flag=false;;  
            }
        }
};
bool Display::flag=false;
int main()
{
    Driver myDriver;
    string input;

    cout<<"Application is running"<<endl;
    //getline(cin,input);
    while(1)
    {
        string input=myDriver.readfile();
        myDriver.WriteFile(input);
        Display::DisplayOnChange(input);
    }
    return 0;
}
