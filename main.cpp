#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <limits>

using namespace std;

struct User {
    string email;
    string username;
    string securityQuestion;
    string securityAnswer;
    unsigned long long passwordHash; 
    bool isAdmin;
};

class LoginSystem {
public:
    pair<bool, string> currentLoggedInUser = {false, ""};

    LoginSystem() {
        loadUsers();
    }

    void loadUsers() {
        ifstream infile(filename);
        string line;
        while (getline(infile, line)) {
            stringstream ss(line);
            User user;
            string passwordHashStr; 

            getline(ss, user.email, ':');
            getline(ss, user.username, ':');
            getline(ss, user.securityQuestion, ':');
            getline(ss, user.securityAnswer, ':');
            getline(ss, passwordHashStr, ':');
            ss >> user.isAdmin; 

            user.passwordHash = stoull(passwordHashStr);
            users[user.email] = user;
        }
        infile.close();
    }

    void saveUsers() {
        ofstream outfile(filename);
        for (const auto& pair : users) {
            const User& user = pair.second;
            outfile << user.email << ":" << user.username << ":" << user.securityQuestion << ":"
                    << user.securityAnswer << ":" << user.passwordHash << ":" << user.isAdmin << endl;
        }
        outfile.close();
    }

    void registerUser() {
        User newUser;
        cout << "Masukkan email: ";
        cin >> newUser.email;

        if (users.count(newUser.email) > 0) {
            cout << "Email sudah terdaftar!\n";
            return;
        }
        newUser.isAdmin = (newUser.email.find("@admin.com") != string::npos);

        cout << "Masukkan username: ";
        cin >> newUser.username;

        cout << "Masukkan pertanyaan keamanan: ";
        cin.ignore(); 
        getline(cin, newUser.securityQuestion);

        cout << "Masukkan jawaban keamanan: ";
        getline(cin, newUser.securityAnswer);

        string password;
        cout << "Masukkan password: ";
        cin >> password;
        newUser.passwordHash = hash<string>{}(password); 

        users[newUser.email] = newUser;
        saveUsers();
        cout << "Registrasi berhasil!\n";
    }

    bool login() {
        string email, password;
        cout << "Masukkan email: ";
        cin >> email;
        cout << "Masukkan password: ";
        cin >> password;

        if (users.count(email) > 0) {
            if (users[email].passwordHash == hash<string>{}(password)) {
                cout << "Login berhasil!\n";
                cout << "Selamat datang, " << (users[email].isAdmin ? "Admin" : "User") << "!\n";
                currentLoggedInUser = {true, email};
                return true;
            } else {
                cout << "Password salah. Apakah Anda lupa password? (y/n): ";
                char forgotPassword;
                cin >> forgotPassword;
                if (forgotPassword == 'y') {
                    if (resetPassword(email)) {
                        cout << "Silakan login kembali dengan password baru Anda.\n";
                    } // Kembali ke menu utama setelah reset password
                } else {
                    cout << "Login gagal!\n";
                }
                return false; // Tidak langsung login setelah reset password
            }
        } else {
            cout << "Email tidak terdaftar!\n";
            return false;
        }
    }

    bool resetPassword(const string& email) {
        if (users.count(email) == 0) {
            cout << "Email tidak terdaftar!\n";
            return false;
        }

        string answer;
        cout << users[email].securityQuestion << ": ";
        cin.ignore(); 
        getline(cin, answer);

        if (answer == users[email].securityAnswer) {
            string newPassword;
            cout << "Masukkan password baru: ";
            cin >> newPassword;
            users[email].passwordHash = hash<string>{}(newPassword);
            saveUsers();
            cout << "Passwordberhasil diubah!\n";
            return true;
        } else {
            cout << "Jawaban keamanan salah!\n";
            return false;
        }
    }

    pair<bool, User> getCurrentUser() const {
        if (currentLoggedInUser.first) {
            return {true, users.at(currentLoggedInUser.second)};
        } else {
            return {false, User{}};
        }
    }

private:
    unordered_map<string, User> users;
    string filename = "users.txt";
};

int main() {
    LoginSystem loginSystem;
    int choice;

    do {
        cout << "\nMenu:\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "0. Keluar\n";
        cout << "Pilihan: ";
        cin >> choice;

        switch (choice) {
            case 1:
                if (loginSystem.login()) { // Masuk ke halaman utama hanya jika login berhasil
                    cout << "Halaman utama\n";
                    string command;
                    while (true) {
                        cout << "Command: ";
                        cin >> command;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

                        auto [isLoggedIn, currentUser] = loginSystem.getCurrentUser();
                        if (isLoggedIn) {
                            if (command == "whoami") {
                                cout << "Username: " << currentUser.username << endl;
                            } else if (command == "logout") {
                                loginSystem.currentLoggedInUser = {false, ""};
                                break;
                            } else {
                                cout << "Command tidak valid.\n";
                            }
                        } else {
                            cout << "Tidak ada pengguna yang login.\n";
                        }
                    }
                } 
                break;
            case 2:
                loginSystem.registerUser();
                break;
            case 0:
                cout << "Keluar dari program.\n";
                break;
            default:
                cout << "Pilihan tidak valid.\n";
        }
    } while (choice != 0);

    return 0;
}
