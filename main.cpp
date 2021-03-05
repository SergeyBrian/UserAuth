#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <ctime>

#include "sha256.h"

using namespace std;

// Global variables definition

bool verbose = false;
const string zero_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
const int hash_length = 64;


// Functions definition

int login(const string& username, const string& password);
bool checkFileHash(FILE * b_file, bool repair);
bool compare(const char * a, const char * b);
int registerUser(string username, string password);
int generateCode();
void getParams(string * username, string * password, int argc, char ** argv);
void writeHash(FILE * file, const char * hash, int number_of_users);
void equate(char * a, const char * b);
string getFileHash(const string& filename);
string getCurrentDateTime();
char * stringToArray(const string& str);

// Program

template <typename T>
void debug(const T& str, bool force=false) {
    if (verbose||force) cout << endl << str << endl;
}

template<typename T>
void printArr(const T& arr) {
    if (!verbose) return;
    for (int i = 0; i < 64; i++)
        cout << arr[i];
    cout << endl;
}

bool compare(const char * a, const char * b) {
    for (int i = 0; i < 64; i ++)
        if (a[i]!=b[i]) return false;
    return true;
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
    debug("File copied!");
}

void equate(char * a, const char * b) {
    for (int i = 0; i < 64; i ++)
        a[i] = b[i];
}

void writeHash(FILE * file, const char * hash, int number_of_users) {
    debug(number_of_users);
    printArr(hash);
    fseek(file, 0, SEEK_SET);
    fwrite(hash, sizeof(char), hash_length, file);
    fseek(file, sizeof(char)*number_of_users*hash_length + sizeof(int), SEEK_CUR);
    fwrite(hash, sizeof(char), hash_length, file);
    fseek(file, sizeof(char)*number_of_users*hash_length, SEEK_CUR);
    fwrite(hash, sizeof(char), hash_length, file);
    debug("Hash written!");
}

bool checkFileHash(FILE * b_file, bool repair=false) {
    debug("Checking file hash");
    FILE * tmp = fopen(".tmp.dat", "wb+");
//    b_file = fopen("users.dat", "rb+");
    int number_of_users;
    char hash[64], hash1[64], hash2[64], realHash[64];
    filecopy(tmp, b_file);

    fseek(tmp, 0, SEEK_SET);
    fseek(b_file, 0, SEEK_SET);

    fread(hash, sizeof(char), 64, b_file);
    fread(&number_of_users, sizeof(int), 1, b_file);
    debug(number_of_users);
    fseek(b_file, number_of_users*sizeof(char)*64, SEEK_CUR);
    fread(hash1, sizeof(char), 64, b_file);
    fseek(b_file, number_of_users*sizeof(char)*64, SEEK_CUR);
    fread(hash2, sizeof(char), 64, b_file);

    if (repair && !(compare(hash,hash1)&&compare(hash,hash2)&&compare(hash1,hash2))) {
        if (!(compare(hash,hash1)||compare(hash1,hash2)||compare(hash,hash2))) {
            debug("Can't repair");
            return false;
        }
        char * trueHash;
        debug("Hash is damaged, but can be repaired!");
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
        equate(hash,trueHash);
    }

    writeHash(tmp, stringToArray(zero_hash), number_of_users);
    fclose(tmp);
    equate(realHash, stringToArray(getFileHash(".tmp.dat")));

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

int login(const string& username, const string& password) {
    debug("Checking if user is registered");
    string filename = "users.dat";

    ifstream check(filename);
    if(!check) return 1;
    check.close();

    debug("Data file exists");

    FILE * usersFile = fopen("users.dat", "rb");

    if (!checkFileHash(usersFile, true)) {
        debug("Users data file is broken and unrepairable. Report can be found in log.txt!", true);
        ofstream log("log.txt", ios::app);
        log << "\nUsers data file broken! This accident appeared at: " << getCurrentDateTime();
        log.close();
        fclose(usersFile);

        exit(1);
    }
    debug("Hash check passed. Decoding users' data");

    int number_of_users;

    fclose(usersFile);
    usersFile = fopen("users.dat", "rb");

    fseek(usersFile, sizeof(char)*64, SEEK_SET);
    fread(&number_of_users, sizeof(int), 1, usersFile);

    debug(number_of_users);
    int noUser = 1; // 1: user with given name not found; 2: user with given name found
    for (int i = 0; i < number_of_users; i ++) {
        char usrn[64];
        fseek(usersFile, sizeof(char)*64*(i+1)+sizeof(int), SEEK_SET);
        fread(usrn, sizeof(char), 64, usersFile);
        printArr(usrn);
        if (compare(usrn, stringToArray(sha256(username)))) {
            noUser = 2;
            fseek(usersFile, sizeof(char)*64*(number_of_users+2+i)+sizeof(int), SEEK_SET);
            char ps[64];
            fread(ps, sizeof(char), 64, usersFile);
            if (compare(ps, stringToArray(sha256(password)))) {
                return 0;
            }
        }
    }

    return noUser;
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

    ifstream check("users.dat");
    if (!check) {
        debug("Users file not found. Creating a new one!");
        string hashed = zero_hash;
        usersFile=fopen("users.dat", "wb");

        char * usernames = stringToArray(sha256(username));
        char * userpasswords = stringToArray(sha256(password));

        char ** users;
        char ** passwords;

        users = new char * [1];
        passwords = new char * [1];

        users[0] = {usernames};
        passwords[0] = {userpasswords};
        int number_of_users = 1;

        fseek(usersFile, sizeof(char)*64, SEEK_SET);
        fwrite(&number_of_users, sizeof(int), 1, usersFile);
        fwrite(users[0], sizeof(char), 64*number_of_users, usersFile);
        fseek(usersFile, sizeof(char)*64, SEEK_CUR);
        fwrite(passwords[0], sizeof(char), 64*number_of_users, usersFile);

        fclose(usersFile);

        usersFile = fopen("users.dat", "rb+");

        writeHash(usersFile, stringToArray(hashed), 1);
        writeHash(usersFile, stringToArray(hashed), 1);

        // Calculate file hash sum
        hashed = getFileHash("users.dat");
        // Write hash sum to file

        writeHash(usersFile, stringToArray(hashed), 1);
        debug("Registration successful!", true);
        fclose(usersFile);
        return 1;
    }

    check.close();
    username = sha256(username);
    password = sha256(password);

    usersFile = fopen("users.dat", "rb+");
    debug("Registering user...");
    int number_of_users;
    fseek(usersFile, sizeof(char) * 64, SEEK_SET);
    fread(&number_of_users, sizeof(int), 1, usersFile);
    char ** passwords;
    passwords = new char * [number_of_users + 1];

    fseek(usersFile, sizeof(char)*64*(number_of_users+2)+sizeof(int), SEEK_SET);
    for (int i = 0; i < number_of_users; i ++) {
        passwords[i] = new char[64];
        fread(passwords[i], sizeof(char), 64, usersFile);
        printArr(passwords[i]);
    }

    passwords[number_of_users] = stringToArray(password);
    printArr(passwords[number_of_users]);

    number_of_users++;

    fseek(usersFile, sizeof(char)*64, SEEK_SET);
    fwrite(&number_of_users, sizeof(int), 1, usersFile);
    fseek(usersFile, sizeof(char)*64*(number_of_users)+sizeof(int), SEEK_SET);
    fwrite(stringToArray(username), sizeof(char),64,usersFile);

    fseek(usersFile, sizeof(char)*64,SEEK_CUR);
    for (int i = 0; i < number_of_users; i++)
        fwrite(passwords[i], sizeof(char), 64, usersFile);
    writeHash(usersFile, stringToArray(zero_hash), number_of_users);
    writeHash(usersFile, stringToArray(zero_hash), number_of_users);

    string hash = getFileHash("users.dat");
    debug(hash);

    writeHash(usersFile, stringToArray(hash), number_of_users);
    debug("Registration successful!", true);

    return 0;
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
    srand(time(nullptr));
    string username, password;
    getParams(&username, &password, argc, argv);
    int loginResult = login(username, password);
    if (loginResult == 0) debug("Login successful!", true);
    else if (loginResult == 1) {
        char reg;
        cout << "There is no user with username " << username << "\nDo you want to register a new user? [y/n]: ";
        cin >> reg;
        if (reg=='y') registerUser(username, password);
        else return 1;
    }
    else if (loginResult == 2) {
        debug("Wrong password!", true);
    }

    return 0;
}
