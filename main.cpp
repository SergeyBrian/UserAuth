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


// Functions definition

bool isRegistered(const string& username, const string& password);
bool checkFileHash(const string& filename, FILE * b_file, bool repair);
int registerUser(string username, string password);
int login (string username, string password);
int generateCode();
void getParams(string * username, string * password, int argc, char ** argv);
void debug(const string&str, bool force);
void writeHash(FILE * file, const string& hash, int number_of_users);
string getFileHash(const string& filename);
string getCurrentDateTime();

// Program

template <typename T>
void debug(const T& str, bool force=false) {
    if (verbose||force) cout << endl << str << endl;
}

void filecopy(FILE *dest, FILE *src)
{
    const int size = 16384;
    char buffer[size];

    while (!feof(src))
    {
        int n = fread(buffer, 1, size, src);
        fwrite(buffer, 1, n, dest);
    }

    fflush(dest);
}

void writeHash(FILE * file, const string& hash, int number_of_users) {
    fseek(file, 0, SEEK_SET);
    fwrite(&hash, sizeof(string), number_of_users, file);
    fseek(file, sizeof(string) + sizeof(int), SEEK_CUR);
    fwrite(&hash, sizeof(string), number_of_users, file);
    fseek(file, -sizeof(string), SEEK_END);
    fwrite(&hash, sizeof(string), number_of_users, file);
}

bool checkFileHash(const string& filename, FILE * b_file, bool repair=false) {
    debug("Checking file hash");
    ifstream file(filename);
    FILE * tmp = fopen(".tmp.dat", "wb+");
    int number_of_users;
    string hash, hash1, hash2, realHash;
    filecopy(tmp, b_file);

    fseek(tmp, 0, SEEK_SET);
    fseek(b_file, 0, SEEK_SET);

    fread(&number_of_users, sizeof(int), 1, b_file);
    fread(&hash, sizeof(string), 1, b_file);
    fseek(b_file, number_of_users*sizeof(string), SEEK_CUR);
    fread(&hash1, sizeof(string), 1, b_file);
    fseek(b_file, -sizeof(string), SEEK_END);
    fread(&hash2, sizeof(string), 1, b_file);
    fclose(b_file);

    debug("First hash: " + hash);
    debug("Second hash: " + hash1);
    debug("Third hash: " + hash2);

    if (repair && (hash!=hash1||hash!=hash2||hash1!=hash2)) {
        debug("Hash is damaged and will be repaired!");
        if (hash != hash1 && hash1 != hash2 && hash != hash2) return false;
        string trueHash;
        if (hash != hash1 && hash1 == hash2) {
            // First hash is broken
            trueHash = hash1;
        }
        if (hash1 != hash2 && hash == hash2) {
            // Second hash is broken
            trueHash = hash;
        }
        if (hash2 != hash && hash == hash1) {
            // Third hash is broken
            trueHash = hash1;
        }
        writeHash(b_file, trueHash, number_of_users);
        debug("Hash successfully repaired!");
        realHash = trueHash;
    }

    fwrite(&zero_hash, sizeof(string), 1, tmp);
    fseek(tmp, number_of_users*sizeof(string), SEEK_CUR);
    fwrite(&zero_hash, sizeof(string), 1, tmp);
    fseek(tmp, 0, SEEK_END);
    fwrite(&zero_hash, sizeof(string), 1, tmp);

    realHash = getFileHash(".tmp.dat");
    fclose(tmp);
    remove(".tmp.dat");

    return (hash == realHash);

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
