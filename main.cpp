#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>

using namespace std;


// Global variables definition

bool verbose = false;


// Functions definition

void debug(const string&str, bool force);
bool isRegistered(string username, string password);
int registerUser(string username, string password);
int login (string username, string password);
int generateCode();
void getParams(string * username, string * password, int argc, char ** argv);

// Program

void debug(const string& str, bool force=false) {
    if (verbose&&!force) cout << endl << str << endl;
}

bool isRegistered(string username, string password) {
    return false;
}

int registerUser(string username, string password) {
    return 1;
}

int login(string username, string password) {
    return 1;
}

int generateCode() {
    return rand()%7;
}

void getParams(string * username, string * password, int argc, char ** argv) {
    if (argc>1) {
        for (int i = 0; i < argc; i++) {
            string argvs = argv[i];
            switch (i) {
                case 1:
                    *username = argvs;
                    break;
                case 2:
                    *password = argvs;
                    break;
                case 3:
                    if (argvs == "-v") verbose = true;
                    break;
                default:
                    debug("Arguments parsing error!", true);
                    break;
            }
        }
        return;
    }
    cout << "Enter username: ";
    cin >> *username;
    cout << "Enter password";
    cin >> *password;
}

int main(int argc, char ** argv) {
    string username, password;
    getParams(&username, &password, argc, argv);

    if (isRegistered(username, password)) login(username, password);
    else {
        char reg;
        cout << "There is no user with username " << username << "\nDo you want to register a new user? [y/n]: ";
        cin >> reg;
        if (reg=='y') registerUser(username, password);
        else return 1;
    }

    return 0;
}
