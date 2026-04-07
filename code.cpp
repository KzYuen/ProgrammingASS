#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>

using namespace std;

// ============================================================
// STRUCTS
// ============================================================

struct student {
    string id;
    string name;
    string stud_email;
    string faculty;
    string phone;
    // vehicle field REMOVED — now managed via vehicle struct
    string password;
};

struct vehicle {
    string vehicleID; // e.g. "V1", "V2"
    string studentID; // owner
    string plate;     // e.g. "WXY1234"
    string type;      // e.g. "Car", "Motorcycle"
};

struct application {
    string studentID;
    string vehicleID; // NEW — which vehicle this pass is for
    int    months;
    string status;    // "pending", "approved", "rejected", "paid", "expired"
    string applyDate;
    string applyMonth;
};

struct admin {
    string adminID;
    string name;
    string password;
};

// ============================================================
// GLOBAL ARRAYS
// ============================================================

student     students[200];
vehicle     vehicles[600]; 
application apps[400];
admin       admins[20];

int studentCount = 0;
int vehicleCount = 0; 
int appsCount    = 0;
int adminCount   = 0;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

// File I/O
void LoadStudentsFromFile();
void LoadVehiclesFromFile();
void LoadApplicationsFromFile();
void LoadAdminFromFile();
void SaveStudentsToFile();
void SaveVehiclesToFile();
void SaveAdminToFile();
void SaveApplicationsToFile();

// Registration
void register_stud();
void register_admin();

// Vehicle management
void registerVehicle(string studentID);
void viewVehicles(string studentID);
void manageVehicles(int index_Student);
int  getVehiclesForStudent(string studentID, int results[], int maxCount);
int  FindVehicleIndexByID(string vehicleID);

// Applications
void register_application(int index_Student);
void renew_application(int index_Student);
void viewApplicationHistory(int index_Student);

// Profile
void ViewStudentProfile(int index_Student);
void ViewProfileAdmin(int index_Admin);
void UpdateStudentProfile(int index_Student);
void UpdatesAdminProfile(int index_Admin);
void AdminViewStudentProfile(int index_Admin); 

// Admin processing
void approveRejectApplication(int app_index);
void statisticsUsage(int index_Admin);

<<<<<<< main
//Helpers
int FindStudentIndexByID(string id);
int FindAdminIndexByID(string id);
int FindApplicationIndexByStudentID(string studentID);
bool hasPendingOrApproved(string studentID);
=======
// Helpers
int    FindStudentIndexByID(string id);
int    FindAdminIndexByID(string id);
bool   hasPendingOrApprovedForVehicle(string vehicleID);
bool   hasActivePaidPassForVehicle(string vehicleID);
>>>>>>> main
string getValidPassword();
int    safeInputInt(int min, int max);
void   CleanupExpiredPasses(string studentID);

// Alerts
void monthEndAlert(int index_Student);

// Menus
void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void MainMenu();

// Date helpers
string getCurrentMonth();
string getCurrentDate();
bool   isApproachingMonthEnd();

// Display helpers
void printLine(char c = '-', int n = 55);
void printHeader(string title);

// ============================================================
// MAIN
// ============================================================

int main() {
    LoadStudentsFromFile();
    LoadVehiclesFromFile();
    LoadApplicationsFromFile();
    LoadAdminFromFile();

    CleanupExpiredPasses(""); // clean up expired passes on startup
    MainMenu();

    SaveStudentsToFile();
    SaveVehiclesToFile();
    SaveApplicationsToFile();
    SaveAdminToFile();

    return 0;
}

// ============================================================
// DATE HELPERS
// ============================================================

string getCurrentDate() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

string getCurrentMonth() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[8];
    strftime(buf, sizeof(buf), "%Y-%m", now);
    return string(buf);
}

bool isApproachingMonthEnd() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    return (now->tm_mday >= 25);
}

// ============================================================
// DISPLAY HELPERS
// ============================================================

void printLine(char c, int n) {
    for (int i = 0; i < n; i++) cout << c;
    cout << "\n";
}

void printHeader(string title) {
    cout << "\n";
    printLine('=');
    cout << "  MPKJ Parking Pass System  |  " << title << "\n";
    printLine('=');
}

// ============================================================
// INPUT HELPERS
// ============================================================

int safeInputInt(int min, int max) {
    int n;
    while (true) {
        cin >> n;
        if (cin.fail() || n < min || n > max) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid number (" << min << "-" << max << "): ";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return n;
        }
    }
}

string getValidPassword() {
    string password;
    while (true) {
        cout << "Password (min 12 chars, upper+lower+digit+special): ";
        getline(cin >> ws, password);

        if (password.length() < 12) {
            cout << "Password must be at least 12 characters.\n";
            continue;
        }

        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if      (isupper(c)) hasUpper   = true;
            else if (islower(c)) hasLower   = true;
            else if (isdigit(c)) hasDigit   = true;
            else if (ispunct(c)) hasSpecial = true;
        }

        if (hasUpper && hasLower && hasDigit && hasSpecial) return password;
        cout << "Password must include uppercase, lowercase, number, and special character.\n";
    }
}

// ============================================================
// FILE I/O
// ============================================================

void LoadStudentsFromFile() {
    ifstream file("students.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || studentCount >= 200) continue;
        stringstream ss(line);
        student& s = students[studentCount];
        if (getline(ss, s.id,          '|') &&
            getline(ss, s.name,        '|') &&
            getline(ss, s.stud_email,  '|') &&
            getline(ss, s.faculty,     '|') &&
            getline(ss, s.phone,       '|') &&
            getline(ss, s.password))
            studentCount++;
    }
}

void LoadVehiclesFromFile() {
    ifstream file("vehicles.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || vehicleCount >= 600) continue;
        stringstream ss(line);
        vehicle& v = vehicles[vehicleCount];
        if (getline(ss, v.vehicleID, '|') &&
            getline(ss, v.studentID, '|') &&
            getline(ss, v.plate,     '|') &&
            getline(ss, v.type))
            vehicleCount++;
    }
}

void LoadApplicationsFromFile() {
    ifstream file("applications.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || appsCount >= 400) continue;
        stringstream ss(line);
        application temp;
        string monthsStr;
        if (getline(ss, temp.studentID,  '|') &&
            getline(ss, temp.vehicleID,  '|') &&
            getline(ss, monthsStr,       '|') &&
            getline(ss, temp.status,     '|') &&
            getline(ss, temp.applyDate,  '|') &&
            getline(ss, temp.applyMonth)) {
            try {
                temp.months = stoi(monthsStr);
                if (temp.months >= 1 && temp.months <= 3)
                    apps[appsCount++] = temp;
            } catch (...) {}
        }
    }
}

void LoadAdminFromFile() {
    ifstream file("admin.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || adminCount >= 20) continue;
        stringstream ss(line);
        admin& a = admins[adminCount];
        if (getline(ss, a.adminID,  '|') &&
            getline(ss, a.name,     '|') &&
            getline(ss, a.password))
            adminCount++;
    }
}

void SaveStudentsToFile() {
    ofstream file("students.txt");
    for (int i = 0; i < studentCount; i++)
        file << students[i].id         << "|"
             << students[i].name       << "|"
             << students[i].stud_email << "|"
             << students[i].faculty    << "|"
             << students[i].phone      << "|"
             << students[i].password   << "\n";
}

void SaveVehiclesToFile() {
    ofstream file("vehicles.txt");
    for (int i = 0; i < vehicleCount; i++)
        file << vehicles[i].vehicleID << "|"
             << vehicles[i].studentID << "|"
             << vehicles[i].plate     << "|"
             << vehicles[i].type      << "\n";
}

void SaveAdminToFile() {
    ofstream file("admin.txt");
    for (int i = 0; i < adminCount; i++)
        file << admins[i].adminID  << "|"
             << admins[i].name     << "|"
             << admins[i].password << "\n";
}

void SaveApplicationsToFile() {
    ofstream file("applications.txt");
<<<<<<< main

    for (int i = 0; i < appsCount; i++) {
        file << apps[i].studentID << "|"
     	     << apps[i].months << "|"
     	     << apps[i].status << "|"
             << apps[i].applyDate << "|"
             << apps [i].applyMonth << "\n";
    }
    file.close();
=======
    for (int i = 0; i < appsCount; i++)
        file << apps[i].studentID  << "|"
             << apps[i].vehicleID  << "|"
             << apps[i].months     << "|"
             << apps[i].status     << "|"
             << apps[i].applyDate  << "|"
             << apps[i].applyMonth << "\n";
>>>>>>> main
}

// ============================================================
// VEHICLE HELPERS
// ============================================================

// Fills results[] with array indices of all vehicles owned by studentID.
// Returns how many were found.
int getVehiclesForStudent(string studentID, int results[], int maxCount) {
    int count = 0;
    for (int i = 0; i < vehicleCount && count < maxCount; i++)
        if (vehicles[i].studentID == studentID)
            results[count++] = i;
    return count;
}

int FindVehicleIndexByID(string vehicleID) {
    for (int i = 0; i < vehicleCount; i++)
        if (vehicles[i].vehicleID == vehicleID) return i;
    return -1;
}

// Returns true if vehicleID has a pending or approved application
bool hasPendingOrApprovedForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++)
        if (apps[i].vehicleID == vehicleID &&
           (apps[i].status == "pending" || apps[i].status == "approved"))
            return true;
    return false;
}

<<<<<<< main
string getValidPassword() {
    string password;
    while (true) {
        cout << "Password must be at least 8 characters, include uppercase, lowercase, number and special character. Please enter password: ";
        getline(cin >> ws, password);

        if (password.length() < 8) {
            cout << "Password must be at least 8 characters long.\n";
            continue;
        }
=======
// Returns true if vehicleID has a currently paid (active) pass
bool hasActivePaidPassForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++)
        if (apps[i].vehicleID == vehicleID && apps[i].status == "paid")
            return true;
    return false;
}
>>>>>>> main

// ============================================================
// CLEANUP — auto-expire passes whose duration has elapsed
// ============================================================

void CleanupExpiredPasses(string studentID) {
    string cur = getCurrentMonth();
    int currentTotal = stoi(cur.substr(0, 4)) * 12 + stoi(cur.substr(5, 2));
    bool modified = false;

    for (int i = 0; i < appsCount; i++) {
        if (!studentID.empty() && apps[i].studentID != studentID) continue;
        if (apps[i].status != "paid" && apps[i].status != "approved") continue;

        try {
            int yr  = stoi(apps[i].applyMonth.substr(0, 4));
            int mo  = stoi(apps[i].applyMonth.substr(5, 2));
            int expiry = yr * 12 + mo + apps[i].months - 1;
            if (currentTotal > expiry) {
                apps[i].status = "expired";
                modified = true;
            }
        } catch (...) {}
    }
    if (modified) SaveApplicationsToFile();
}

// ============================================================
// REGISTRATION
// ============================================================

void register_stud() {
    if (studentCount >= 200) {
        cout << "  System full. Cannot register more students.\n";
        return;
    }
    student stud;
    printHeader("Student Registration");

    stud.id = "S" + to_string(studentCount + 1);
    cout << "  Generated ID: " << stud.id << "\n\n";

    cout << "  Name    : "; getline(cin >> ws, stud.name);
    cout << "  Email   : "; getline(cin >> ws, stud.stud_email);
    cout << "  Faculty : "; getline(cin >> ws, stud.faculty);
    cout << "  Phone   : "; getline(cin >> ws, stud.phone);

    stud.password = getValidPassword();
    students[studentCount++] = stud;
    SaveStudentsToFile();

    cout << "\n  Registration successful! ID: " << stud.id << "\n";
    cout << "  Please add your vehicle(s) from the Student Menu > Manage Vehicles.\n";
}

void register_admin() {
    if (adminCount >= 20) {
        cout << "  System full. Cannot register more admins.\n";
        return;
    }
    admin ADM;
    printHeader("Admin Registration");

    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "  Generated Admin ID: " << ADM.adminID << "\n\n";
    cout << "  Name: "; getline(cin >> ws, ADM.name);

    ADM.password = getValidPassword();
    admins[adminCount++] = ADM;
    SaveAdminToFile();

    cout << "\n  Admin registration successful!\n";
}

// ============================================================
// VEHICLE MANAGEMENT
// ============================================================

void registerVehicle(string studentID) {
    if (vehicleCount >= 600) {
        cout << "  Vehicle capacity reached.\n";
        return;
    }
    vehicle v;
    v.vehicleID = "V" + to_string(vehicleCount + 1);
    v.studentID = studentID;

    cout << "  Plate number (e.g. WXY1234) : "; getline(cin >> ws, v.plate);
    cout << "  Type (Car/Motorcycle/Van)   : "; getline(cin >> ws, v.type);

    vehicles[vehicleCount++] = v;
    SaveVehiclesToFile();

    cout << "  Vehicle registered. Vehicle ID: " << v.vehicleID << "\n";
}

void viewVehicles(string studentID) {
    int idx[20];
    int count = getVehiclesForStudent(studentID, idx, 20);

    if (count == 0) {
        cout << "  No vehicles registered.\n";
        return;
    }

    cout << "  " << left << setw(10) << "VehicleID"
                         << setw(16) << "Plate"
                         << setw(14) << "Type"
                         << "Active Pass\n";
    printLine();

    for (int i = 0; i < count; i++) {
        vehicle& v = vehicles[idx[i]];
        string pass = hasActivePaidPassForVehicle(v.vehicleID) ? "Yes" : "No";
        cout << "  " << left << setw(10) << v.vehicleID
                              << setw(16) << v.plate
                              << setw(14) << v.type
                              << pass << "\n";
    }
}

void manageVehicles(int index_Student) {
    string id = students[index_Student].id;
    while (true) {
        printHeader("Manage Vehicles");
        viewVehicles(id);
        printLine();
        cout << "  1. Add Vehicle\n";
        cout << "  2. Back\n";
        cout << "  Choice: ";
        int ch = safeInputInt(1, 2);
        if (ch == 1) registerVehicle(id);
        else break;
    }
}

// ============================================================
// APPLICATIONS — NEW
// ============================================================

void register_application(int index_Student) {
    printHeader("New Parking Pass Application");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);

    if (vCount == 0) {
        cout << "  No vehicles registered. Please add a vehicle first (Manage Vehicles).\n";
        return;
    }

    if (appsCount >= 400) {
        cout << "  System full. Cannot accept more applications.\n";
        return;
    }

    // Show vehicles with their current status
    cout << "  Select a vehicle to apply for:\n";
    printLine();
    for (int i = 0; i < vCount; i++) {
        vehicle& v = vehicles[idx[i]];
        string tag;
        if      (hasPendingOrApprovedForVehicle(v.vehicleID)) tag = " [Pending/Approved]";
        else if (hasActivePaidPassForVehicle(v.vehicleID))    tag = " [Active pass — use Renew]";
        else                                                   tag = " [No active pass]";
        cout << "  " << (i + 1) << ". " << v.plate
             << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick vehicle (1-" << vCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, vCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[idx[pick - 1]];

    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  This vehicle already has a pending or approved application. Please wait.\n";
        return;
    }
    if (hasActivePaidPassForVehicle(chosen.vehicleID)) {
        cout << "  This vehicle has an active pass. Use Option 5 (Renew) instead.\n";
        return;
    }

    application APP;
    APP.studentID  = studentID;
    APP.vehicleID  = chosen.vehicleID;
    APP.applyDate  = getCurrentDate();
    APP.applyMonth = getCurrentMonth();
    APP.status     = "pending";

    cout << "  Months (1-3): ";
    APP.months = safeInputInt(1, 3);

    apps[appsCount++] = APP;
    SaveApplicationsToFile();

    cout << "  Application for " << chosen.plate
         << " submitted on " << APP.applyDate << ". Pending admin approval.\n";
}

// ============================================================
// APPLICATIONS — RENEW
// ============================================================

void renew_application(int index_Student) {
    printHeader("Renew Parking Pass");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);

    if (vCount == 0) {
        cout << "  No vehicles registered.\n";
        return;
    }

    // Only list vehicles that have an active paid pass
    int eligibleIdx[20];
    int eligibleCount = 0;
    for (int i = 0; i < vCount; i++)
        if (hasActivePaidPassForVehicle(vehicles[idx[i]].vehicleID))
            eligibleIdx[eligibleCount++] = idx[i];

    if (eligibleCount == 0) {
        cout << "  No vehicle has an active (paid) pass to renew.\n";
        cout << "  Use New Application (Option 4) instead.\n";
        return;
    }

    cout << "  Select vehicle to renew:\n";
    printLine();
    for (int i = 0; i < eligibleCount; i++) {
        vehicle& v = vehicles[eligibleIdx[i]];
        string tag = hasPendingOrApprovedForVehicle(v.vehicleID) ? " [Renewal already pending]" : "";
        cout << "  " << (i + 1) << ". " << v.plate
             << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick (1-" << eligibleCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, eligibleCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[eligibleIdx[pick - 1]];

    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  This vehicle already has a pending renewal. Please wait for admin.\n";
        return;
    }

    // Find and display the current paid pass for this vehicle
    int paidIdx = -1;
    for (int i = appsCount - 1; i >= 0; i--)
        if (apps[i].vehicleID == chosen.vehicleID && apps[i].status == "paid")
            { paidIdx = i; break; }

    cout << "\n  Active pass details:\n";
    cout << "  Vehicle    : " << chosen.plate << " (" << chosen.type << ")\n";
    cout << "  Applied on : " << apps[paidIdx].applyDate  << "\n";
    cout << "  Duration   : " << apps[paidIdx].months     << " month(s)\n";
    printLine();

    char confirm;
    cout << "  Confirm renewal? (y/n): ";
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') {
        cout << "  Renewal cancelled.\n";
        return;
    }

    if (appsCount >= 400) {
        cout << "  System full.\n";
        return;
    }

    application renewal;
    renewal.studentID  = studentID;
    renewal.vehicleID  = chosen.vehicleID;
    renewal.applyDate  = getCurrentDate();
    renewal.applyMonth = getCurrentMonth();
    renewal.status     = "pending";

    cout << "  Months to renew (1-3): ";
    renewal.months = safeInputInt(1, 3);
    cout << "  Estimated cost: RM " << fixed << setprecision(2)
         << (renewal.months * 30.0) << "\n";

    apps[appsCount++] = renewal;
    SaveApplicationsToFile();

    cout << "\n  Renewal submitted on " << renewal.applyDate << ".\n";
    cout << "  Current pass stays active until admin approves this renewal.\n";
}

// ============================================================
// STUDENT PROFILE
// ============================================================

void ViewStudentProfile(int index_Student) {
    if (index_Student < 0 || index_Student >= studentCount) return;
    printHeader("Student Profile");
    student& s = students[index_Student];

    printLine();
    cout << "  ID      : " << s.id         << "\n";
    cout << "  Name    : " << s.name       << "\n";
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";
    cout << "  Phone   : " << s.phone      << "\n";
    printLine();

    // Vehicles table with linked application ID
    cout << "  REGISTERED VEHICLES\n";
    printLine();

    int idx[20];
    int vCount = getVehiclesForStudent(s.id, idx, 20);

    if (vCount == 0) {
        cout << "  No vehicles registered.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID"
                              << setw(14) << "Plate"
                              << setw(14) << "Type"
                              << "Linked Application\n";
        printLine();

        for (int i = 0; i < vCount; i++) {
            vehicle& v = vehicles[idx[i]];

            // Find most recent active (non-rejected/expired) application for this vehicle
            string appInfo = "None";
            for (int j = appsCount - 1; j >= 0; j--) {
                if (apps[j].vehicleID == v.vehicleID &&
                    apps[j].status != "rejected" && apps[j].status != "expired") {
                    appInfo = "AppIdx:" + to_string(j) + " [" + apps[j].status + "]";
                    break;
                }
            }
            cout << "  " << left << setw(10) << v.vehicleID
                                  << setw(14) << v.plate
                                  << setw(14) << v.type
                                  << appInfo  << "\n";
        }
    }
    printLine();
}

void UpdateStudentProfile(int index_Student) {
    printHeader("Update Profile");

    cout << "  Name    : "; getline(cin >> ws, students[index_Student].name);
    cout << "  Faculty : "; getline(cin >> ws, students[index_Student].faculty);
    cout << "  Phone   : "; getline(cin >> ws, students[index_Student].phone);

    char changePass;
    cout << "  Change Password? (y/n): ";
    cin >> changePass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (changePass == 'y' || changePass == 'Y')
        students[index_Student].password = getValidPassword();
    else
        cout << "  Password unchanged.\n";

    SaveStudentsToFile();
    cout << "  Profile updated successfully!\n";
}

// ============================================================
// ADMIN — VIEW ANY STUDENT PROFILE
// ============================================================

void AdminViewStudentProfile(int index_Admin) {
    printHeader("View Student Profile");

    if (studentCount == 0) {
        cout << "  No students registered.\n";
        return;
    }

    // List all students for the admin to pick
    cout << "  " << left << setw(6) << "No."
                         << setw(8) << "ID"
                         << setw(22) << "Name"
                         << "Faculty\n";
    printLine();
    for (int i = 0; i < studentCount; i++)
        cout << "  " << left << setw(6)  << (i + 1)
                              << setw(8)  << students[i].id
                              << setw(22) << students[i].name
                              << students[i].faculty << "\n";
    printLine();

    cout << "  Select student number (0 to cancel): ";
    int pick = safeInputInt(0, studentCount);
    if (pick == 0) return;

    student& s = students[pick - 1];

    printLine('=');
    cout << "  STUDENT: " << s.name << " (" << s.id << ")\n";
    printLine('=');
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";
    cout << "  Phone   : " << s.phone      << "\n";

    // Vehicles
    printLine();
    cout << "  REGISTERED VEHICLES\n";
    printLine();
    int idx[20];
    int vCount = getVehiclesForStudent(s.id, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles registered.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID"
                              << setw(16) << "Plate"
                              << setw(14) << "Type"
                              << "Active Pass\n";
        printLine();
        for (int i = 0; i < vCount; i++) {
            vehicle& v = vehicles[idx[i]];
            string pass = hasActivePaidPassForVehicle(v.vehicleID) ? "Yes" : "No";
            cout << "  " << left << setw(10) << v.vehicleID
                                  << setw(16) << v.plate
                                  << setw(14) << v.type
                                  << pass << "\n";
        }
    }

    // Full application history for this student
    printLine();
    cout << "  APPLICATION HISTORY\n";
    printLine();
    cout << "  " << left << setw(8)  << "AppIdx"
                         << setw(12) << "Vehicle"
                         << setw(12) << "Date"
                         << setw(8)  << "Months"
                         << "Status\n";
    printLine();

    bool anyApp = false;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != s.id) continue;
        anyApp = true;
        int vi = FindVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        cout << "  " << left << setw(8)  << i
                              << setw(12) << plate
                              << setw(12) << apps[i].applyDate
                              << setw(8)  << apps[i].months
                              << apps[i].status << "\n";
    }
    if (!anyApp) cout << "  No applications found.\n";
    printLine();
}

// ============================================================
// ADMIN — PROFILE UPDATE
// ============================================================

void UpdatesAdminProfile(int index_Admin) {
    printHeader("Update Admin Profile");

    cout << "  Name: "; getline(cin >> ws, admins[index_Admin].name);

    char changePass;
    cout << "  Change Password? (y/n): ";
    cin >> changePass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (changePass == 'y' || changePass == 'Y')
        admins[index_Admin].password = getValidPassword();
    else
        cout << "  Password unchanged.\n";

    SaveAdminToFile();
    cout << "  Admin profile updated successfully!\n";
}

// ============================================================
// APPLICATION HISTORY + PAYMENT
// ============================================================

void viewApplicationHistory(int index_Student) {
    printHeader("Application History");
    string id = students[index_Student].id;
    bool found = false;
    int count = 1;

    cout << "  " << left << setw(5)  << "No."
                         << setw(10) << "Vehicle"
                         << setw(12) << "Date"
                         << setw(8)  << "Months"
                         << "Status\n";
    printLine();

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != id) continue;
        found = true;

        int vi = FindVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;

        cout << "  " << left << setw(5)  << count++
                              << setw(10) << plate
                              << setw(12) << apps[i].applyDate
                              << setw(8)  << apps[i].months
                              << apps[i].status << "\n";

        // Prompt payment for approved applications
        if (apps[i].status == "approved") {
            printLine();
            cout << "  APPROVED — " << plate << " | "
                 << apps[i].months << " month(s) | RM "
                 << fixed << setprecision(2) << (apps[i].months * 30.0) << "\n";
            char pay;
            cout << "  Pay now? (y/n): ";
            cin >> pay;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (pay == 'y' || pay == 'Y') {
                apps[i].status = "paid";
                SaveApplicationsToFile();
                cout << "  Payment successful! Pass is now active.\n";
            } else {
                cout << "  Payment skipped. You can pay next time from this menu.\n";
            }
        }
    }

    if (!found) {
        cout << "  No applications found.\n";
        return;
    }

    // Spending summary
    double totalSpent  = 0;
    int    totalMonths = 0;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID == id && apps[i].status == "paid") {
            totalMonths += apps[i].months;
            totalSpent  += apps[i].months * 30.0;
        }
    }
<<<<<<< main

=======
    printLine();
    cout << "  SUMMARY\n";
    printLine();
    cout << "  Total months paid : " << totalMonths << "\n";
    cout << "  Total spent       : RM " << fixed << setprecision(2) << totalSpent << "\n";
>>>>>>> main
    printLine();
}

// ============================================================
// MONTH-END ALERT
// ============================================================

void monthEndAlert(int index_Student) {
    if (!isApproachingMonthEnd()) return;
    string id = students[index_Student].id;
    int idx[20];
    int vCount = getVehiclesForStudent(id, idx, 20);
    for (int i = 0; i < vCount; i++) {
        if (hasActivePaidPassForVehicle(vehicles[idx[i]].vehicleID)) {
            cout << "\n";
            printLine('*');
            cout << "  REMINDER: Month end approaching! Please renew to avoid disruption.\n";
            printLine('*');
            return;
        }
    }
}

// ============================================================
// ADMIN — PROCESS APPLICATIONS
// ============================================================

void ViewProfileAdmin(int index_Admin) {
    printHeader("Admin Process Applications");
    cout << "  Admin ID : " << admins[index_Admin].adminID << "\n";
    cout << "  Name     : " << admins[index_Admin].name    << "\n\n";
    cout << "  ------------ Pending Applications ------------\n";

    cout << "  " << left << setw(7)  << "Index"
                         << setw(9)  << "StudID"
                         << setw(12) << "Vehicle"
                         << setw(12) << "Date"
                         << setw(8)  << "Months"
                         << "Status\n";
    printLine();

    bool found = false;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].status != "pending") continue;
        found = true;
        int vehicle_index = FindVehicleIndexByID(apps[i].vehicleID);
        string plate = (vehicle_index != -1) ? vehicles[vehicle_index].plate : apps[i].vehicleID;
        cout << "  " << left << setw(7)  << i
                              << setw(9)  << apps[i].studentID
                              << setw(12) << plate
                              << setw(12) << apps[i].applyDate
                              << setw(8)  << apps[i].months
                              << apps[i].status << "\n";
    }

    if (!found) {
        cout << "  No pending applications.\n";
        return;
    }

    cout << "\n  Enter application index to process (-1 to cancel): ";
    int choice = safeInputInt(-1, appsCount - 1);
    if (choice == -1) return;

    if (choice >= 0 && choice < appsCount) {
        if (apps[choice].status == "pending")
            approveRejectApplication(choice);
        else
            cout << "  This application has already been processed.\n";
    } else {
        cout << "  Invalid index.\n";
    }
}

void approveRejectApplication(int app_index) {
    if (app_index < 0 || app_index >= appsCount) return;

    int vi = FindVehicleIndexByID(apps[app_index].vehicleID);
    string plate = (vi != -1) ? vehicles[vi].plate : apps[app_index].vehicleID;

    cout << "\n  Student  : " << apps[app_index].studentID << "\n";
    cout << "  Vehicle  : " << plate                       << "\n";
    cout << "  Duration : " << apps[app_index].months      << " month(s)\n";
    cout << "\n  1. Approve\n  2. Reject\n  Decision: ";
    int decision = safeInputInt(1, 2);

    if (decision == 1) {
        // If this is a renewal, expire the old paid pass for the same vehicle
        for (int i = 0; i < appsCount; i++) {
            if (i == app_index) continue;
            if (apps[i].vehicleID == apps[app_index].vehicleID && apps[i].status == "paid") {
                apps[i].status = "expired";
            }
        }
        apps[app_index].status = "approved";
        cout << "  Application approved.\n";
    } else {
        apps[app_index].status = "rejected";
        cout << "  Application rejected.\n";
    }
    SaveApplicationsToFile();
}

// ============================================================
// STATISTICS
// ============================================================

void statisticsUsage(int index_Admin) {
    printHeader("Admin: Statistics & Analytics");
    cout << "  Admin: " << admins[index_Admin].name << "\n\n";

    int total = appsCount;
    int approved = 0, rejected = 0, pending = 0, paid = 0, expired = 0;
    int durationCount[4] = {0};

    string facultyNames[20]; int facultyApps[20] = {0}, facultyApproved[20] = {0}, facultyCount = 0;
    string monthLabels[12];  int monthApps[12] = {0};

    {
        time_t t = time(0); struct tm* now = localtime(&t);
        for (int m = 11; m >= 0; m--) {
            int mo = now->tm_mon - m, yr = now->tm_year + 1900;
            while (mo < 0) { mo += 12; yr--; }
            char buf[16]; sprintf(buf, "%04d-%02d", yr, mo + 1);
            monthLabels[11 - m] = string(buf);
        }
    }

    for (int i = 0; i < appsCount; i++) {
        if      (apps[i].status == "approved") approved++;
        else if (apps[i].status == "rejected") rejected++;
        else if (apps[i].status == "pending")  pending++;
        else if (apps[i].status == "paid")     paid++;
        else if (apps[i].status == "expired")  expired++;

        if (apps[i].months >= 1 && apps[i].months <= 3)
            durationCount[apps[i].months]++;

        int si = FindStudentIndexByID(apps[i].studentID);
        if (si != -1) {
            string fac = students[si].faculty;
            int fi = -1;
            for (int f = 0; f < facultyCount; f++)
                if (facultyNames[f] == fac) { fi = f; break; }
            if (fi == -1 && facultyCount < 20) { fi = facultyCount; facultyNames[facultyCount++] = fac; }
            if (fi != -1) {
                facultyApps[fi]++;
                if (apps[i].status == "approved" || apps[i].status == "paid") facultyApproved[fi]++;
            }
        }
        for (int m = 0; m < 12; m++)
            if (apps[i].applyMonth == monthLabels[m]) { monthApps[m]++; break; }
    }

    double utilRate = studentCount > 0 ? (double)(paid + approved) / studentCount * 100.0 : 0.0;

    printLine();
    cout << "  OVERALL SUMMARY\n"; printLine();
    cout << "  Total students     : " << studentCount << "\n";
    cout << "  Total vehicles     : " << vehicleCount << "\n";
    cout << "  Total applications : " << total        << "\n";
    cout << "  Approved (unpaid)  : " << approved     << "\n";
    cout << "  Paid / Active      : " << paid         << "\n";
    cout << "  Rejected           : " << rejected     << "\n";
    cout << "  Pending            : " << pending      << "\n";
    cout << "  Expired            : " << expired      << "\n";
    cout << "  Utilisation rate   : " << fixed << setprecision(1) << utilRate << "%\n";

    printLine();
    cout << "  BY DURATION\n"; printLine();
    for (int m = 1; m <= 3; m++) {
        cout << "  " << m << " month(s) : ";
        for (int j = 0; j < durationCount[m] && j < 40; j++) cout << "#";
        if (durationCount[m] > 40) cout << "+";
        cout << "  (" << durationCount[m] << ")\n";
    }

    printLine();
    cout << "  MONTHLY TREND (last 12 months)\n"; printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) if (monthApps[m] > maxBar) maxBar = monthApps[m];
    for (int m = 0; m < 12; m++) {
        cout << "  " << monthLabels[m] << " : ";
        int bar = (monthApps[m] * 30) / maxBar;
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << ")\n";
    }

    printLine();
    cout << "  BY FACULTY\n"; printLine();
    cout << "  " << left << setw(14) << "Faculty" << setw(10) << "Applied" << setw(12) << "Approved" << "Avg/month\n";
    printLine();
    for (int f = 0; f < facultyCount; f++) {
        double avg = (double)facultyApps[f] / 12.0;
        cout << "  " << left << setw(14) << facultyNames[f]
             << setw(10) << facultyApps[f]
             << setw(12) << facultyApproved[f]
             << fixed << setprecision(1) << avg << "\n";
    }
    printLine();
    cout << "\n  [Note] Analytics support negotiation with MPKJ for better student rates.\n";
}

// ============================================================
// FIND HELPERS
// ============================================================

int FindStudentIndexByID(string id) {
    for (int i = 0; i < studentCount; i++)
        if (students[i].id == id) return i;
    return -1;
}

int FindAdminIndexByID(string id) {
    for (int i = 0; i < adminCount; i++)
        if (admins[i].adminID == id) return i;
    return -1;
}

// ============================================================
// MENUS
// ============================================================

void studentMenu(int index_Student) {
    monthEndAlert(index_Student);
    int choice;
    while (true) {
        printHeader("Student Menu");
        cout << "  1. View Profile\n";
        cout << "  2. Update Profile\n";
        cout << "  3. Manage Vehicles\n";
        cout << "  4. New Application\n";
        cout << "  5. Renew Pass\n";
        cout << "  6. View Application Status / Pay\n";
        cout << "  7. Logout\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 7);

        if      (choice == 1) ViewStudentProfile(index_Student);
        else if (choice == 2) UpdateStudentProfile(index_Student);
        else if (choice == 3) manageVehicles(index_Student);
        else if (choice == 4) register_application(index_Student);
        else if (choice == 5) renew_application(index_Student);
        else if (choice == 6) viewApplicationHistory(index_Student);
        else if (choice == 7) { cout << "  Logging out...\n"; break; }
    }
}

void adminMenu(int index_Admin) {
    int choice;
    while (true) {
        printHeader("Admin Menu");
        cout << "  1. Process Pending Applications\n";
        cout << "  2. View Student Profile\n";
        cout << "  3. Statistics & Analytics\n";
        cout << "  4. Update Profile\n";
        cout << "  5. Logout\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 5);

        if      (choice == 1) ViewProfileAdmin(index_Admin);
        else if (choice == 2) AdminViewStudentProfile(index_Admin);
        else if (choice == 3) statisticsUsage(index_Admin);
        else if (choice == 4) UpdatesAdminProfile(index_Admin);
        else if (choice == 5) { cout << "  Logging out...\n"; break; }
    }
}

<<<<<<< main
// ============================================================
// MAIN MENU
// ============================================================

void MainMenu(){
=======
void MainMenu() {
>>>>>>> main
    int choice;
    string password, id;

    while (true) {
        printHeader("Main Menu");
        cout << "  1. Student Sign Up\n";
        cout << "  2. Admin Sign Up\n";
        cout << "  3. Student Login\n";
        cout << "  4. Admin Login\n";
        cout << "  5. Exit\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 5);

        if (choice == 1) {
            register_stud();
        } else if (choice == 2) {
            register_admin();
<<<<<<< main
        }else if(choice == 3){

            while(true){

                cout << "Enter Student ID: " << endl;
                cin >> student_id;
            
                int index_Student = FindStudentIndexByID(student_id);

                if(index_Student == -1) {
                    cout << "Invalid Student ID. Please try again. " << endl;
                    continue;
                }
            
                int attempts = 0;
                bool loginOK  = false;

                while(attempts < 3){
                    cout << "Enter Password: " << endl;
                    getline(cin >> ws, password_Student);

                    if(students[index_Student].password == password_Student){
                        cout << "Login Successful!" << endl;
                        studentMenu(index_Student);
                        loginOK = true;
                        break;
                    }else{
                        cout << "Invalid Password. Please try again. " << endl; 
                        attempts++;
=======
        } else if (choice == 3) {
            while (true) {
                cout << "  Student ID: "; cin >> id;
                int si = FindStudentIndexByID(id);
                if (si == -1) { cout << "  Invalid ID. Try again.\n"; continue; }

                bool ok = false;
                for (int att = 0; att < 3; att++) {
                    cout << "  Password: ";
                    getline(cin >> ws, password);
                    if (students[si].password == password) {
                        cout << "  Login Successful!\n";
                        CleanupExpiredPasses(students[si].id);
                        studentMenu(si);
                        ok = true; break;
>>>>>>> main
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
<<<<<<< main

                if(!loginOK){
                    cout << "Too many failed attempts. Returning to main menu.\n";
                }

                break;
            }

        }else if(choice == 4){

            while(true){
            cout << "Enter Admin ID: " << endl;
            cin >> Admin_id;

            int index_Admin = FindAdminIndexByID(Admin_id);

            if(index_Admin == -1){
                cout << "Invalid Admin ID. Please try again. " << endl;
                continue;
            }
            
            int attempts = 0;
            bool loginOK  = false;

            while(attempts < 3){
                cout << "Enter Password: " << endl;
                getline(cin >> ws, password_Admin);

                if(admins[index_Admin].password == password_Admin){
                    cout << "Login Successful!" << endl;
                    adminMenu(index_Admin);
                    loginOK = true;
                    break;
                }else{
                    cout << "Invalid Password. Please try again. " << endl; 
                    attempts++;
=======
                if (!ok) cout << "  Too many failed attempts. Returning to main menu.\n";
                break;
            }
        } else if (choice == 4) {
            while (true) {
                cout << "  Admin ID: "; cin >> id;
                int ai = FindAdminIndexByID(id);
                if (ai == -1) { cout << "  Invalid ID. Try again.\n"; continue; }

                bool ok = false;
                for (int att = 0; att < 3; att++) {
                    cout << "  Password: ";
                    getline(cin >> ws, password);
                    if (admins[ai].password == password) {
                        cout << "  Login Successful!\n";
                        adminMenu(ai);
                        ok = true; break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
>>>>>>> main
                }
                if (!ok) cout << "  Too many failed attempts. Returning to main menu.\n";
                break;
            }
<<<<<<< main
            if(!loginOK){
                cout << "Too many failed attempts. Returning to main menu.\n";
            }
=======
        } else if (choice == 5) {
            cout << "  Exiting...\n";
>>>>>>> main
            break;
        }
    }
}