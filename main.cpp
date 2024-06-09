#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <limits>
#include <vector>
#include <ctime>

using namespace std;

class User
{
private:
    string email;
    string username;
    string securityQuestion;
    string securityAnswer;
    unsigned long long passwordHash;
    bool isAdmin;

public:
    // Setter methods
    void setEmail(const string &newEmail) { email = newEmail; }
    void setUsername(const string &newUsername) { username = newUsername; }
    void setSecurityQuestion(const string &newSecurityQuestion) { securityQuestion = newSecurityQuestion; }
    void setSecurityAnswer(const string &newSecurityAnswer) { securityAnswer = newSecurityAnswer; }
    void setPasswordHash(unsigned long long newPasswordHash) { passwordHash = newPasswordHash; }
    void setIsAdmin(bool newIsAdmin) { isAdmin = newIsAdmin; }

    // Getter methods
    string getEmail() const { return email; }
    string getUsername() const { return username; }
    string getSecurityQuestion() const { return securityQuestion; }
    string getSecurityAnswer() const { return securityAnswer; }
    unsigned long long getPasswordHash() const { return passwordHash; }
    bool getIsAdmin() const { return isAdmin; }
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

protected:
    pair<bool, string> currentLoggedInUser = {false, ""};

public:
    LoginSystem()
    {
        loadUsers();
    }

    virtual ~LoginSystem() = default;

    void loadUsers()
    {
        ifstream infile(filename);
        string line;
        while (getline(infile, line))
        {
            stringstream ss(line);
            User user;
            string passwordHashStr;

            string email, username, securityQuestion, securityAnswer;
            bool isAdmin;
            unsigned long long passwordHash;

            getline(ss, email, ':');
            getline(ss, username, ':');
            getline(ss, securityQuestion, ':');
            getline(ss, securityAnswer, ':');
            getline(ss, passwordHashStr, ':');
            ss >> isAdmin;

            passwordHash = stoull(passwordHashStr);

            user.setEmail(email);
            user.setUsername(username);
            user.setSecurityQuestion(securityQuestion);
            user.setSecurityAnswer(securityAnswer);
            user.setPasswordHash(passwordHash);
            user.setIsAdmin(isAdmin);

            users[email] = user;
        }
        infile.close();
    }

    void saveUsers()
    {
        ofstream outfile(filename);
        for (const auto &pair : users)
        {
            const User &user = pair.second;
            outfile << user.getEmail() << ":" << user.getUsername() << ":" << user.getSecurityQuestion() << ":"
                    << user.getSecurityAnswer() << ":" << user.getPasswordHash() << ":" << user.getIsAdmin() << endl;
        }
        outfile.close();
    }

    virtual void registerUser()
    {
        User newUser;
        string email, username, securityQuestion, securityAnswer, password;
        bool isAdmin;

        cout << "Masukkan email:\n";
        cout << ">> ";
        cin >> email;

        if (users.count(email) > 0)
        {
            cout << "Email sudah terdaftar!\n";
            return;
        }
        isAdmin = (email.find("@admin.com") != string::npos);

        cout << "Masukkan username:\n";
        cout << ">> ";
        cin >> username;

        cout << "Masukkan pertanyaan keamanan:\n";
        cout << ">> ";
        cin.ignore();
        getline(cin, securityQuestion);

        cout << "Masukkan jawaban keamanan:\n";
        cout << ">> ";
        getline(cin, securityAnswer);

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
        unsigned long long passwordHash = hash<string>{}(password);

        newUser.setEmail(email);
        newUser.setUsername(username);
        newUser.setSecurityQuestion(securityQuestion);
        newUser.setSecurityAnswer(securityAnswer);
        newUser.setPasswordHash(passwordHash);
        newUser.setIsAdmin(isAdmin);

        users[email] = newUser;
        saveUsers();
        cout << "Registrasi berhasil!\n";
    }

    virtual bool login()
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
            if (users[email].getPasswordHash() == hash<string>{}(password))
            {
                cout << "Login berhasil!\n";
                cout << "Selamat datang, " << (users[email].getIsAdmin() ? "Admin" : "User") << "!\n";
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

    virtual bool logout()
    {
        if (currentLoggedInUser.first)
        {
            currentLoggedInUser = {false, ""};
            cout << "Logout berhasil!\n";
            return true;
        }
        else
        {
            cout << "Tidak ada user yang sedang login.\n";
            return false;
        }
    }

    virtual bool resetPassword(const string &email)
    {
        if (users.count(email) == 0)
        {
            cout << "Email tidak terdaftar!\n";
            return false;
        }

        string answer;
        cout << users[email].getSecurityQuestion() << ": ";
        cin.ignore();
        getline(cin, answer);

        if (answer == users[email].getSecurityAnswer())
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
            users[email].setPasswordHash(hash<string>{}(newPassword));
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

    virtual void changePassword(const string &email)
    {
        if (users.count(email) == 0)
        {
            cout << "Email tidak ditemukan!\n";
            return;
        }

        string currentPassword;
        cout << "Masukkan password saat ini:\n";
        cout << ">> ";
        cin >> currentPassword;

        if (users[email].getPasswordHash() != hash<string>{}(currentPassword))
        {
            cout << "Password saat ini salah!\n";
            return;
        }

        string newPassword;
        bool validPassword = false;
        while (!validPassword)
        {
            cout << "Masukkan password baru (minimal 8 karakter, mengandung huruf besar, huruf kecil, angka, dan simbol):\n";
            cout << ">> ";
            cin >> newPassword;

            validPassword = isValidPassword(newPassword);
            if (!validPassword)
            {
                cout << "Password tidak valid. Silahkan coba lagi!\n";
            }
        }
        users[email].setPasswordHash(hash<string>{}(newPassword));
        saveUsers();
        cout << "Password berhasil diubah!\n";
    }

    virtual void editUser(const string &emailToEdit)
    {
        if (users.count(emailToEdit) == 0)
        {
            cout << "Email tidak ditemukan!\n";
            return;
        }

        User &userToEdit = users[emailToEdit]; // Reference ke user yang akan diedit

        cout << "\nEdit User:\n";

        // Edit email (dengan validasi)
        string newEmail;
        do
        {
            cout << "Masukkan email baru (kosongkan jika tidak ingin mengubah):\n";
            cout << ">> ";
            cin.ignore();
            getline(cin, newEmail);
            if (newEmail.empty())
            {
                break; // Tidak mengubah email jika input kosong
            }
            else if (users.count(newEmail) > 0 && newEmail != userToEdit.getEmail())
            {
                cout << "Email sudah terdaftar!\n";
            }
            else
            {
                userToEdit.setEmail(newEmail);
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
            else if (newUsername != userToEdit.getUsername())
            {
                bool usernameExists = false;
                for (const auto &pair : users)
                {
                    if (pair.second.getUsername() == newUsername)
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
                    userToEdit.setUsername(newUsername);
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
        userToEdit.setPasswordHash(hash<string>{}(newPassword));

        // Edit pertanyaan keamanan
        cout << "Masukkan pertanyaan keamanan baru (kosongkan jika tidak ingin mengubah):\n";
        cout << ">> ";
        string newSecurityQuestion;
        getline(cin, newSecurityQuestion);
        if (!newSecurityQuestion.empty())
        {
            userToEdit.setSecurityQuestion(newSecurityQuestion);
        }

        // Edit jawaban keamanan
        cout << "Masukkan jawaban keamanan baru (kosongkan jika tidak ingin mengubah):\n";
        cout << ">> ";
        string newSecurityAnswer;
        getline(cin, newSecurityAnswer);
        if (!newSecurityAnswer.empty())
        {
            userToEdit.setSecurityAnswer(newSecurityAnswer);
        }

        saveUsers();
        cout << "User berhasil diubah!\n";
    }

    virtual void deleteUser(const string &emailToDelete)
    {
        if (users.count(emailToDelete) == 0)
        {
            cout << "Email tidak ditemukan!\n";
            return;
        }

        string confirmDelete;
        cout << "Apakah Anda yakin ingin menghapus user " << emailToDelete << "? (y/n): ";
        cin >> confirmDelete;

        if (confirmDelete == "y" || confirmDelete == "Y")
        {
            users.erase(emailToDelete);
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
            cout << "Email: " << user.getEmail() << "\n"
                 << "Username: " << user.getUsername() << "\n"
                 << "Admin: " << (user.getIsAdmin() ? "Ya" : "Tidak") << "\n"
                 << "---------------------\n";
        }
    }

    virtual void handleAdminCommands()
    {
        string command;
        while (true)
        {
            auto [isLoggedIn, currentUser] = getCurrentUser();
            cout << currentUser.getUsername() << ":~$ ";
            cin >> command; // Membaca input perintah di sini

            if (command == "add")
            {
                registerUser();
            }
            else if (command == "edit")
            {
                vector<string> emails = listEmails();
                string emailToEdit;
                cout << "Masukkan email yang ingin di edit: ";
                cin >> emailToEdit;

                if (find(emails.begin(), emails.end(), emailToEdit) != emails.end())
                {
                    cin.ignore(); // Membersihkan input buffer sebelum memanggil getline
                    editUser(emailToEdit);
                }
                else
                {
                    cout << "Email tidak ditemukan!\n";
                }
            }
            else if (command == "delete")
            {
                vector<string> emails = listEmails();
                string emailToDelete;
                cout << "Masukkan email yang ingin dihapus: ";
                cin >> emailToDelete;

                if (find(emails.begin(), emails.end(), emailToDelete) != emails.end())
                {
                    deleteUser(emailToDelete);
                }
                else
                {
                    cout << "Email tidak ditemukan!\n";
                }
            }
            else if (command == "list")
            {
                listUsers();
            }
            else if (command == "changepassword")
            {
                changePassword(currentUser.getEmail());
            }
            else if (command == "logout")
            {
                if (logout())
                {
                    break;
                }
            }
            else if (command == "help")
            {
                cout << "Available commands:\n";
                cout << "  list: Menampilkan daftar semua user\n";
                cout << "  add: Menambahkan user baru\n";
                cout << "  edit: Mengubah data user\n";
                cout << "  delete: Menghapus user\n";
                cout << "  changepassword: Mengganti password\n";
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
            cout << currentUser.getUsername() << ":~$ ";
            cin >> command;

            stringstream ss(command);
            string action;
            ss >> action; // Baca kata pertama sebagai aksi

            if (action == "whoami")
            {
                cout << currentUser.getUsername() << endl;
            }
            else if (action == "changepassword")
            {
                loginSystem.changePassword(currentUser.getEmail());
            }
            else if (action == "logout")
            {
                if (loginSystem.logout())
                {
                    break;
                }
            }
            else if (action == "help")
            { // Menambahkan perintah help
                cout << "Available commands:\n";
                cout << "  whoami: Menampilkan username pengguna saat ini\n";
                cout << "  changepassword: Mengganti password\n";
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

    vector<string> listEmails()
    {
        vector<string> emails;
        cout << "Daftar Email Pengguna:\n";
        for (const auto &pair : users)
        {
            cout << pair.second.getEmail() << "\n";
            emails.push_back(pair.second.getEmail());
        }
        return emails;
    }

protected:
    void logActivity(const string &activity)
    {
        ofstream logfile("activity.log", ios_base::app);
        time_t now = time(0);
        string dt = ctime(&now);
        dt.pop_back(); // Remove the newline character
        logfile << dt << " - " << activity << endl;
        logfile.close();
    }

private:
    unordered_map<string, User> users;
    string filename = "users.txt";
};

class LoggingLoginSystem : public LoginSystem
{
public:
    bool login() override
    {
        bool result = LoginSystem::login();
        if (result)
        {
            logActivity("User logged in: " + currentLoggedInUser.second);
        }
        return result;
    }

    bool logout() override
    {
        bool result = LoginSystem::logout();
        if (result)
        {
            logActivity("User logged out: " + currentLoggedInUser.second);
        }
        return result;
    }

    void registerUser() override
    {
        LoginSystem::registerUser();
        logActivity("New user registered");
    }

    void editUser(const string &emailToEdit) override
    {
        LoginSystem::editUser(emailToEdit);
        logActivity("User edited: " + emailToEdit);
    }

    void changePassword(const string &email) override
    {
        LoginSystem::changePassword(email);
        logActivity("Password changed for user: " + email);
    }

    void handleAdminCommands() override
    {
        logActivity("Admin commands session started for user: " + currentLoggedInUser.second);
        LoginSystem::handleAdminCommands();
        logActivity("Admin commands session ended for user: " + currentLoggedInUser.second);
    }
};

int main()
{
    LoggingLoginSystem loginSystem;
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
                if (currentUser.getIsAdmin())
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
