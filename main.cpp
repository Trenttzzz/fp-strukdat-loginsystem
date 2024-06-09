#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <limits>

using namespace std;

struct User
{
    string email;
    string username;
    string securityQuestion;
    string securityAnswer;
    unsigned long long passwordHash;
    bool isAdmin;
};

class LoginSystem
{
private:
    bool isValidPassword(const string &password)
    {
        if (password.length() < 8)
        {
            return false;
        }

        bool hasUpper = false, hasLower = false, hasDigit = false, hasSymbol = false;
        for (char c : password)
        {
            if (isupper(c))
                hasUpper = true;
            if (islower(c))
                hasLower = true;
            if (isdigit(c))
                hasDigit = true;
            if (!isalnum(c))
                hasSymbol = true; // jika bukan huruf atau angka, maka simbol
        }

        return hasUpper && hasLower && hasDigit && hasSymbol;
    }

public:
    pair<bool, string> currentLoggedInUser = {false, ""};

    LoginSystem()
    {
        loadUsers();
    }

    void loadUsers()
    {
        ifstream infile(filename);
        string line;
        while (getline(infile, line))
        {
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

    void saveUsers()
    {
        ofstream outfile(filename);
        for (const auto &pair : users)
        {
            const User &user = pair.second;
            outfile << user.email << ":" << user.username << ":" << user.securityQuestion << ":"
                    << user.securityAnswer << ":" << user.passwordHash << ":" << user.isAdmin << endl;
        }
        outfile.close();
    }

    void registerUser()
    {
        User newUser;
        cout << "Masukkan email:\n";
        cout << ">> ";

        cin >> newUser.email;

        if (users.count(newUser.email) > 0)
        {
            cout << "Email sudah terdaftar!\n";
            return;
        }
        newUser.isAdmin = (newUser.email.find("@admin.com") != string::npos);

        cout << "Masukkan username:\n";
        cout << ">> ";

        cin >> newUser.username;

        cout << "Masukkan pertanyaan keamanan:\n";
        cout << ">> ";

        cin.ignore();
        getline(cin, newUser.securityQuestion);

        cout << "Masukkan jawaban keamanan:\n";
        cout << ">> ";

        getline(cin, newUser.securityAnswer);

        string password;
        bool validPassword = false;
        while (!validPassword)
        {
            cout << "masukkan password (minimal 8 karakter, mengandung huruf besar, huruf kecil, angka, dan simbol):\n";
            cout << ">> ";

            cin >> password;

            validPassword = isValidPassword(password);
            if (!validPassword)
            {
                cout << "Password tidak valid. Silahkan coba lagi!\n";
            }
        }
        newUser.passwordHash = hash<string>{}(password);

        users[newUser.email] = newUser;
        saveUsers();
        cout << "Registrasi berhasil!\n";
    }

    bool login()
    {
        string email, password;
        cout << "Masukkan email:\n";
        cout << ">> ";
        cin >> email;
        cout << "Masukkan password:\n";
        cout << ">> ";
        cin >> password;

        if (users.count(email) > 0)
        {
            if (users[email].passwordHash == hash<string>{}(password))
            {
                cout << "Login berhasil!\n";
                cout << "Selamat datang, " << (users[email].isAdmin ? "Admin" : "User") << "!\n";
                currentLoggedInUser = {true, email};
                return true;
            }
            else
            {
                cout << "Password salah. Apakah Anda lupa password? (y/n):\n";
                cout << ">> ";
                char forgotPassword;
                cin >> forgotPassword;
                if (forgotPassword == 'y')
                {
                    if (resetPassword(email))
                    {
                        cout << "Silakan login kembali dengan password baru Anda.\n";
                    } // Kembali ke menu utama setelah reset password
                }
                else
                {
                    cout << "Login gagal!\n";
                }
                return false; // Tidak langsung login setelah reset password
            }
        }
        else
        {
            cout << "Email tidak terdaftar!\n";
            return false;
        }
    }

    bool resetPassword(const string &email)
    {
        if (users.count(email) == 0)
        {
            cout << "Email tidak terdaftar!\n";
            return false;
        }

        string answer;
        cout << users[email].securityQuestion << ": ";
        cin.ignore();
        getline(cin, answer);

        if (answer == users[email].securityAnswer)
        {
            string newPassword;
            bool validPassword = false;
            while (!validPassword)
            {
                cout << "masukkan password baru (minimal 8 karakter, mengandung huruf besar, huruf kecil, angka, dan simbol):\n";
                cout << ">> ";
                cin >> newPassword;

                validPassword = isValidPassword(newPassword);
                if (!validPassword)
                {
                    cout << "Password tidak valid. Silahkan coba lagi!\n";
                }
            }
            users[email].passwordHash = hash<string>{}(newPassword);
            saveUsers();
            cout << "Password berhasil diubah!\n";
            return true;
        }
        else
        {
            cout << "Jawaban keamanan salah!\n";
            return false;
        }
    }

    void editUser(const string &usernameToEdit)
    {
        auto it = users.begin();
        for (; it != users.end(); ++it)
        {
            if (it->second.username == usernameToEdit)
            {
                break; // Keluar dari loop jika user ditemukan
            }
        }

        if (it == users.end())
        {
            cout << "Username tidak ditemukan!\n";
            return;
        }

        User &userToEdit = it->second; // Reference ke user yang akan diedit

        cout << "\nEdit User:\n";

        // Edit email (dengan validasi)
        string newEmail;
        do
        {
            cout << "Masukkan email baru (kosongkan jika tidak ingin mengubah):\n";
            cout << ">> ";

            getline(cin, newEmail);
            if (newEmail.empty())
            {
                break; // Tidak mengubah email jika input kosong
            }
            else if (users.count(newEmail) > 0 && newEmail != userToEdit.email)
            {
                cout << "Email sudah terdaftar!\n";
            }
            else
            {
                userToEdit.email = newEmail;
                break;
            }
        } while (true);

        // Edit username (dengan validasi)
        string newUsername;
        do
        {
            cout << "Masukkan username baru (kosongkan jika tidak ingin mengubah):\n";
            cout << ">> ";

            getline(cin, newUsername);
            if (newUsername.empty())
            {
                break; // Tidak mengubah username jika input kosong
            }
            else if (newUsername != userToEdit.username)
            {
                bool usernameExists = false;
                for (const auto &pair : users)
                {
                    if (pair.second.username == newUsername)
                    {
                        usernameExists = true;
                        break;
                    }
                }
                if (usernameExists)
                {
                    cout << "Username sudah digunakan!\n";
                }
                else
                {
                    userToEdit.username = newUsername;
                    break;
                }
            }
            else
            {
                break; // Tidak mengubah username jika input sama dengan username lama
            }
        } while (true);

        // Edit password (dengan validasi)
        string newPassword;
        bool passwordValid = false;
        while (!passwordValid)
        {
            cout << "Masukkan password baru (minimal 8 karakter, harus mengandung huruf besar, huruf kecil, angka, dan simbol):\n";
            cout << ">> ";

            getline(cin, newPassword);

            passwordValid = isValidPassword(newPassword);
            if (!passwordValid)
            {
                cout << "Password tidak valid. Silakan coba lagi.\n";
            }
        }
        userToEdit.passwordHash = hash<string>{}(newPassword);

        // Edit pertanyaan keamanan
        cout << "Masukkan pertanyaan keamanan baru (kosongkan jika tidak ingin mengubah):\n";
        cout << ">> ";

        string newSecurityQuestion;
        getline(cin, newSecurityQuestion);
        if (!newSecurityQuestion.empty())
        {
            userToEdit.securityQuestion = newSecurityQuestion;
        }

        // Edit jawaban keamanan
        cout << "Masukkan jawaban keamanan baru (kosongkan jika tidak ingin mengubah):\n";
        cout << ">> ";

        string newSecurityAnswer;
        getline(cin, newSecurityAnswer);
        if (!newSecurityAnswer.empty())
        {
            userToEdit.securityAnswer = newSecurityAnswer;
        }

        saveUsers();
        cout << "User berhasil diubah!\n";
    }

    void deleteUser(const string &usernameToDelete)
    {
        auto it = users.begin();
        for (; it != users.end(); ++it)
        {
            if (it->second.username == usernameToDelete)
            {
                break; // Keluar dari loop jika user ditemukan
            }
        }

        if (it == users.end())
        {
            cout << "Username tidak ditemukan!\n";
            return;
        }

        string confirmDelete;
        cout << "Apakah Anda yakin ingin menghapus user " << usernameToDelete << "? (y/n): ";
        cin >> confirmDelete;

        if (confirmDelete == "y" || confirmDelete == "Y")
        {
            users.erase(it);
            saveUsers();
            cout << "User berhasil dihapus!\n";
        }
        else
        {
            cout << "Penghapusan user dibatalkan.\n";
        }
    }

    void listUsers()
    {
        if (users.empty())
        {
            cout << "Tidak ada user terdaftar.\n";
            return;
        }

        cout << "Daftar User:\n";
        for (const auto &pair : users)
        {
            const User &user = pair.second;
            cout << "Email: " << user.email << "\n"
                 << "Username: " << user.username << "\n"
                 << "Admin: " << (user.isAdmin ? "Ya" : "Tidak") << "\n"
                 << "---------------------\n";
        }
    }

    void handleAdminCommands()
    {
        string command;
        while (true)
        {
            auto [isLoggedIn, currentUser] = getCurrentUser();
            cout << currentUser.username << ":~$ ";
            cin >> command; // Membaca input perintah di sini

            stringstream ss(command);
            string action, targetUsername;
            ss >> action; // Baca kata pertama sebagai aksi

            if (action == "add")
            {
                registerUser();
            }
            else if (action == "edit")
            {
                if (ss >> targetUsername)
                {
                    editUser(targetUsername);
                }
                else
                {
                    cout << "Penggunaan: edit <username>\n";
                }
            }
            else if (action == "delete")
            {
                if (ss >> targetUsername)
                {
                    deleteUser(targetUsername);
                }
                else
                {
                    cout << "Penggunaan: delete <username>\n";
                }
            }
            else if (action == "list")
            {
                listUsers();
            }
            else if (action == "logout")
            {
                currentLoggedInUser = {false, ""};
                break;
            }
            else if (action == "help")
            {
                cout << "Available commands:\n";
                cout << "  list: Menampilkan daftar semua user\n";
                cout << "  add: Menambahkan user baru\n";
                cout << "  edit <username>: Mengubah data user\n";
                cout << "  delete <username>: Menghapus user\n";
                cout << "  logout: Keluar dari mode admin\n";
            }
            else
            {
                cout << "Perintah tidak valid.\n";
            }
        }
    }

    void handleUserCommand(LoginSystem &loginSystem)
    {
        string command;
        while (true)
        {
            auto [isLoggedIn, currentUser] = loginSystem.getCurrentUser();
            cout << currentUser.username << ":~$ ";
            getline(cin, command); // Menggunakan getline untuk membaca seluruh baris

            stringstream ss(command);
            string action;
            ss >> action; // Baca kata pertama sebagai aksi

            if (action == "whoami")
            {
                cout << currentUser.username << endl;
            }
            else if (action == "logout")
            {
                loginSystem.currentLoggedInUser = {false, ""};
                break;
            }
            else if (action == "help")
            { // Menambahkan perintah help
                cout << "Available commands:\n";
                cout << "  whoami: Menampilkan username pengguna saat ini\n";
                cout << "  logout: Keluar dari aplikasi\n";
            }
            else
            {
                cout << "Perintah tidak valid.\n";
            }
        }
    }

    pair<bool, User> getCurrentUser() const
    {
        if (currentLoggedInUser.first)
        {
            return {true, users.at(currentLoggedInUser.second)};
        }
        else
        {
            return {false, User{}};
        }
    }

private:
    unordered_map<string, User> users;
    string filename = "users.txt";
};

int main()
{
    LoginSystem loginSystem;
    int choice;

    do
    {
        cout << "\nMenu:\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "0. Keluar\n";
        cout << "Pilihan: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            if (loginSystem.login())
            { // Masuk ke halaman utama hanya jika login berhasil
                auto [isLoggedIn, currentUser] = loginSystem.getCurrentUser();
                if (currentUser.isAdmin)
                {
                    loginSystem.handleAdminCommands();
                }
                else
                {
                    loginSystem.handleUserCommand(loginSystem); // Panggil handleUserCommand untuk pengguna biasa
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
