#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <ctime>

#include "sha256.h"

using namespace std;

// Global variables definition

bool verbose = false;
const string zero_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4hash_length9b934ca495991b7852b855";
const int hash_length = 64;


// Functions definition

bool isRegistered(const string& username, const string& password);
bool checkFileHash(const string& filename, FILE * b_file, bool repair);
bool compare(const char * a, const char * b);
int registerUser(string username, string password);
int login (string username, string password);
int generateCode();
void getParams(string * username, string * password, int argc, char ** argv);
void debug(const string&str, bool force);
void writeHash(FILE * file, const string& hash, int number_of_users);
void writeHash(FILE * file, const char * hash, int number_of_users);
void stringToArray(char * arr, const string& str);
void equate(char * a, const char * b);
string getFileHash(const string& filename);
string getCurrentDateTime();
char * stringToArray(const string& str);

// Program

template <typename T>
void debug(const T& str, bool force=false) {
    if (verbose||force) cout << endl << str << endl;
}

bool compare(const char * a, const char * b) {
    for (int i = 0; i < 64; i ++)
        if (a[i]!=b[i]) return false;
    return true;
}

void stringToArray(char * array, const string& str) {
    size_t length = str.length() + 1;
    array = new char[length];
    strcpy(array, str.c_str());
}

char * stringToArray(const string& str) {
    char * array;
    size_t length = str.length() + 1;
    array = new char[length];
    strcpy(array, str.c_str());
    return array;
}

void filecopy(FILE *ft, FILE *fs)
{
    char ch;
    debug("Creating a copy of file");
    if(fs == NULL)
    {
        return;
    }
    if(ft == NULL)
    {
        cout<<"\nError Occurred!";
        return;
    }
    ch = fgetc(fs);
    while(ch != EOF)
    {
        fputc(ch, ft);
        ch = fgetc(fs);
    }
    fclose(fs);
    fclose(ft);
    debug("File copied!");
}

void equate(char * a, const char * b) {
    for (int i = 0; i < 64; i ++)
        a[i] = b[i];
}

void writeHash(FILE * file, const string& hash, int number_of_users) {
    debug("Writing hash " + hash);
    char * arr;
    stringToArray(arr, hash);
    fseek(file, 0, SEEK_SET);
    fwrite(arr, sizeof(char), hash_length, file);
    fseek(file, sizeof(char)*number_of_users*hash_length + sizeof(int), SEEK_CUR);
    fwrite(arr, sizeof(char), hash_length, file);
    fseek(file, -hash_length*sizeof(char), SEEK_END);
    fwrite(arr, sizeof(char), hash_length, file);
    debug("Hash written!");
}

void writeHash(FILE * file, const char * hash, int number_of_users) {
    fseek(file, 0, SEEK_SET);
    fwrite(hash, sizeof(char), hash_length, file);
    fseek(file, sizeof(char)*number_of_users*hash_length + sizeof(int), SEEK_CUR);
    fwrite(hash, sizeof(char), hash_length, file);
    fseek(file, -hash_length*sizeof(char), SEEK_END);
    fwrite(hash, sizeof(char), hash_length, file);
    debug("Hash written!");
}

bool checkFileHash(const string& filename, FILE * b_file, bool repair=false) {
    debug("Checking file hash for file " + filename);
    FILE * tmp = fopen(".tmp.dat", "wb+");
    int number_of_users;
    char hash[64], hash1[64], hash2[64], realHash[64];
    filecopy(tmp, b_file);

    fseek(tmp, 0, SEEK_SET);
    fseek(b_file, sizeof(string), SEEK_SET);

    fread(&number_of_users, sizeof(int), 1, b_file);

    debug(number_of_users);

    fread(&hash, sizeof(char), 64, b_file);
    fseek(b_file, number_of_users*sizeof(char)*64, SEEK_CUR);
    fread(&hash1, sizeof(char), 64, b_file);
    fseek(b_file, -sizeof(char), SEEK_END);
    fread(&hash2, sizeof(char), 64, b_file);
    fclose(b_file);

//    debug("First hash: ");
//    debug("Second hash: ");
//    debug("Third hash: ");

    if (repair && !(compare(hash,hash1)||compare(hash,hash2)||compare(hash1,hash2))) {
        debug("Hash is damaged and will be repaired!");
        if (!(compare(hash,hash1)&&compare(hash1,hash2)&&compare(hash,hash2))) return false;
        char trueHash[64];
        if (compare(hash1,hash2)&&!compare(hash, hash1)) {
            // First hash is broken
            equate(trueHash, hash1);
        }
        if (compare(hash,hash2)&&!compare(hash,hash1)) {
            // Second hash is broken
            equate(trueHash, hash);
        }
        if (compare(hash,hash1)&&!compare(hash,hash2)) {
            // Third hash is broken
            equate(trueHash, hash1);
        }
        writeHash(b_file, trueHash, number_of_users);
        debug("Hash successfully repaired!");
        equate(realHash,trueHash);
    }

//    fwrite(&zero_hash, sizeof(char), 64, tmp);
//    fseek(tmp, number_of_users*sizeof(char)*64, SEEK_CUR);
//    fwrite(&zero_hash, sizeof(char), 64, tmp);
//    fseek(tmp, 0, SEEK_END);
//    fwrite(&zero_hash, sizeof(char), 64, tmp);

    writeHash(tmp, zero_hash, number_of_users);

    equate(realHash,stringToArray(getFileHash(".tmp.dat")));
    fclose(tmp);
    remove(".tmp.dat");

    return (compare(hash, realHash));

}

string getCurrentDateTime() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);

    string str(buffer);
    return str;
}

bool isRegistered(const string& username, const string& password) {
    debug("Checking if user is registered");
    FILE * usersFile;
    string filename = "users.dat";
    string fileHash = getFileHash(filename);
    usersFile = fopen("users.dat", "rb");
    ifstream check(filename);
    if(!check) return false;
    debug("Data file exists");
    if (!checkFileHash(filename, usersFile, true)) {
        debug("Users data file is broken and unrepairable. Report can be found in log.txt!", true);
        ofstream log("log.txt", ios::app);

        log << "Users data file broken! This accident appeared at: " << getCurrentDateTime();
        log.close();

        exit(1);
    }
    debug("Hash check OK. Decoding users data");

    int number_of_users;

    fseek(usersFile, sizeof(string), SEEK_SET);
    fread(&number_of_users, sizeof(int), 1, usersFile);

    debug(number_of_users + " users found");

    return false;
}

string getFileHash(const string& filename) {
    debug("Getting hash for file " + filename);
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
    debug("Registering user");
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
        string hashed = zero_hash;
        usersFile=fopen("users.dat", "wb");
        int number_of_users = 1;
        string users[1] = {sha256(username)};
        string passwords[1] = {sha256(password)};
        writeHash(usersFile, hashed, 1);
//        fwrite(&hashed, sizeof(string), 1, usersFile);
//        fwrite(&number_of_users, sizeof(int),  1, usersFile);
//        fwrite(users, sizeof(string), 1, usersFile);
//        fwrite(&hashed, sizeof(string), 1, usersFile);
//        fwrite(passwords, sizeof(string), 1, usersFile);
//        fwrite(&hashed, sizeof(string), 1, usersFile);
//        fclose(usersFile);

        // Calculate file hash sum
        hashed = getFileHash("users.dat");

        // Write hash sum to file

        usersFile = fopen("users.dat", "rb+");
        writeHash(usersFile, zero_hash, 1);
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
            }
        }
        return;
    }
    cout << "Enter username: ";
    cin >> *username;
    cout << "Enter password: ";
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
