// ============================================================
// MPKJ Monthly Car Parking Pass Management System
// Course      : UECS1104 Programming and Problem Solving
// Semester    : February 2026
// Group Leader: Kazin Yuen Kah Zing (2505264)
// Members     : Cynthia Chan Ern Tung (2505570)
// Members     : Wallace Chan Wai Hoong (2500582)
// Members     : Lim Yu Feng (2303363)
// ============================================================
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

// ============================================================
// STRUCTS
// ============================================================

struct student {
    string id;
    string name;
    string stud_email;
    string faculty;   // validated: A-K
    string phone;
    string password;
    bool   deleted;   // soft-delete: true = account deactivated, records kept
};

struct vehicle {
    string vehicleID; // "V1", "V2", ...
    string studentID;
    string plate;
    string type;      // Car / Motorcycle / Van
};

struct application {
    string studentID;  // owning student's ID
    string vehicleID;  // vehicle this pass is for
    string faculty;    // snapshot of faculty at application time
    int    months;     // duration (1-3)
    string status;     // pending | approved | rejected | expired
    string paymentStatus;  // unpaid | paid | 
    string applyDate;  // "YYYY-MM-DD" — submission date
    string applyMonth; // "YYYY-MM"    — used for analytics
    string expiryDate; // "YYYY-MM-DD" — last valid day of the pass
                       //   New pass   : set at submission time
                       //   Renewal    : set at PAYMENT time (so we can extend
                       //                from the current live pass's expiry)
};

struct admin {
    string adminID;
    string name;
    string email;
    string phone;
    string password;
};

// ============================================================
// GLOBALS
// ============================================================

student     students[200];
vehicle     vehicles[600];
application apps[400];
admin       admins[20];

int studentCount = 0;
int vehicleCount = 0;
int appsCount    = 0;
int adminCount   = 0;

// Faculty table — A to K (11 faculties).
const string FAC_CODES[11]  = { "A","B","C","D","E","F","G","H","I","J","K" };
const string FAC_LABELS[11] = {
    "A- M. Kandiah Faculty of Medicine and Health Sciences (MKF FMHS)",
    "B-Lee Kong Chian Faculty of Engineering and Science (LKC FES)",
    "C-Institute of Management and Leadership Development (IMLD)",
    "D-Institute of Postgraduate Studies and Research (IPSR)",
    "E-Centre for Corporate and Community Development (CCCD)",
    "F-Faculty of Accountancy and Management (FAM)",
    "G-Faculty of Chinese Studies (FCS)",
    "H-Faculty of Education (FOE)",
    "I-Centre for Foundation Studies (CFS)",
    "J-Faculty of Creative Industries (FCI)",
    "K-Institute of International Education (IIE)"
};
const int FAC_COUNT = 11;

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

void loadStudentsFromFile();
void loadVehiclesFromFile();
void loadApplicationsFromFile();
void loadAdminFromFile();
void saveStudentsToFile();
void saveVehiclesToFile();
void saveAdminToFile();
void saveApplicationsToFile();

void registerStudent();
void registerAdmin();

void registerVehicle(string studentID);
void viewVehicles(string studentID);
void manageVehicles(int index_Student);
int  getVehiclesForStudent(string studentID, int results[], int maxCount);
int  findVehicleIndexByID(string vehicleID);

void registerApplication(int index_Student);
void renewApplication(int index_Student);
void viewApplicationHistory(int index_Student);

void viewStudentProfile(int index_Student);
void updateStudentProfile(int index_Student);
void updateAdminProfile(int index_Admin);
void adminViewStudentProfile(int index_Admin);

void viewProfileAdmin(int index_Admin);
void approveRejectApplication(int app_index);
void statisticsUsage(int index_Admin);
void generateSummaryReport(int index_Admin);
void deleteStudent(int index_Admin);

int    findStudentIndexByID(string id);
int    findAdminIndexByID(string id);
bool   hasPendingOrApprovedForVehicle(string vehicleID);
bool   hasActivePaidPassForVehicle(string vehicleID);
int    facultyIndex(string fac);
bool   isRenewalApp(int appIdx);
string getValidPassword();
string getValidFaculty();
int    safeInputInt(int min, int max);
void   cleanupExpiredPasses(string studentID);

void expiryAlert(int index_Student);   // 7-day expiry reminder on login

void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void mainMenu();

string getCurrentMonth();
string getCurrentDate();

// Date arithmetic
int    daysInMonth(int year, int month);
string lastDayOfMonth(int year, int month);
string calcExpiryNewPass(string applyMonth, int months);
string addMonthsToExpiry(string expiryDate, int months);
int    daysBetween(string fromDate, string toDate);

void printLine(char c = '-', int n = 60);
void printHeader(string title);
void buildMonthLabels(string labels[], int count);

// ============================================================
// MAIN
// ============================================================

int main() {
    loadStudentsFromFile();
    loadVehiclesFromFile();
    loadApplicationsFromFile();
    loadAdminFromFile();

    cleanupExpiredPasses("");
    mainMenu();

    saveStudentsToFile();
    saveVehiclesToFile();
    saveApplicationsToFile();
    saveAdminToFile();

    return 0;
}

// ============================================================
// DATE HELPERS — current date / month
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

void buildMonthLabels(string labels[], int count) {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    for (int m = count - 1; m >= 0; m--) {
        int mo = now->tm_mon - m;
        int yr = now->tm_year + 1900;
        while (mo < 0) { mo += 12; yr--; }
        char buf[8];
        sprintf(buf, "%04d-%02d", yr, mo + 1);
        labels[count - 1 - m] = string(buf);
    }
}

// ============================================================
// DATE ARITHMETIC HELPERS
// ============================================================

// Returns the number of days in a given month, leap-year aware.
int daysInMonth(int year, int month) {
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }
    const int d[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    return d[month];
}

// Returns the last calendar day of a month as "YYYY-MM-DD".
string lastDayOfMonth(int year, int month) {
    char buf[11];
    sprintf(buf, "%04d-%02d-%02d", year, month, daysInMonth(year, month));
    return string(buf);
}

// Expiry date for a NEW pass applied in applyMonth for N months.
// The pass covers [applyMonth, applyMonth + months - 1].
// e.g. applyMonth="2026-04", months=3  ->  covers Apr/May/Jun  ->  "2026-06-30"
string calcExpiryNewPass(string applyMonth, int months) {
    int yr = stoi(applyMonth.substr(0, 4));
    int mo = stoi(applyMonth.substr(5, 2));
    mo += months - 1;
    while (mo > 12) { mo -= 12; yr++; }
    return lastDayOfMonth(yr, mo);
}

// Extend an existing expiry date by N full months.
// e.g. expiryDate="2026-04-30", months=3  ->  last day of July 2026  ->  "2026-07-31"
string addMonthsToExpiry(string expiryDate, int months) {
    int yr = stoi(expiryDate.substr(0, 4));
    int mo = stoi(expiryDate.substr(5, 2));
    mo += months;
    while (mo > 12) { mo -= 12; yr++; }
    return lastDayOfMonth(yr, mo);
}

// Days from fromDate to toDate.  Positive = toDate is in the future.
int daysBetween(string fromDate, string toDate) {
    struct tm t1 = {}, t2 = {};
    t1.tm_year = stoi(fromDate.substr(0, 4)) - 1900;
    t1.tm_mon  = stoi(fromDate.substr(5, 2)) - 1;
    t1.tm_mday = stoi(fromDate.substr(8, 2));
    t2.tm_year = stoi(toDate.substr(0, 4)) - 1900;
    t2.tm_mon  = stoi(toDate.substr(5, 2)) - 1;
    t2.tm_mday = stoi(toDate.substr(8, 2));
    time_t time1 = mktime(&t1);
    time_t time2 = mktime(&t2);
    return static_cast<int>(difftime(time2, time1) / 86400.0);
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
            cout << "  Please enter a number (" << min << "-" << max << "): ";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return n;
        }
    }
}

string getValidPassword() {
    string pw;
    while (true) {
        cout << "  Password (min 12 chars, upper + lower + digit + special): ";
        getline(cin >> ws, pw);
        if ((int)pw.length() < 12) {
            cout << "  Password must be at least 12 characters.\n"; continue;
        }
        bool hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : pw) {
            if      (isupper(c)) hasU = true;
            else if (islower(c)) hasL = true;
            else if (isdigit(c)) hasD = true;
            else if (ispunct(c)) hasS = true;
        }
        if (hasU && hasL && hasD && hasS) return pw;
        cout << "  Must include uppercase, lowercase, digit, and special character.\n";
    }
}

string getAbbrevName(int fi) {
    string label = FAC_LABELS[fi];
    size_t open  = label.rfind('(');
    size_t close = label.rfind(')');
    if (open != string::npos && close != string::npos && close > open) {
        return label.substr(open + 1, close - open - 1);
    }
    return FAC_CODES[fi];
}

string getValidFaculty() {
    string fac;
    while (true) {
        cout << "  Faculty (A=MKF FMHS  B=LKC FES  C=IMLD  D=IPSR  E=CCCD\n"
             << "           F=FAM      G=FCS      H=FOE   I=CFS   J=FCI  K=IIE)\n"
             << "  Enter code (A-K): ";
        getline(cin >> ws, fac);
        for (char& c : fac) c = toupper(c);
        for (int i = 0; i < FAC_COUNT; i++) {
            if (FAC_CODES[i] == fac) {
                return getAbbrevName(i); // returns e.g. "LKC FES"
            } 
        }
        cout << "  Invalid faculty. Please enter A, B, C, D, E, F, G, H, I, J or K.\n";
    }
}

// ============================================================
// FILE I/O
// ============================================================

/*
students.txt  — 7 fields:
  id | name | email | faculty | phone | password | deleted(1/0)
Backward-compatible: missing 7th field defaults to "not deleted".
*/
void loadStudentsFromFile() {
    ifstream file("students.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || studentCount >= 200) continue;
        stringstream ss(line);
        student& s = students[studentCount];
        string delStr;
        if (getline(ss, s.id,         '|') &&
            getline(ss, s.name,       '|') &&
            getline(ss, s.stud_email, '|') &&
            getline(ss, s.faculty,    '|') &&
            getline(ss, s.phone,      '|') &&
            getline(ss, s.password)) {
            s.deleted = (getline(ss, delStr) && delStr == "1");
            studentCount++;
        }
    }
}

void loadVehiclesFromFile() {
    ifstream file("vehicles.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || vehicleCount >= 600) continue;
        stringstream ss(line);
        vehicle& v = vehicles[vehicleCount];
        if (getline(ss, v.vehicleID, '|') &&
            getline(ss, v.studentID, '|') &&
            getline(ss, v.plate,     '|') &&
            getline(ss, v.type)) {
            vehicleCount++;
        }
    }
}

/*
applications.txt  — 8 fields:
  studentID | vehicleID | faculty | months | status | paymentStatus | applyDate | applyMonth | expiryDate
Backward-compatible: missing 8th field is auto-derived from applyMonth + months.
*/
void loadApplicationsFromFile() {
    ifstream file("applications.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || appsCount >= 400) continue;
        stringstream ss(line);
        application temp;
        string monthsStr;
        if (getline(ss, temp.studentID,     '|') &&
            getline(ss, temp.vehicleID,     '|') &&
            getline(ss, temp.faculty,       '|') &&
            getline(ss, monthsStr,          '|') &&
            getline(ss, temp.status,        '|') &&
            getline(ss, temp.paymentStatus, '|') &&
            getline(ss, temp.applyDate,     '|') &&
            getline(ss, temp.applyMonth)) {
            try {
                temp.months = stoi(monthsStr);
                if (temp.months < 1 || temp.months > 3) continue;

                // 8th field: expiryDate — may be absent in old files
                if (!getline(ss, temp.expiryDate)) temp.expiryDate = "";

                // Derive expiryDate for old records or unset renewal records
                if (temp.expiryDate.empty() && !temp.applyMonth.empty()) {
                    temp.expiryDate = calcExpiryNewPass(temp.applyMonth, temp.months);
                }

                if (temp.status == "paid") {
                    temp.status        = "approved";
                    temp.paymentStatus = "paid";
                } else if (temp.paymentStatus.empty()) {
                    temp.paymentStatus = "unpaid";
                }

                apps[appsCount++] = temp;
            } catch (...) {}
        }
    }
}

void loadAdminFromFile() {
    ifstream file("admin.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || adminCount >= 20) continue;
        stringstream ss(line);
        admin& a = admins[adminCount];
        if (getline(ss, a.adminID, '|') &&
            getline(ss, a.name,    '|') &&
            getline(ss, a.email,   '|') &&
            getline(ss, a.phone,   '|') &&
            getline(ss, a.password)) {
            adminCount++;
        }
    }
}

void saveStudentsToFile() {
    ofstream file("students.txt");
    for (int i = 0; i < studentCount; i++) {
        file << students[i].id         << "|"
             << students[i].name       << "|"
             << students[i].stud_email << "|"
             << students[i].faculty    << "|"
             << students[i].phone      << "|"
             << students[i].password   << "|"
             << (students[i].deleted ? "1" : "0") << "\n";
    }
}

void saveVehiclesToFile() {
    ofstream file("vehicles.txt");
    for (int i = 0; i < vehicleCount; i++) {
        file << vehicles[i].vehicleID << "|"
             << vehicles[i].studentID << "|"
             << vehicles[i].plate     << "|"
             << vehicles[i].type      << "\n";
    }
}

void saveAdminToFile() {
    ofstream file("admin.txt");
    for (int i = 0; i < adminCount; i++) {
        file << admins[i].adminID  << "|"
             << admins[i].name     << "|"
             << admins[i].email    << "|"
             << admins[i].phone    << "|"
             << admins[i].password << "\n";
    }
}

void saveApplicationsToFile() {
    ofstream file("applications.txt");
    for (int i = 0; i < appsCount; i++) {
        file << apps[i].studentID     << "|"
             << apps[i].vehicleID     << "|"
             << apps[i].faculty       << "|"
             << apps[i].months        << "|"
             << apps[i].status        << "|"
             << apps[i].paymentStatus << "|"
             << apps[i].applyDate     << "|"
             << apps[i].applyMonth    << "|"
             << apps[i].expiryDate    << "\n";
    }
}

// ============================================================
// SMALL HELPERS
// ============================================================

int facultyIndex(string fac) {
    for (int i = 0; i < FAC_COUNT; i++) {
        if (getAbbrevName(i) == fac) {
            return i;
        }
    }
    for (int i = 0; i < FAC_COUNT; i++) {  // fallback for old data files
        if (FAC_CODES[i] == fac) { 
            return i;
        }
    }
    return -1;
}

bool isRenewalApp(int appIdx) {
    string thisApplyDate = apps[appIdx].applyDate;
    for (int j = 0; j < appIdx; j++) {
        if (apps[j].vehicleID == apps[appIdx].vehicleID &&
           (apps[j].paymentStatus == "paid" || apps[j].status == "expired")) {
            // If the old pass expired more than 90 days before this application,
            // treat this as a fresh new application, not a renewal
            if (!apps[j].expiryDate.empty()) {
                int gap = daysBetween(apps[j].expiryDate, thisApplyDate);
                if (gap > 90) continue; // too long a gap — not a renewal
            }
            return true;
        }
    }
    return false;
}

int getVehiclesForStudent(string studentID, int results[], int maxCount) {
    int count = 0;
    for (int i = 0; i < vehicleCount && count < maxCount; i++) {
        if (vehicles[i].studentID == studentID) results[count++] = i;
    }
    return count;
}

int findVehicleIndexByID(string vehicleID) {
    for (int i = 0; i < vehicleCount; i++) if (vehicles[i].vehicleID == vehicleID) return i;
    return -1;
}

bool hasPendingOrApprovedForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].vehicleID == vehicleID &&
           (apps[i].status == "pending" || apps[i].status == "approved"))
            return true;
    }
    return false;
}

bool hasActivePaidPassForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].vehicleID == vehicleID &&
            apps[i].paymentStatus == "paid" &&
            apps[i].status != "expired")
            return true;
    }
    return false;
}

int findStudentIndexByID(string id) {
    for (int i = 0; i < studentCount; i++) if (students[i].id == id) return i;
    return -1;
}

int findAdminIndexByID(string id) {
    for (int i = 0; i < adminCount; i++) if (admins[i].adminID == id) return i;
    return -1;
}

// ============================================================
// CLEANUP — auto-expire passes using exact expiryDate
// ============================================================
/*
Uses the stored expiryDate field (exact "YYYY-MM-DD") for comparison.
Runs on every startup and on each student login.
*/
void cleanupExpiredPasses(string studentID) {
    string today = getCurrentDate();
    bool modified = false;
    for (int i = 0; i < appsCount; i++) {
        if (!studentID.empty() && apps[i].studentID != studentID) continue;
        if (apps[i].status == "expired" || apps[i].status == "rejected" ||
            apps[i].status == "pending") continue;
        if (apps[i].expiryDate.empty()) continue;
        if (today > apps[i].expiryDate) {
            apps[i].status = "expired";
            modified = true;
        }
    }
    if (modified) saveApplicationsToFile();
}

// ============================================================
// EXPIRY ALERT — shown on every student login
// ============================================================
/*
expiryAlert(int index_Student)
Replaces the old month-end calendar check.
Per spec: compare today with the expiryDate of the latest active paid pass.
  - <= 7 days remaining  →  warn "expires in N days"
  - 0 days               →  warn "expires TODAY"
  - > 7 days             →  no alert (avoid noise for healthy passes)
  - No active pass       →  "no active pass" notice
*/
void expiryAlert(int index_Student) {
    string id    = students[index_Student].id;
    string today = getCurrentDate();

    // Find the latest expiryDate among all currently paid passes
    string latestExpiry = "";
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID == id         &&
            apps[i].paymentStatus == "paid" &&
            apps[i].status != "expired"     &&
            !apps[i].expiryDate.empty()) {
            if (latestExpiry.empty() || apps[i].expiryDate > latestExpiry)
                latestExpiry = apps[i].expiryDate;
        }
    }

    if (latestExpiry.empty()) return;

    int daysLeft = daysBetween(today, latestExpiry);

    if (daysLeft >= 0 && daysLeft <= 7) {
        printLine('*');
        if (daysLeft == 0) {
            cout << "  *** WARNING: Your parking pass expires TODAY!             ***\n";
        } else {
            cout << "  *** WARNING: Your parking pass will expire in "
                 << daysLeft << " day(s).  ***\n";
        }
        cout << "  *** Please renew your pass to avoid interruption.         ***\n";
        cout << "  *** Current expiry : " << latestExpiry << "                       ***\n";
        printLine('*');
    }
    // > 7 days: pass is healthy, no alert shown
}

// ============================================================
// REGISTRATION
// ============================================================

void registerStudent() {
    if (studentCount >= 200) { cout << "  System full.\n"; return; }
    student stud;
    printHeader("Student Registration");
    stud.id      = "S" + to_string(studentCount + 1);
    stud.deleted = false;
    cout << "  Generated ID : " << stud.id << "\n\n";
    cout << "  Name    : "; getline(cin >> ws, stud.name);
    cout << "  Email   : "; getline(cin >> ws, stud.stud_email);
    stud.faculty  = getValidFaculty();
    cout << "  Phone   : "; getline(cin >> ws, stud.phone);
    stud.password = getValidPassword();
    students[studentCount++] = stud;
    saveStudentsToFile();
    cout << "\n  Registration successful! Your ID: " << stud.id << "\n";
    cout << "  Add your vehicle(s) via Student Menu > Manage Vehicles.\n";
}

void registerAdmin() {
    if (adminCount >= 20) { cout << "  System full.\n"; return; }
    admin ADM;
    printHeader("Admin Registration");
    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "  Generated Admin ID : " << ADM.adminID << "\n\n";
    cout << "  Name  : "; getline(cin >> ws, ADM.name);
    cout << "  Email : "; getline(cin >> ws, ADM.email);
    cout << "  Phone : "; getline(cin >> ws, ADM.phone);
    ADM.password = getValidPassword();
    admins[adminCount++] = ADM;
    saveAdminToFile();
    cout << "\n  Admin registration successful!\n";
}

// ============================================================
// VEHICLE MANAGEMENT
// ============================================================

void registerVehicle(string studentID) {
    if (vehicleCount >= 600) { cout << "  Vehicle capacity reached.\n"; return; }
    vehicle v;
    v.vehicleID = "V" + to_string(vehicleCount + 1);
    v.studentID = studentID;
    cout << "  Plate number (e.g. WXY1234) : "; getline(cin >> ws, v.plate);
    cout << "  Type (Car/Motorcycle/Van)   : "; getline(cin >> ws, v.type);
    vehicles[vehicleCount++] = v;
    saveVehiclesToFile();
    cout << "  Vehicle registered. ID: " << v.vehicleID << "\n";
}

void viewVehicles(string studentID) {
    int idx[20];
    int count = getVehiclesForStudent(studentID, idx, 20);
    if (count == 0) { cout << "  No vehicles registered.\n"; return; }

    cout << "  " << left << setw(10) << "VehicleID"
                         << setw(16) << "Plate"
                         << setw(14) << "Type"
                         << setw(10) << "Pass"
                         << "Expiry\n";
    printLine();
    for (int i = 0; i < count; i++) {
        vehicle& v  = vehicles[idx[i]];
        bool active = hasActivePaidPassForVehicle(v.vehicleID);
        string exp  = "";
        if (active) {
            for (int j = appsCount - 1; j >= 0; j--) {
                if (apps[j].vehicleID == v.vehicleID &&
                    apps[j].paymentStatus == "paid"  &&
                    apps[j].status != "expired") {
                    exp = apps[j].expiryDate; break;
                }
            }
        }
        cout << "  " << left << setw(10) << v.vehicleID
                              << setw(16) << v.plate
                              << setw(14) << v.type
                              << setw(10) << (active ? "Active" : "None")
                              << exp      << "\n";
    }
}

void manageVehicles(int index_Student) {
    string id = students[index_Student].id;
    while (true) {
        printHeader("Manage Vehicles");
        viewVehicles(id); printLine();
        cout << "  1. Add Vehicle\n  2. Back\n  Choice: ";
        if (safeInputInt(1, 2) == 1) registerVehicle(id);
        else break;
    }
}

// ============================================================
// APPLICATION — NEW PASS
// ============================================================
/*
expiryDate is set at submission time for new passes:
  last day of (applyMonth + months - 1)
  e.g. apply 2026-04 for 3 months → covers Apr, May, Jun → expires 2026-06-30
*/
void registerApplication(int index_Student) {
    printHeader("New Parking Pass Application");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles registered. Add one via Manage Vehicles first.\n"; return;
    }
    if (appsCount >= 400) { cout << "  System full.\n"; return; }

    cout << "  Select vehicle to apply for:\n"; printLine();
    for (int i = 0; i < vCount; i++) {
        vehicle& v = vehicles[idx[i]];
        string tag;
        if (hasPendingOrApprovedForVehicle(v.vehicleID))   tag = " [Pending/Approved]";
        else if (hasActivePaidPassForVehicle(v.vehicleID)) tag = " [Active — use Renew]";
        else                                                tag = " [No active pass]";
        cout << "  " << (i + 1) << ". " << v.plate << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick (1-" << vCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, vCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[idx[pick - 1]];
    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  Already has a pending/approved application. Please wait.\n"; return;
    }
    if (hasActivePaidPassForVehicle(chosen.vehicleID)) {
        cout << "  Has an active pass — use Renew Pass instead.\n"; return;
    }

    application APP;
    APP.studentID     = studentID;
    APP.vehicleID     = chosen.vehicleID;
    APP.faculty       = students[index_Student].faculty;
    APP.applyDate     = getCurrentDate();
    APP.applyMonth    = getCurrentMonth();
    APP.status        = "pending";
    APP.paymentStatus = "unpaid";

    cout << "  Months (1-3): ";
    APP.months     = safeInputInt(1, 3);
    APP.expiryDate = calcExpiryNewPass(APP.applyMonth, APP.months);

    cout << "  Estimated cost  : RM " << fixed << setprecision(2) << (APP.months * 30.0) << "\n";
    cout << "  Pass valid until: " << APP.expiryDate << " (after approval & payment)\n";

    apps[appsCount++] = APP;
    saveApplicationsToFile();
    cout << "  Application for " << chosen.plate << " submitted on "
         << APP.applyDate << ". Pending admin approval.\n";
}

// ============================================================
// APPLICATION — RENEWAL
// ============================================================
/*
expiryDate is deliberately left empty here and computed at PAYMENT time because:
  - Pass still active at payment  →  extend from current expiry
  - Pass lapsed before payment    →  start fresh from today
This matches the spec's two-case rule exactly.
*/
void renewApplication(int index_Student) {
    printHeader("Renew Parking Pass");
    string studentID = students[index_Student].id;
    string today     = getCurrentDate();

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);
    if (vCount == 0) { cout << "  No vehicles registered.\n"; return; }

    int eligIdx[20]; int eligCount = 0;
    for (int i = 0; i < vCount; i++) {
        string vid = vehicles[idx[i]].vehicleID;
        bool eligible = false;
        for (int j = appsCount - 1; j >= 0; j--) {
            if (apps[j].vehicleID == vid &&
                (apps[j].paymentStatus == "paid" || apps[j].status == "expired")) {
                 eligible = true; break;
            }
        }
        if (eligible) eligIdx[eligCount++] = idx[i];
    }
    if (eligCount == 0) {
        cout << "  No vehicle has a previous pass to renew.\n";
        cout << "  Use New Application for first-time applications.\n"; return;
    }
    cout << "  Select vehicle to renew:\n"; printLine();
    for (int i = 0; i < eligCount; i++) {
        vehicle& v = vehicles[eligIdx[i]];
        string tag = hasPendingOrApprovedForVehicle(v.vehicleID) ? " [Renewal pending]"
                   : (hasActivePaidPassForVehicle(v.vehicleID)    ? " [Active pass]"
                                                                  : " [Expired — renews from today]");
        cout << "  " << (i + 1) << ". " << v.plate << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick (1-" << eligCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, eligCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[eligIdx[pick - 1]];
    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  A renewal for this vehicle is already pending.\n"; return;
    }

    // Find active paid pass to display details
    int paidIdx = -1;
    for (int i = appsCount - 1; i >= 0; i--) {
        if (apps[i].vehicleID == chosen.vehicleID &&
           (apps[i].paymentStatus == "paid" || apps[i].status == "expired")) {
            paidIdx = i; break;
        }
    }
    bool passIsActive = (paidIdx != -1 && apps[paidIdx].status != "expired" &&
                        !apps[paidIdx].expiryDate.empty() &&
                        apps[paidIdx].expiryDate >= today);

    cout << "\n  Previous pass details:\n";
    cout << "  Vehicle       : " << chosen.plate << " (" << chosen.type << ")\n";
    cout << "  Applied on    : " << apps[paidIdx].applyDate  << "\n";
    cout << "  Duration      : " << apps[paidIdx].months     << " month(s)\n";
    cout << "  Expires on    : " << apps[paidIdx].expiryDate << "\n";
    cout << "  Status        : " << (passIsActive ? "Active" : "Expired") << "\n";
    if (passIsActive)
        cout << "  Days remaining: " << daysBetween(today, apps[paidIdx].expiryDate) << "\n";
    else
        cout << "  NOTE: Pass expired — renewal will start from today.\n";
    printLine();

    char confirm;
    cout << "  Confirm renewal? (y/n): ";
    cin >> confirm; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') { cout << "  Renewal cancelled.\n"; return; }
    if (appsCount >= 400) { cout << "  System full.\n"; return; }

    application renewal;
    renewal.studentID     = studentID;
    renewal.vehicleID     = chosen.vehicleID;
    renewal.faculty       = students[index_Student].faculty;
    renewal.applyDate     = today;
    renewal.applyMonth    = getCurrentMonth();
    renewal.status        = "pending";
    renewal.paymentStatus = "unpaid";
    renewal.expiryDate    = "";   // computed at payment time

    cout << "  Months to renew (1-3): ";
    renewal.months = safeInputInt(1, 3);
    cout << "  Estimated cost: RM " << fixed << setprecision(2) << (renewal.months * 30.0) << "\n";

    // Preview projected expiry (if paid before current pass expires)
    if (passIsActive) {
        string preview = addMonthsToExpiry(apps[paidIdx].expiryDate, renewal.months);
        cout << "  Projected new expiry: " << preview << " (extending from current expiry)\n";
    } else {
        string preview = calcExpiryNewPass(getCurrentMonth(), renewal.months);
        cout << "  Projected new expiry: " << preview << " (starting from today)\n";
    }

    apps[appsCount++] = renewal;
    saveApplicationsToFile();
    cout << "\n  Renewal submitted on " << renewal.applyDate << ".\n";
    cout << "  Your current pass stays active until you pay for this renewal.\n";
}

// ============================================================
// STUDENT PROFILE
// ============================================================

void viewStudentProfile(int index_Student) {
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

    cout << "  REGISTERED VEHICLES\n"; printLine();
    int idx[20];
    int vCount = getVehiclesForStudent(s.id, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles registered.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID"
                              << setw(14) << "Plate"
                              << setw(14) << "Type"
                              << setw(12) << "Status"
                              << "Expiry\n";
        printLine();
        for (int i = 0; i < vCount; i++) {
            vehicle& v    = vehicles[idx[i]];
            string status = "No pass", expiry = "";
            for (int j = appsCount - 1; j >= 0; j--) {
                if (apps[j].vehicleID == v.vehicleID &&
                    apps[j].status != "rejected" && apps[j].status != "expired") {
                    status = (apps[j].paymentStatus == "paid") ? "paid" : apps[j].status;
                    expiry = apps[j].expiryDate;
                    break;
                }
            }
            cout << "  " << left << setw(10) << v.vehicleID
                                  << setw(14) << v.plate
                                  << setw(14) << v.type
                                  << setw(12) << status
                                  << expiry   << "\n";
        }
    }
    printLine();
}

void updateStudentProfile(int index_Student) {
    printHeader("Update Profile");
    cout << "  Name    : "; getline(cin >> ws, students[index_Student].name);
    cout << "  Phone   : "; getline(cin >> ws, students[index_Student].phone);

    char chFac;
    cout << "  Change faculty? (y/n): ";
    cin >> chFac; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chFac == 'y' || chFac == 'Y') students[index_Student].faculty = getValidFaculty();

    char chPass;
    cout << "  Change password? (y/n): ";
    cin >> chPass; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chPass == 'y' || chPass == 'Y')
        students[index_Student].password = getValidPassword();
    else
        cout << "  Password unchanged.\n";

    saveStudentsToFile();
    cout << "  Profile updated successfully!\n";
}

// ============================================================
// STUDENT — TRANSACTIONS & ANALYTICS
// ============================================================
/*
Key behaviours:
- ALL historical records shown (paid, expired, rejected, pending, approved).
- Analytics include paid AND expired (both were purchased).
- Approved records prompt payment.
- Renewal payment:
    Still active  →  addMonthsToExpiry(oldExpiry, months)  [spec rule 1]
    Already lapsed→  calcExpiryNewPass(today, months)      [spec rule 2]
    Old pass is then marked "expired" (superseded by renewal).
*/
void viewApplicationHistory(int index_Student) {
    printHeader("My Parking Pass Transactions");
    string id    = students[index_Student].id;
    string today = getCurrentDate();
    bool   found = false;
    int    num   = 1;

    int myNewApps = 0, myRenewals = 0, myPaidMonths = 0;
    double myTotalSpent = 0;
    string mLabels[12]; buildMonthLabels(mLabels, 12);
    double monthSpend[12] = {};

    cout << "  " << left << setw(5) << "No." << setw(10) << "Vehicle"
         << setw(12) << "Applied" << setw(7) << "Mths"
         << setw(12) << "Status" << setw(12) << "Expiry" << "Type\n";
    printLine('-', 70);

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != id) continue;
        found = true;

        int vi = findVehicleIndexByID(apps[i].vehicleID);
        string plate   = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        string appType = isRenewalApp(i) ? "Renewal" : "New";
        string expDisp = apps[i].expiryDate.empty() ? "TBD" : apps[i].expiryDate;

        cout << "  " << left << setw(5) << num++ << setw(10) << plate
             << setw(12) << apps[i].applyDate << setw(7) << apps[i].months
             << setw(12) << apps[i].status    << setw(12) << expDisp
             << appType  << "\n";

        // ── Payment prompt ───────────────────────────────────────
        if (apps[i].status == "approved") {
            printLine('-', 70);
            cout << "  APPROVED — " << plate << " | "
                 << apps[i].months << " month(s) | RM "
                 << fixed << setprecision(2) << (apps[i].months * 30.0) << "\n";

            // Preview the expiry before asking for payment
            if (isRenewalApp(i)) {
                string baseExpiry = "";
                for (int k = i - 1; k >= 0; k--) {
                    if (apps[k].vehicleID == apps[i].vehicleID &&
                       (apps[k].paymentStatus == "paid" || apps[k].status == "expired") &&
                       !apps[k].expiryDate.empty()) {
                        baseExpiry = apps[k].expiryDate; break;
                    }
                }
                if (!baseExpiry.empty() && baseExpiry >= today)
                    cout << "  Extends pass (" << baseExpiry << ")  ->  new expiry: "
                         << addMonthsToExpiry(baseExpiry, apps[i].months) << "\n";
                else
                    cout << "  Previous pass expired — new pass from today  ->  expiry: "
                         << calcExpiryNewPass(getCurrentMonth(), apps[i].months) << "\n";
            } else {
                cout << "  Pass valid until: " << apps[i].expiryDate << "\n";
            }

            char pay;
            cout << "  Pay now? (y/n): ";
            cin >> pay; cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (pay == 'y' || pay == 'Y') {
                apps[i].paymentStatus = "paid";

                if (isRenewalApp(i)) {
                    // Compute definitive expiryDate at payment moment
                    string baseExpiry = "";
                    for (int k = i - 1; k >= 0; k--) {
                        if (apps[k].vehicleID == apps[i].vehicleID &&
                           (apps[k].paymentStatus == "paid" || apps[k].status == "expired") &&
                           !apps[k].expiryDate.empty()) {
                            baseExpiry = apps[k].expiryDate; break;
                        }
                    }
                    if (!baseExpiry.empty() && baseExpiry >= today)
                        apps[i].expiryDate = addMonthsToExpiry(baseExpiry, apps[i].months);
                    else
                        apps[i].expiryDate = calcExpiryNewPass(getCurrentMonth(), apps[i].months);

                    // Supersede the old paid pass (happens at payment, not at approval)
                    for (int k = 0; k < appsCount; k++) {
                        if (k == i) continue;
                        if (apps[k].vehicleID == apps[i].vehicleID &&
                            apps[k].paymentStatus == "paid" && apps[k].status != "expired") {
                            apps[k].status = "expired";
                        }
                    }
                }
                // New pass: expiryDate was set at submission — no change needed.

                saveApplicationsToFile();
                cout << "  Payment successful! Pass is now active.\n";
                cout << "  Pass expires on : " << apps[i].expiryDate << "\n";
            } else {
                cout << "  Payment skipped. You can pay later from this menu.\n";
            }
        }

        // Analytics — include paid AND expired (both were purchased)
        if (apps[i].paymentStatus == "paid" || apps[i].status == "expired") {
            myPaidMonths += apps[i].months;
            myTotalSpent += apps[i].months * 30.0;
            for (int m = 0; m < 12; m++) {
                if (apps[i].applyMonth == mLabels[m]) {
                    monthSpend[m] += apps[i].months * 30.0; break;
                }
            }
        }
        if (appType == "New") myNewApps++;
        else                  myRenewals++;
    }

    if (!found) { cout << "  No applications found.\n"; return; }

    // ── Summary ──────────────────────────────────────────────────
    printLine('-', 70);
    cout << "  MY TRANSACTION SUMMARY\n"; printLine('-', 70);
    cout << "  New applications  : " << myNewApps    << "\n";
    cout << "  Renewals          : " << myRenewals   << "\n";
    cout << "  Total months paid : " << myPaidMonths << "\n";
    cout << "  Total spent       : RM " << fixed << setprecision(2) << myTotalSpent << "\n";
    if (myPaidMonths > 0)
        cout << "  Avg cost/month    : RM "
             << fixed << setprecision(2) << (myTotalSpent / myPaidMonths) << "\n";

    // ── Monthly breakdown ────────────────────────────────────────
    cout << "\n  MONTHLY PAYMENT HISTORY (last 12 months)\n"; printLine('-', 70);
    bool anyPayment = false; int maxSpend = 1;
    for (int m = 0; m < 12; m++) if (monthSpend[m] > maxSpend) maxSpend = (int)monthSpend[m];
    for (int m = 0; m < 12; m++) {
        if (monthSpend[m] == 0) continue;
        anyPayment = true;
        int bar = (int)(monthSpend[m] * 20 / maxSpend);
        cout << "  " << mLabels[m] << " RM"
             << setw(7) << fixed << setprecision(2) << monthSpend[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "\n";
    }
    if (!anyPayment) cout << "  No payments in the last 12 months.\n";

    // ── Renewal rate ─────────────────────────────────────────────
    if (myRenewals > 0 && myNewApps > 0) {
        double rr = (double)myRenewals / (myNewApps + myRenewals) * 100.0;
        cout << "\n  Your renewal rate: " << fixed << setprecision(0) << rr << "%";
        if      (rr >= 80) cout << "  (Very consistent user!)";
        else if (rr >= 50) cout << "  (Average User!)";
        else               cout << "  (Mostly new applications.)";
        cout << "\n";
    }

    // ── Inline expiry reminder ───────────────────────────────────
    string latestExpiry = "";
    for (int k = 0; k < appsCount; k++) {
        if (apps[k].studentID == id && apps[k].paymentStatus == "paid" &&
            !apps[k].expiryDate.empty()) {
            if (latestExpiry.empty() || apps[k].expiryDate > latestExpiry)
                latestExpiry = apps[k].expiryDate;
        }
    }
    if (!latestExpiry.empty()) {
        int dLeft = daysBetween(today, latestExpiry);
        if (dLeft >= 0 && dLeft <= 7) {
            printLine('*');
            if (dLeft == 0)
                cout << "  *** WARNING: Your parking pass expires TODAY!       ***\n";
            else
                cout << "  *** WARNING: Pass expires in " << dLeft << " day(s)!           ***\n";
            cout << "  *** Please renew to avoid interruption.             ***\n";
            printLine('*');
        }
    }
    printLine('-', 70);
}

// ============================================================
// ADMIN — PROCESS APPLICATIONS
// ============================================================

void viewProfileAdmin(int index_Admin) {
    printHeader("Admin: Process Applications");
    cout << "  Admin ID : " << admins[index_Admin].adminID << "\n";
    cout << "  Name     : " << admins[index_Admin].name    << "\n";
    cout << "  Email    : " << admins[index_Admin].email   << "\n";
    cout << "  Phone    : " << admins[index_Admin].phone   << "\n\n";

    bool found = false;
    cout << "  " << left << setw(7) << "Index" << setw(9) << "StudID"
         << setw(12) << "Vehicle" << setw(8) << "Fac"
         << setw(12) << "Date"   << setw(7) << "Mths"
         << setw(12) << "CurrExpiry" << "Type\n";
    printLine('-', 78);

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].status != "pending") continue;
        found = true;
        int vi = findVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        string currExpiry = "-";
        if (isRenewalApp(i)) {
            for (int k = i - 1; k >= 0; k--) {
                if (apps[k].vehicleID == apps[i].vehicleID &&
                    apps[k].paymentStatus == "paid" &&
                    apps[k].status != "expired"     &&
                    !apps[k].expiryDate.empty()) {
                    currExpiry = apps[k].expiryDate;
                    break;
                }
            }
        }
        cout << "  " << left << setw(7) << i
             << setw(9)  << apps[i].studentID
             << setw(12) << plate
             << setw(8)  << ("[" + apps[i].faculty + "]")
             << setw(12) << apps[i].applyDate
             << setw(7)  << apps[i].months
             << setw(12) << currExpiry
             << (isRenewalApp(i) ? "Renewal" : "New") << "\n";
    }

    if (!found) { cout << "  No pending applications.\n"; return; }

    cout << "\n  Enter index to process (-1 to cancel): ";
    int choice = safeInputInt(-1, appsCount - 1);
    if (choice == -1) return;

    if (choice >= 0 && choice < appsCount) {
        if (apps[choice].status == "pending") approveRejectApplication(choice);
        else cout << "  Already processed.\n";
    } else {
        cout << "  Invalid index.\n";
    }
}

void approveRejectApplication(int app_index) {
    if (app_index < 0 || app_index >= appsCount) return;

    int vi = findVehicleIndexByID(apps[app_index].vehicleID);
    string plate = (vi != -1) ? vehicles[vi].plate : apps[app_index].vehicleID;
    bool renewal = isRenewalApp(app_index);

    cout << "\n  Student  : " << apps[app_index].studentID << "\n";
    cout << "  Vehicle  : " << plate << "\n";
    cout << "  Faculty  : " << apps[app_index].faculty << "\n";
    cout << "  Duration : " << apps[app_index].months << " month(s)\n";
    cout << "  Type     : " << (renewal ? "Renewal" : "New") << "\n";

    if (renewal) {
        for (int k = app_index - 1; k >= 0; k--) {
            if (apps[k].vehicleID == apps[app_index].vehicleID &&
                apps[k].paymentStatus == "paid" &&
                apps[k].status != "expired"     &&
                !apps[k].expiryDate.empty()) {
                cout << "  Current pass expires : " << apps[k].expiryDate << "\n";
                break;
            }
        }
        cout << "  NOTE: Current pass remains active until student pays for this renewal.\n";
    }

    cout << "\n  1. Approve\n  2. Reject\n  Decision: ";
    int decision = safeInputInt(1, 2);

    if (decision == 1) {
        apps[app_index].status = "approved";
        cout << "  Application approved. Student must pay to activate.\n";
    } else {
        apps[app_index].status = "rejected";
        cout << "  Application rejected.\n";
    }
    saveApplicationsToFile();
}

// ============================================================
// ADMIN — STATISTICS & ANALYTICS
// ============================================================
/*
Faculty breakdown display:
The full faculty name is printed on its own line, then the numbers on the next
line indented below it. This guarantees all numeric columns stay perfectly
aligned regardless of how long the faculty name is.

Layout per faculty:
  -------...------
  [X] Full Faculty Name (full label from FAC_LABELS)
      Apps: N   (N%)   Paid: N   Rejected: N   New: N   Renewals: N  [<-- Most]
*/
void statisticsUsage(int index_Admin) {
    printHeader("Admin: Statistics & Analytics");
    cout << "  Admin: " << admins[index_Admin].name << "\n";
    cout << "  Date : " << getCurrentDate() << "\n\n";

    int total    = appsCount;
    int approved = 0, rejected = 0, pending = 0, paid = 0, expired = 0;
    int durationCount[4] = {0};

    int facApps[FAC_COUNT]     = {0};
    int facApproved[FAC_COUNT] = {0};
    int facPaid[FAC_COUNT]     = {0};
    int facRejected[FAC_COUNT] = {0};
    int facNew[FAC_COUNT]      = {0};
    int facRenew[FAC_COUNT]    = {0};

    string mLabels[12]; buildMonthLabels(mLabels, 12);
    int monthApps[12]    = {0};
    int monthRevenue[12] = {0};

    for (int i = 0; i < appsCount; i++) {
        if      (apps[i].status        == "rejected") rejected++;
        else if (apps[i].status        == "pending")  pending++;
        else if (apps[i].status        == "expired")  expired++;
        else if (apps[i].paymentStatus == "paid")     paid++;
        else if (apps[i].status        == "approved") approved++;

        if (apps[i].months >= 1 && apps[i].months <= 3) durationCount[apps[i].months]++;

        int fi = facultyIndex(apps[i].faculty);
        if (fi != -1) {
            facApps[fi]++;
            if (apps[i].status        == "approved") facApproved[fi]++;
            if (apps[i].paymentStatus == "paid")     facPaid[fi]++;
            if (apps[i].status        == "rejected") facRejected[fi]++;
            if (isRenewalApp(i)) facRenew[fi]++;
            else                 facNew[fi]++;
        }

        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                monthApps[m]++;
                if (apps[i].paymentStatus == "paid") monthRevenue[m] += apps[i].months * 30;
                break;
            }
        }
    }

    int activeNow = paid + approved;
    double utilRate = studentCount > 0 ? (double)activeNow / studentCount * 100.0 : 0.0;

    // ── Overall summary ──────────────────────────────────────────
    printLine();
    cout << "  OVERALL SUMMARY\n"; printLine();
    cout << "  Total students       : " << studentCount << "\n";
    cout << "  Total vehicles       : " << vehicleCount << "\n";
    cout << "  Total applications   : " << total        << "\n";
    cout << "  Approved (unpaid)    : " << approved     << "\n";
    cout << "  Paid / Active        : " << paid         << "\n";
    cout << "  Rejected             : " << rejected     << "\n";
    cout << "  Pending              : " << pending      << "\n";
    cout << "  Expired              : " << expired      << "\n";
    cout << "  Utilisation rate     : " << fixed << setprecision(1) << utilRate << "%\n";

    // ── Faculty breakdown — full name, two-line layout ────────────
    int peakFac = 0;
    for (int f = 1; f < FAC_COUNT; f++) {
        if (facApps[f] > facApps[peakFac]) peakFac = f;
    }

    printLine();
    cout << "  FACULTY BREAKDOWN\n";
    cout << "  (Name line)  [Code] Full Faculty Name\n";
    cout << "  (Stats line)   Apps | % | Paid | Rejected | New | Renewals\n";
    printLine();

    for (int f = 0; f < FAC_COUNT; f++) {
        double pct  = (total > 0) ? (double)facApps[f] / total * 100.0 : 0.0;
        string star = (f == peakFac && facApps[peakFac] > 0) ? "  <-- Most" : "";

        // Line 1: code + full faculty name
        cout << "  [" << FAC_CODES[f] << "] " << FAC_LABELS[f] << "\n";

        // Line 2: numbers, all left-aligned under fixed-width labels
        cout << "       "
             << "Apps:"     << left << setw(5) << facApps[f]
             << "("         << setw(4) << (to_string((int)round(pct)) + "%") << ")  "
             << "Paid:"     << setw(5) << facPaid[f]
             << "Rejected:" << setw(5) << facRejected[f]
             << "New:"      << setw(5) << facNew[f]
             << "Renewals:" << facRenew[f]
             << star        << "\n";

        printLine('-', 60);
    }

    // ── Faculty bar chart — code + bar + count ───────────────────
    cout << "\n  FACULTY APPLICATION CHART\n"; printLine();
    for (int f = 0; f < FAC_COUNT; f++) {
        // Truncate label to 35 chars so bar stays on one line
        string lbl = FAC_LABELS[f];
        if ((int)lbl.size() > 35) lbl = lbl.substr(0, 32) + "...";
        cout << "  " << left << setw(36) << lbl << " : ";
        for (int j = 0; j < facApps[f] && j < 30; j++) cout << "#";
        if (facApps[f] > 30) cout << "+";
        cout << "  (" << facApps[f] << ")\n";
    }

    // ── Duration breakdown ───────────────────────────────────────
    printLine();
    cout << "  BY PASS DURATION\n"; printLine();
    for (int m = 1; m <= 3; m++) {
        cout << "  " << m << " month(s) : ";
        for (int j = 0; j < durationCount[m] && j < 40; j++) cout << "#";
        if (durationCount[m] > 40) cout << "+";
        cout << "  (" << durationCount[m] << ")\n";
    }

    // ── Monthly trend ────────────────────────────────────────────
    printLine();
    cout << "  MONTHLY APPLICATION TREND (last 12 months)\n"; printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) if (monthApps[m] > maxBar) maxBar = monthApps[m];
    for (int m = 0; m < 12; m++) {
        int bar = (monthApps[m] * 30) / maxBar;
        cout << "  " << mLabels[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << " apps, RM "
             << fixed << setprecision(0) << (double)monthRevenue[m] << ")\n";
    }
    printLine();
    cout << "  Run 'Generate Summary Report' for the full printable report.\n";
}

// BE AWARE: DO NOT UNCOMMENT — PROTOTYPE ONLY
// ============================================================
// ADMIN — STATISTICS BY YEAR (COMMENTED OUT — NOT IN MAIN MENU)
// To activate: uncomment this function and add option 8 to adminMenu().
// ============================================================
/*
void statisticsUsageByYear(int index_Admin) {
    printHeader("Admin: Statistics by Year");
    cout << "  Admin: " << admins[index_Admin].name << "\n\n";
    int minYear = 9999, maxYear = 0;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].applyMonth.size() < 4) continue;
        try { int yr = stoi(apps[i].applyMonth.substr(0, 4));
              if (yr < minYear) minYear = yr;
              if (yr > maxYear) maxYear = yr; } catch (...) {}
    }
    if (minYear > maxYear) { cout << "  No application data available.\n"; return; }
    int yearList[50]; int yearCount = 0;
    for (int y = minYear; y <= maxYear && yearCount < 50; y++) yearList[yearCount++] = y;
    cout << "  Available years:\n";
    for (int i = 0; i < yearCount; i++) cout << "  " << (i + 1) << ". " << yearList[i] << "\n";
    cout << "  0. Cancel\n  Choice: ";
    int pick = safeInputInt(0, yearCount);
    if (pick == 0) return;
    int selectedYear = yearList[pick - 1];
    string mLabels[12];
    for (int m = 0; m < 12; m++) {
        char buf[8]; sprintf(buf, "%04d-%02d", selectedYear, m + 1); mLabels[m] = string(buf);
    }
    cout << "\n  Year: " << selectedYear << "\n"; printLine();
    int monthApps[12] = {0}, monthRevenue[12] = {0};
    int facApps[FAC_COUNT] = {0}, facPaid[FAC_COUNT] = {0};
    int totalApps = 0, totalRevenue = 0;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].applyMonth.size() < 4) continue;
        if (apps[i].applyMonth.substr(0, 4) != to_string(selectedYear)) continue;
        totalApps++;
        int fi = facultyIndex(apps[i].faculty);
        if (fi != -1) { facApps[fi]++; if (apps[i].status == "paid") facPaid[fi]++; }
        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                monthApps[m]++;
                if (apps[i].status == "paid") {
                    int rev = apps[i].months * 30; monthRevenue[m] += rev; totalRevenue += rev;
                }
                break;
            }
        }
    }
    cout << "  Total applications in " << selectedYear << " : " << totalApps    << "\n";
    cout << "  Total revenue collected               : RM "
         << fixed << setprecision(2) << (double)totalRevenue << "\n";
    cout << "\n  MONTHLY TREND — " << selectedYear << "\n"; printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) if (monthApps[m] > maxBar) maxBar = monthApps[m];
    for (int m = 0; m < 12; m++) {
        int bar = (monthApps[m] * 30) / maxBar;
        cout << "  " << mLabels[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << " apps, RM "
             << fixed << setprecision(0) << (double)monthRevenue[m] << ")\n";
    }
    printLine(); cout << "  FACULTY BREAKDOWN — " << selectedYear << "\n"; printLine();
    for (int f = 0; f < FAC_COUNT; f++) {
        cout << "  [" << FAC_CODES[f] << "] " << FAC_LABELS[f] << "\n";
        cout << "       Apps:" << left << setw(5) << facApps[f]
             << "Paid:" << facPaid[f] << "\n";
        printLine('-', 60);
    }
    printLine();
}
*/
// To add to adminMenu: cout << "  8. Statistics by Year\n";
//                       safeInputInt(1, 8)
//                       else if (choice == 8) statisticsUsageByYear(index_Admin);

// ============================================================
// ADMIN — SUMMARY REPORT
// ============================================================

void generateSummaryReport(int index_Admin) {
    printHeader("Admin: Summary Report");
    cout << "  Generated by : " << admins[index_Admin].name << "\n";
    cout << "  Date         : " << getCurrentDate() << "\n\n";

    string mLabels[12]; buildMonthLabels(mLabels, 12);

    int facMonthApps[FAC_COUNT][12] = {};
    int facTotal[FAC_COUNT]         = {0};
    int facNew[FAC_COUNT]           = {0};
    int facRenew[FAC_COUNT]         = {0};
    int facPaidMonths[FAC_COUNT]    = {0};
    int facRevenue[FAC_COUNT]       = {0};
    int monthTotal[12]              = {0};
    int monthRevenue[12]            = {0};
    int totalRevenue = 0, paidCount = 0, totalPaidMonths = 0;

    for (int i = 0; i < appsCount; i++) {
        int fi = facultyIndex(apps[i].faculty);
        if (fi == -1) continue;
        facTotal[fi]++;
        if (isRenewalApp(i)) facRenew[fi]++; else facNew[fi]++;
        if (apps[i].paymentStatus == "paid") {
            facPaidMonths[fi] += apps[i].months;
            facRevenue[fi]    += apps[i].months * 30;
            totalRevenue      += apps[i].months * 30;
            totalPaidMonths   += apps[i].months;
            paidCount++;
        }
        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                facMonthApps[fi][m]++; monthTotal[m]++;
                if (apps[i].paymentStatus == "paid") monthRevenue[m] += apps[i].months * 30;
                break;
            }
        }
    }

    if (appsCount == 0) {
        cout << "  No applications data available yet.\n";
        printLine('=', 60); cout << "  [End of Report]\n"; return;
    }

    int peakFac = 0;
    for (int f = 1; f < FAC_COUNT; f++) if (facTotal[f] > facTotal[peakFac]) peakFac = f;
    int peakMonth = 0;
    for (int m = 1; m < 12; m++) if (monthTotal[m] > monthTotal[peakMonth]) peakMonth = m;

    // ── SECTION 1 ────────────────────────────────────────────────
    cout << "  ================================================\n";
    cout << "  SECTION 1: Application Averages by Faculty\n";
    cout << "  ================================================\n";

    for (int f = 0; f < FAC_COUNT; f++) {
        double avg = facTotal[f] / 12.0;
        cout << "\n  [" << FAC_CODES[f] << "] " << FAC_LABELS[f] << "\n";
        printLine('-', 60);
        cout << "  " << left << setw(10) << "Total" << setw(10) << "New"
             << setw(12) << "Renewals" << setw(12) << "Avg/Month"
             << setw(13) << "PaidMonths" << "Revenue\n";
        cout << "  " << left << setw(10) << facTotal[f]
             << setw(10) << facNew[f]
             << setw(12) << facRenew[f]
             << setw(12) << fixed << setprecision(1) << avg
             << setw(13) << facPaidMonths[f]
             << "RM " << fixed << setprecision(2) << (double)facRevenue[f] << "\n";
    }

    cout << "\n"; printLine('=', 60);
    if (facTotal[peakFac] > 0)
        cout << "  Highest demand: [" << FAC_CODES[peakFac]
             << "] " << FAC_LABELS[peakFac] << "\n";

    // ── SECTION 2 ────────────────────────────────────────────────
    // Sub-tables use [Code] columns (always same width) for alignment.
    // Full legend printed separately.
    cout << "\n  ================================================\n";
    cout << "  SECTION 2: Monthly Volume by Faculty (last 12 months)\n";
    cout << "  ================================================\n";

    int split = 6;   // faculties A-F in table 1, G-K in table 2

    cout << "\n  [ Faculties A to " << FAC_CODES[split - 1] << " ]\n";
    cout << "  " << left << setw(10) << "Month";
    for (int f = 0; f < split && f < FAC_COUNT; f++) cout << setw(6) << ("[" + FAC_CODES[f] + "]");
    cout << "  Total\n"; printLine('-', 52);
    for (int m = 0; m < 12; m++) {
        cout << "  " << left << setw(10) << mLabels[m];
        for (int f = 0; f < split && f < FAC_COUNT; f++) cout << setw(6) << facMonthApps[f][m];
        cout << "  " << monthTotal[m] << "\n";
    }
    printLine('-', 52);

    if (FAC_COUNT > split) {
        cout << "\n  [ Faculties " << FAC_CODES[split]
             << " to " << FAC_CODES[FAC_COUNT - 1] << " ]\n";
        cout << "  " << left << setw(10) << "Month";
        for (int f = split; f < FAC_COUNT; f++) cout << setw(6) << ("[" + FAC_CODES[f] + "]");
        cout << "  Revenue\n"; printLine('-', 48);
        for (int m = 0; m < 12; m++) {
            cout << "  " << left << setw(10) << mLabels[m];
            for (int f = split; f < FAC_COUNT; f++) cout << setw(6) << facMonthApps[f][m];
            cout << "  RM " << right << fixed << setprecision(0)
                 << (double)monthRevenue[m] << "\n";
        }
        printLine('-', 48);
    }

    cout << "\n  Faculty Code Reference:\n";
    for (int f = 0; f < FAC_COUNT; f++)
        cout << "    [" << FAC_CODES[f] << "] " << FAC_LABELS[f] << "\n";

    if (monthTotal[peakMonth] > 0)
        cout << "\n  Peak month: " << mLabels[peakMonth]
             << " (" << monthTotal[peakMonth] << " apps)\n";
    else
        cout << "\n  Peak month: No data yet.\n";

    // ── SECTION 3 ────────────────────────────────────────────────
    int activeNow = 0;
    for (int i = 0; i < appsCount; i++)
        if (apps[i].paymentStatus == "paid" || apps[i].status == "approved") activeNow++;
    double utilRate  = studentCount > 0 ? (double)activeNow / studentCount * 100.0 : 0.0;
    double avgMonths = paidCount > 0 ? (double)totalPaidMonths / paidCount : 0.0;

    cout << "\n  ================================================\n";
    cout << "  SECTION 3: Car Park Utilisation - Sungai Long\n";
    cout << "  ================================================\n"; printLine('-', 60);
    cout << "  Registered students       : " << studentCount  << "\n";
    cout << "  Registered vehicles       : " << vehicleCount  << "\n";
    cout << "  Currently active passes   : " << activeNow     << "\n";
    cout << "  Utilisation rate          : " << fixed << setprecision(1) << utilRate << "%\n";
    cout << "  Total revenue collected   : RM "
         << fixed << setprecision(2) << (double)totalRevenue << "\n";
    cout << "  Avg months per paid pass  : ";
    if (paidCount > 0) cout << fixed << setprecision(1) << avgMonths << "\n";
    else               cout << "N/A (no paid passes yet)\n";
    cout << "  Avg monthly revenue       : RM ";
    if (totalRevenue > 0) cout << fixed << setprecision(2) << (totalRevenue / 12.0) << "\n";
    else                  cout << "0.00\n";
    printLine('-', 60);

    // ── SECTION 4 ────────────────────────────────────────────────
    cout << "\n  ================================================\n";
    cout << "  SECTION 4: Negotiation Insights (UTAR vs MPKJ)\n";
    cout << "  ================================================\n"; printLine('-', 60);

    if (activeNow == 0) {
        cout << "  No active passes yet — insufficient data for insights.\n";
        printLine('=', 60); cout << "  [End of Report]\n"; return;
    }

    cout << "  1. " << activeNow
         << " students actively using MPKJ passes = steady revenue.\n";
    cout << "  2. Utilisation rate of " << fixed << setprecision(1) << utilRate
         << "% shows consistent demand.\n";
    if (facTotal[peakFac] > 0)
        cout << "  3. Highest demand from [" << FAC_CODES[peakFac] << "] "
             << FAC_LABELS[peakFac]
             << "\n     -- consider faculty-bundle pricing.\n";
    if (monthTotal[peakMonth] > 0)
        cout << "  4. Peak month: " << mLabels[peakMonth]
             << " -- useful for MPKJ capacity planning.\n";
    cout << "  5. Avg " << fixed << setprecision(1) << avgMonths
         << " months/pass -- bulk discount for 3-month passes\n"
         << "     could increase revenue and student loyalty.\n";
    cout << "  6. Total RM " << fixed << setprecision(2) << (double)totalRevenue
         << " paid -- demonstrates UTAR's bargaining power.\n";
    printLine('=', 60); cout << "  [End of Report]\n";
}

// ============================================================
// ADMIN — VIEW STUDENT PROFILE
// ============================================================

void adminViewStudentProfile(int index_Admin) {
    (void)index_Admin;
    printHeader("Admin: View Student Profile");
    if (studentCount == 0) { cout << "  No students registered.\n"; return; }

    cout << "  " << left << setw(6) << "No." << setw(8) << "ID"
                         << setw(22) << "Name" << setw(8) << "Faculty" << "Status\n";
    printLine();
    for (int i = 0; i < studentCount; i++) {
        string status = students[i].deleted ? "(DELETED)" : "Active";
        cout << "  " << left << setw(6)  << (i + 1)
                              << setw(8)  << students[i].id
                              << setw(22) << students[i].name
                              << setw(8)  << ("[" + students[i].faculty + "]")
                              << status   << "\n";
    }
    printLine();

    cout << "  Select student (0 to cancel): ";
    int pick = safeInputInt(0, studentCount);
    if (pick == 0) return;

    student& s = students[pick - 1];
    printLine('=');
    cout << "  " << s.name << " (" << s.id << ")";
    if (s.deleted) cout << "  [DELETED ACCOUNT]";
    cout << "\n"; printLine('=');
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";
    cout << "  Phone   : " << s.phone      << "\n";
    cout << "  Status  : " << (s.deleted ? "Deleted (records retained)" : "Active") << "\n";

    printLine(); cout << "  VEHICLES\n"; printLine();
    int idx[20]; int vCount = getVehiclesForStudent(s.id, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID" << setw(16) << "Plate"
                              << setw(14) << "Type" << setw(10) << "Pass" << "Expiry\n";
        printLine();
        for (int i = 0; i < vCount; i++) {
            vehicle& v  = vehicles[idx[i]];
            bool active = hasActivePaidPassForVehicle(v.vehicleID);
            string exp  = "";
            if (active) {
                for (int j = appsCount - 1; j >= 0; j--) {
                    if (apps[j].vehicleID == v.vehicleID &&
                        apps[j].paymentStatus == "paid"  &&
                        apps[j].status != "expired") {
                        exp = apps[j].expiryDate;
                        break;
                    }
                }
            }
            cout << "  " << left << setw(10) << v.vehicleID << setw(16) << v.plate
                                  << setw(14) << v.type << setw(10) << (active?"Active":"None")
                                  << exp << "\n";
        }
    }

    printLine(); cout << "  APPLICATION HISTORY\n"; printLine();
    cout << "  " << left << setw(6) << "Idx" << setw(12) << "Vehicle"
                         << setw(12) << "Date" << setw(7) << "Mths"
                         << setw(12) << "Status" << setw(12) << "Expiry" << "Type\n";
    printLine('-', 75);
    bool any = false;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != s.id) continue;
        any = true;
        int vi = findVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        string displayStatus = (apps[i].paymentStatus == "paid") ? "paid" : apps[i].status;
        cout << "  " << left << setw(6) << i << setw(12) << plate
                     << setw(12) << apps[i].applyDate << setw(7) << apps[i].months
                     << setw(12) << displayStatus
                     << setw(12) << (apps[i].expiryDate.empty() ? "TBD" : apps[i].expiryDate)
                     << (isRenewalApp(i) ? "Renewal" : "New") << "\n";
    }
    if (!any) cout << "  No applications found.\n";
    printLine();
}

// ============================================================
// ADMIN — DELETE STUDENT (soft-delete)
// ============================================================
/*
Marks the student as deleted (deleted = true). Login is blocked.
ALL records — vehicles, applications, payments — are fully preserved in
the data files and appear unchanged in all stats and year-end reports.
*/
void deleteStudent(int index_Admin) {
    (void)index_Admin;
    printHeader("Admin: Delete Student Account");
    if (studentCount == 0) { cout << "  No students registered.\n"; return; }

    cout << "  " << left << setw(6) << "No." << setw(8) << "ID"
                         << setw(22) << "Name" << setw(8) << "Faculty" << "Status\n";
    printLine();
    for (int i = 0; i < studentCount; i++) {
        string status = students[i].deleted ? "(DELETED)" : "Active";
        cout << "  " << left << setw(6)  << (i + 1)
                              << setw(8)  << students[i].id
                              << setw(22) << students[i].name
                              << setw(8)  << ("[" + students[i].faculty + "]")
                              << status   << "\n";
    }
    printLine();
    cout << "  Select student to delete (0 to cancel): ";
    int pick = safeInputInt(0, studentCount);
    if (pick == 0) { cout << "  Cancelled.\n"; return; }

    student& s = students[pick - 1];
    if (s.deleted) { cout << "  This account is already deleted.\n"; return; }

    cout << "\n  Student to delete:\n";
    cout << "  ID      : " << s.id         << "\n";
    cout << "  Name    : " << s.name       << "\n";
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";

    int activePasses = 0, pendingApps = 0;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != s.id) continue;
        if ((apps[i].paymentStatus == "paid" && apps[i].status != "expired")
        || apps[i].status == "approved") activePasses++;
        if (apps[i].status == "pending") pendingApps++;
    }
    if (activePasses > 0)
        cout << "  WARNING : " << activePasses << " active/approved pass(es) will remain in records.\n";
    if (pendingApps > 0)
        cout << "  WARNING : " << pendingApps  << " pending application(s) will remain in records.\n";

    cout << "\n  NOTE: All vehicle, application and payment records are PRESERVED\n";
    cout << "        for reporting purposes. Only login access is removed.\n";
    printLine();

    char confirm;
    cout << "  Confirm deletion? (y/n): ";
    cin >> confirm; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') { cout << "  Deletion cancelled.\n"; return; }

    s.deleted = true;
    saveStudentsToFile();
    cout << "\n  Account '" << s.name << "' (" << s.id << ") has been deactivated.\n";
    cout << "  All records are retained in the system for reporting.\n";
}

// ============================================================
// ADMIN — UPDATE PROFILE
// ============================================================

void updateAdminProfile(int index_Admin) {
    printHeader("Update Admin Profile");
    cout << "  Name  : "; getline(cin >> ws, admins[index_Admin].name);
    cout << "  Email : "; getline(cin >> ws, admins[index_Admin].email);
    cout << "  Phone : "; getline(cin >> ws, admins[index_Admin].phone);

    char chPass;
    cout << "  Change password? (y/n): ";
    cin >> chPass; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chPass == 'y' || chPass == 'Y')
        admins[index_Admin].password = getValidPassword();
    else
        cout << "  Password unchanged.\n";

    saveAdminToFile();
    cout << "  Admin profile updated!\n";
}

// ============================================================
// MENUS
// ============================================================

void studentMenu(int index_Student) {
    expiryAlert(index_Student);    // 7-day check on every login
    while (true) {
        printHeader("Student Menu");
        cout << "  1. View Profile\n";
        cout << "  2. Update Profile\n";
        cout << "  3. Manage Vehicles\n";
        cout << "  4. New Application\n";
        cout << "  5. Renew Pass\n";
        cout << "  6. My Transactions & Analytics\n";
        cout << "  7. Logout\n";
        printLine();
        cout << "  Choice: ";
        int choice = safeInputInt(1, 7);
        if      (choice == 1) viewStudentProfile(index_Student);
        else if (choice == 2) updateStudentProfile(index_Student);
        else if (choice == 3) manageVehicles(index_Student);
        else if (choice == 4) registerApplication(index_Student);
        else if (choice == 5) renewApplication(index_Student);
        else if (choice == 6) viewApplicationHistory(index_Student);
        else if (choice == 7) { cout << "  Logging out...\n"; break; }
    }
}

void adminMenu(int index_Admin) {
    while (true) {
        printHeader("Admin Menu");
        cout << "  1. Process Pending Applications\n";
        cout << "  2. View Student Profile\n";
        cout << "  3. Statistics & Analytics\n";
        cout << "  4. Generate Summary Report\n";
        cout << "  5. Update Profile\n";
        cout << "  6. Delete Student Account\n";
        cout << "  7. Logout\n";
        // NOTE: Option 8 (Statistics by Year) is commented out above.
        // To enable: uncomment statisticsUsageByYear(), change safeInputInt to (1,8),
        // and add:  cout << "  8. Statistics by Year\n";
        //           else if (choice == 8) statisticsUsageByYear(index_Admin);
        printLine();
        cout << "  Choice: ";
        int choice = safeInputInt(1, 7);
        if      (choice == 1) viewProfileAdmin(index_Admin);
        else if (choice == 2) adminViewStudentProfile(index_Admin);
        else if (choice == 3) statisticsUsage(index_Admin);
        else if (choice == 4) generateSummaryReport(index_Admin);
        else if (choice == 5) updateAdminProfile(index_Admin);
        else if (choice == 6) deleteStudent(index_Admin);
        else if (choice == 7) { cout << "  Logging out...\n"; break; }
    }
}

void mainMenu() {
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
            registerStudent();

        } else if (choice == 2) {
            registerAdmin();

        } else if (choice == 3) {
            bool loginDone = false;
            for (int idAtt = 0; idAtt < 3 && !loginDone; idAtt++) {
                cout << "  Student ID: ";
                cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
                int si = findStudentIndexByID(id);
                if (si == -1) {
                    if (idAtt < 2)
                        cout << "  Invalid ID. (" << (2 - idAtt) << " attempt(s) left).\n";
                    else
                        cout << "  Invalid ID. Too many failed attempts. Returning to main menu.\n";
                    if (idAtt == 2) loginDone = true;
                    continue;
                }
                if (students[si].deleted) {
                    cout << "  This account has been deactivated. Please contact the admin.\n";
                    loginDone = true; continue;
                }
                bool ok = false;
                for (int att = 0; att < 3; att++) {
                    cout << "  Password: "; getline(cin >> ws, password);
                    if (students[si].password == password) {
                        cout << "  Login Successful!\n";
                        cleanupExpiredPasses(students[si].id);
                        studentMenu(si);
                        ok = true; break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
                if (!ok) cout << "  Too many failed password attempts. Returning to main menu.\n";
                loginDone = true;
            }

        } else if (choice == 4) {
            bool loginDone = false;
            for (int idAtt = 0; idAtt < 3 && !loginDone; idAtt++) {
                cout << "  Admin ID: ";
                cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
                int ai = findAdminIndexByID(id);
                if (ai == -1) {
                    if (idAtt < 2)
                        cout << "  Invalid ID. (" << (2 - idAtt) << " attempt(s) left).\n";
                    else
                        cout << "  Invalid ID. Too many failed attempts. Returning to main menu.\n";
                    if (idAtt == 2) loginDone = true;
                    continue;
                }
                bool ok = false;
                for (int att = 0; att < 3; att++) {
                    cout << "  Password: "; getline(cin >> ws, password);
                    if (admins[ai].password == password) {
                        cout << "  Login Successful!\n";
                        adminMenu(ai);
                        ok = true; break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
                if (!ok) cout << "  Too many failed password attempts. Returning to main menu.\n";
                loginDone = true;
            }

        } else if (choice == 5) {
            cout << "  Exiting...\n"; break;
        }
    }
}