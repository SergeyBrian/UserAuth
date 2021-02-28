#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>

#include "sha256.h"

using namespace std;

// Global variables definition

bool verbose = false;


// Functions definition

void debug(const string&str, bool force);
bool isRegistered(const string& username, const string& password);
int registerUser(string username, string password);
int login (string username, string password);
int generateCode();
void getParams(string * username, string * password, int argc, char ** argv);
string getFileHash(const string& filename);

// Program

template <typename T>
void debug(const T& str, bool force=false) {
    if (verbose||force) cout << endl << str << endl;
}

bool isRegistered(const string& username, const string& password) {
    return false;
}

string getFileHash(const string& filename) {
    ifstream data;
    unsigned char x;
    string hashed;

    data.open(filename, ios::binary);
    while (data >> x)
        hashed += x;
    hashed = sha256(hashed);
    data.close();
    return hashed;
}

int registerUser(string username, string password) {
    ofstream codeFile;
    codeFile.open("code.txt");
    int code = generateCode();
    int ucode;
    debug(code);

    codeFile << code;
    codeFile.close();

    cout << "Please, enter code from code.txt: ";
    cin >> ucode;

    if (code!=ucode) {
        debug("Wrong code!", true);
        remove("code.txt");
        return 0;
    }
    remove("code.txt");

    FILE * usersFile;

    if (1) {
        string hashed = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
        usersFile=fopen("users.dat", "wb");
        int number_of_users = 1;
        string users[1] = {sha256(username)};
        string passwords[1] = {sha256(password)};
        fwrite(&hashed, sizeof(string), 1, usersFile);
        fwrite(&number_of_users, sizeof(int),  1, usersFile);
        fwrite(users, sizeof(string), 1, usersFile);
        fwrite(&hashed, sizeof(string), 1, usersFile);
        fwrite(passwords, sizeof(string), 1, usersFile);
        fwrite(&hashed, sizeof(string), 1, usersFile);
        fclose(usersFile);

        // Calculate file hash sum
        hashed = getFileHash("users.dat");

        // Write hash sum to file

        usersFile = fopen("users.dat", "rb+");
        fseek(usersFile, 0, SEEK_SET);
        fwrite(&hashed, sizeof(string), 1, usersFile);
        fseek(usersFile, sizeof(string) + sizeof(int), SEEK_CUR);
        fwrite(&hashed, sizeof(string), 1, usersFile);
        fseek(usersFile, 0, SEEK_END);
        fwrite(&hashed, sizeof(string), 1, usersFile);
        debug("Registration successful!", true);
        return 1;
    }
    debug("Registration failed!", true);
    
    return 0;
}

int login(string username, string password) {
    return 1;
}

int generateCode() {
    return 100000 + rand()%(999999-100000+1);
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
