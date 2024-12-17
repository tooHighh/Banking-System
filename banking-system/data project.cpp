#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

struct transaction
{
    string date;
    double amount;
    transaction *next;
};

struct account
{
    string IBAN;
    string accountName;
    double balance;
    string currency;
    double limitDepositPerDay;
    double limitWithdrawPerMonth;
    transaction *txn;
    account *next;
};

struct user
{
    int userID;
    string fname;
    string lname;
    account *acct;
    user *next, *previous;
};

struct userList
{
    user *head = NULL, *tail = NULL;
};

void addUserToList(userList *list, user *usr)
{
    usr->next = NULL;
    usr->previous = NULL;

    if (list->head == NULL)
    {
        list->head = usr;
        return;
    }

    user *u = list->head;
    while (u->next != NULL)
        u = u->next;

    u->next = usr;
    usr->previous = u;
    list->tail = usr;
}

// https://stackoverflow.com/questions/57882748/how-do-i-remove-trailing-zeros-when-printing-a-floating-point-number

string convertScientificToReadable(double num)
{
    stringstream ss;
    ss << fixed << setprecision(2) << num;
    string str = ss.str();

    str = str.substr(0, str.find_last_not_of('0') + 1);

    if (str.find('.') == str.size() - 1)
    {
        str = str.substr(0, str.size() - 1);
    }
    return str;
}

bool addAccToUser(user *usr, account *acct)
{
    acct->next = NULL;
    account *cur = usr->acct;

    if (usr->acct == NULL)
    {
        usr->acct = acct;
        return 1;
    }

    while (cur->next != NULL)
    {
        if (cur->accountName == acct->accountName && acct->IBAN == cur->IBAN)
            return 0;

        cur = cur->next;
    }

    if (cur->accountName == acct->accountName && acct->IBAN == cur->IBAN)
        return 0;

    cur->next = acct;
    return 1;
}

void addTxnToAcc(account *acc, transaction *txn)
{
    txn->next = NULL;
    transaction *cur = acc->txn;

    if (acc->txn == NULL)
    {
        acc->txn = txn;
        return;
    }

    while (cur->next != NULL)
        cur = cur->next;

    cur->next = txn;
}

userList *readFile()
{
    userList *list = new userList;
    ifstream is("users.txt");
    if (!is.is_open())
    {
        return list;
    }

    account *acct;
    user *usr;
    transaction *txn;

    while (!is.eof())
    {
        char ltype;
        is >> ltype;

        if (ltype == '-')
        {
            usr = new user;
            is >> usr->userID;
            is.ignore(1);
            getline(is, usr->fname, ',');
            getline(is, usr->lname);
            usr->acct = NULL;
            addUserToList(list, usr);
        }
        if (ltype == '#')
        {
            acct = new account;
            getline(is, acct->IBAN, ',');
            getline(is, acct->accountName, ',');
            is >> acct->balance;
            getline(is, acct->currency, ',');
            is >> acct->limitDepositPerDay;
            is.ignore(acct->currency.length() + 1);
            is >> acct->limitWithdrawPerMonth;
            is.ignore(acct->currency.length() + 1);
            acct->txn = NULL;
            addAccToUser(usr, acct);
        }

        if (ltype == '*')
        {
            txn = new transaction;
            getline(is, txn->date, ',');
            is >> txn->amount;
            is.ignore(acct->currency.length() + 1);
            addTxnToAcc(acct, txn);
        }
    }
    is.close();
    return list;
}

void print(userList *list)
{
    if (list == NULL || list->head == NULL)
    {
        cout << "The user list is empty or not initialized." << endl;
        return;
    }

    user *cur = list->head;
    while (cur != NULL)
    {
        cout << endl;
        cout << "========================" << endl;
        cout << "User ID: " << cur->userID << endl;
        cout << "First Name: " << cur->fname << endl;
        cout << "Last Name: " << cur->lname << endl;
        cout << "========================" << endl;

        account *curAct = cur->acct;

        while (curAct != NULL)
        {
            cout << "  - IBAN: " << curAct->IBAN << endl;
            cout << "  - Account Name: " << curAct->accountName << endl;
            cout << "  - Balance: " << convertScientificToReadable(curAct->balance) << endl;
            cout << "  - Currency: " << curAct->currency << endl;
            cout << "  - Daily Deposit Limit: " << convertScientificToReadable(curAct->limitDepositPerDay) << endl;
            cout << "  - Monthly Withdrawal Limit: " << convertScientificToReadable(curAct->limitWithdrawPerMonth) << endl;

            transaction *curTxn = curAct->txn;

            while (curTxn != NULL)
            {
                cout << "    * Transaction Date: " << curTxn->date << endl;
                cout << "    * Transaction Amount: " << convertScientificToReadable(curTxn->amount) << endl;
                curTxn = curTxn->next;
            }
            curAct = curAct->next;
        }
        cur = cur->next;
    }
}

double convert(double balance, string currency1, string currency2)
{
    if (currency2 == "$")
    {
        if (currency1 == "L.L")
            return balance / 1500; // L.L to USD
        else if (currency1 == "€")
            return balance / 0.92; // Euro to USD
        else
            return balance;
    }
    else if (currency2 == "L.L")
    {
        if (currency1 == "$")
            return balance * 1500; // USD to L.L
        else if (currency1 == "€")
            return balance * 1500 / 0.92; // Euro to L.L
        else
            return balance;
    }
    else if (currency2 == "€")
    {
        if (currency1 == "L.L")
            return balance / 1500 * 0.92; // L.L to Euro
        else if (currency1 == "$")
            return balance * 0.92; // USD to Euro
        else
            return balance;
    }
    return 0;
}

tm returnRealTime()
{
    time_t tt = time(NULL);
    struct tm ti = *localtime(&tt);
    ti.tm_mon += 1;
    ti.tm_year += 1900;

    return ti;
}

tm returnStringAsTime(const char *time)
{
    struct tm ti;

    sscanf_s(time, "%d/%d/%d", &ti.tm_mday, &ti.tm_mon, &ti.tm_year);

    return ti;
}

string returnTimeAsString(tm time)
{
    string day = (time.tm_mday < 10 ? "0" : "") + to_string(time.tm_mday);
    string month = (time.tm_mon < 10 ? "0" : "") + to_string(time.tm_mon);

    string tt = day + "/" + month + "/" + to_string(time.tm_year);
    return tt;
}

account *returnAcc(userList *list, int ID, string IBAN)
{
    user *usr = list->head;
    while (usr != NULL)
    {
        if (usr->userID == ID)
        {
            account *acc = usr->acct;
            while (acc != NULL)
            {
                if (acc->IBAN == IBAN)
                    return acc;
                acc = acc->next;
            }
        }
        usr = usr->next;
    }
    return NULL;
}

void transfer(userList *list, double amount, account *acc1, account *acc2)
{
    if (!acc1 || !acc2)
    {
        cout << "\nInvalid input!\n";
        return;
    }

    if (amount <= 0 || amount > acc1->balance)
    {
        cout << "\nNo enough money!\n";
        return;
    }

    double convertedAmount = convert(amount, acc1->currency, acc2->currency);

    tm currentTime = returnRealTime();
    string cts = returnTimeAsString(currentTime);

    double countAmountDeposit = 0;
    double countAmountWithdraw = 0;

    transaction *tn1 = acc1->txn;
    transaction *tn2 = acc2->txn;

    while (tn2 != NULL)
    {
        if (tn2->date == cts && tn2->amount > 0)
            countAmountDeposit += tn2->amount;

        tn2 = tn2->next;
    }

    int mon = -1;
    int year = -1;

    int cMon = currentTime.tm_mon;
    int cYear = currentTime.tm_year;

    while (tn1 != NULL)
    {
        mon = returnStringAsTime(tn1->date.c_str()).tm_mon;
        year = returnStringAsTime(tn1->date.c_str()).tm_year;
        if (mon == cMon && year == cYear && tn1->amount < 0)
            countAmountWithdraw += tn1->amount;

        tn1 = tn1->next;
    }

    if (abs(countAmountWithdraw + amount) > abs(acc1->limitWithdrawPerMonth) ||
        countAmountDeposit + convertedAmount > acc2->limitDepositPerDay)
    {
        cout << "\nYou've reached your limits.\n";
        return;
    }

    transaction *t1 = new transaction, *t2 = new transaction;
    t1->date = cts;
    t2->date = cts;

    t1->amount = -amount;
    t2->amount = convertedAmount;

    t1->next = t2->next = NULL;

    if (acc1->txn == NULL)
        acc1->txn = t1;

    else
        addTxnToAcc(acc1, t1);

    if (acc2->txn == NULL)
        acc2->txn = t2;
    else
        addTxnToAcc(acc2, t2);

    acc2->balance += convertedAmount;
    acc1->balance -= amount;

    cout << "\nThe transfer was successful!\n";
}

// https://www.geeksforgeeks.org/merge-sort-for-linked-list/
// -- split -- merge -- MergeSort --

transaction *split(transaction *head)
{
    transaction *fast = head;
    transaction *slow = head;

    while (fast != NULL && fast->next != NULL)
    {
        fast = fast->next->next;
        if (fast != NULL)
        {
            slow = slow->next;
        }
    }

    transaction *temp = slow->next;
    slow->next = NULL;
    return temp;
}

transaction *merge(transaction *first, transaction *second)
{

    if (first == nullptr)
        return second;
    if (second == nullptr)
        return first;

    tm firstTm = returnStringAsTime(first->date.c_str());
    tm secondTm = returnStringAsTime(second->date.c_str());

    if (firstTm.tm_year < secondTm.tm_year || firstTm.tm_mday < secondTm.tm_mday || firstTm.tm_mon < secondTm.tm_mon)
    {
        first->next = merge(first->next, second);
        return first;
    }
    else
    {
        second->next = merge(first, second->next);
        return second;
    }
}

transaction *MergeSort(transaction *head)
{

    if (head == NULL || head->next == NULL)
        return head;

    transaction *second = split(head);

    head = MergeSort(head);
    second = MergeSort(second);

    return merge(head, second);
}

void sort(userList *list)
{
    if (!list || !list->head)
        return;

    user *cur = list->head;
    while (cur != NULL)
    {
        account *acc = cur->acct;
        while (acc != NULL)
        {
            acc->txn = MergeSort(acc->txn);

            acc = acc->next;
        }

        cur = cur->next;
    }
}

void writeFile(userList *list)
{
    ofstream os("users.txt");
    user *usr = list->head;
    bool firstUsr = false;

    while (usr != NULL)
    {
        account *acct = usr->acct;

        os << (!firstUsr ? "" : "\n")
           << "-" << usr->userID << "," << usr->fname << "," << usr->lname;
        firstUsr = true;

        while (acct != NULL)
        {
            os << endl
               << "#" << acct->IBAN << "," << acct->accountName << "," << convertScientificToReadable(acct->balance) << acct->currency << "," << convertScientificToReadable(acct->limitDepositPerDay) << acct->currency << "," << convertScientificToReadable(acct->limitWithdrawPerMonth) << acct->currency;

            transaction *txn = acct->txn;
            while (txn != NULL)
            {
                os << endl
                   << "*" << txn->date << "," << convertScientificToReadable(txn->amount) << acct->currency;

                txn = txn->next;
            }

            acct = acct->next;
        }

        usr = usr->next;
    }
}

void deleteTxn(userList *list, string date)
{
    user *usr = list->head;
    tm todayDate = returnRealTime();
    tm dateToDeletePrev = returnStringAsTime(date.c_str());

    if (todayDate.tm_year < dateToDeletePrev.tm_year)
    {
        cout << "\nThe date is in the future!\n";
        return;
    }
    else if (todayDate.tm_year == dateToDeletePrev.tm_year)
    {
        if (todayDate.tm_mon < dateToDeletePrev.tm_mon)
        {
            cout << "\nThe date is in the future!\n";
            return;
        }
        else if (todayDate.tm_mon == dateToDeletePrev.tm_mon)
        {
            if (todayDate.tm_mday < dateToDeletePrev.tm_mday)
            {
                cout << "\nThe date is in the future!\n";
                return;
            }
        }
    }

    while (usr != NULL)
    {
        account *acc = usr->acct;
        while (acc != NULL)
        {
            transaction *txn = acc->txn, *prev = NULL;
            while (txn != NULL)
            {
                tm txnDate = returnStringAsTime(txn->date.c_str());

                if (txnDate.tm_year < dateToDeletePrev.tm_year)
                {
                    cout << "\nThe date is in the future!\n";
                    return;
                }
                else if (txnDate.tm_year == dateToDeletePrev.tm_year)
                {
                    if (txnDate.tm_mon < dateToDeletePrev.tm_mon)
                    {
                        cout << "\nThe date is in the future!\n";
                        return;
                    }
                    else if (txnDate.tm_mon == dateToDeletePrev.tm_mon)
                    {
                        if (txnDate.tm_mday < dateToDeletePrev.tm_mday)
                        {
                            cout << "\nThe date is in the future!\n";
                            return;
                        }
                    }
                    if (prev == NULL)
                    {
                        acc->txn = txn->next;
                        delete txn;
                        txn = acc->txn;
                    }
                    else
                    {
                        prev->next = txn->next;
                        delete txn;
                        txn = prev->next;
                    }
                    continue;
                }
                prev = txn;
                txn = txn->next;
            }
            acc = acc->next;
        }
        usr = usr->next;
    }
    cout << "\nThe delete was successfull!\n";
}

void createAccount(userList *list)
{
    user *usr = list->head;
    int ID;
    cout << "Enter the ID of the user: ";
    cin >> ID;
    account *acc = NULL;
    while (usr != NULL)
    {
        if (usr->userID == ID)
        {
            acc = new account;
            cin.ignore();
            cout << "Enter the IBAN of the account: ";
            getline(cin, acc->IBAN);
            cout << "Enter the account name: ";
            getline(cin, acc->accountName);
            cout << "Enter the currency of the account: ";
            getline(cin, acc->currency);
            cout << "Enter the balance of the account: ";
            cin >> acc->balance;
            cout << "Enter the limit deposit per day: ";
            cin >> acc->limitDepositPerDay;
            cout << "Enter the limit withdraw per month: ";
            cin >> acc->limitWithdrawPerMonth;
            acc->txn = NULL;
            break;
        }
        usr = usr->next;
    }

    if (acc == NULL)
    {
        cout << "\nThe user doesn't exist!\n";
        return;
    }

    if (!list || !list->head)
        return;

    user *cur = list->head;
    while (cur != NULL)
    {
        if (cur->userID == usr->userID)
        {
            int res = addAccToUser(usr, acc);
            if (res == 0)
            {
                cout << "\nAccount already present!\n";
                delete acc;
            }
            else
                cout << "\nThe account was created succefully!\n";

            return;
        }

        cur = cur->next;
    }
}

void freeMemory(userList *list)
{
    user *cur = list->head;
    while (cur != NULL)
    {
        account *acc = cur->acct;
        while (acc != NULL)
        {
            transaction *txn = acc->txn;
            while (txn != NULL)
            {
                transaction *nextTxn = txn->next;
                delete txn;
                txn = nextTxn;
            }
            account *nextAcc = acc->next;
            delete acc;
            acc = nextAcc;
        }
        user *nextUser = cur->next;
        delete cur;
        cur = nextUser;
    }
    delete list;
}

enum
{
    CREATE_ACC = 1,
    TRANSFER,
    DELETE_DATES_BEFORE,
    SORT,
    SAVE_TO_FILE,
    DISPLAY,
    EXIT
};

void consoleOutput(userList *list)
{
    int res;

    while (true)
    {

        do
        {
            cout << endl;
            cout << CREATE_ACC << ". Create an account" << endl;
            cout << TRANSFER << ". Transfer money" << endl;
            cout << DELETE_DATES_BEFORE << ". Delete transactions before a specific date" << endl;
            cout << SORT << ". Sort the transactions" << endl;
            cout << SAVE_TO_FILE << ". Save the list to the file" << endl;
            cout << DISPLAY << ". Display the list content" << endl;
            cout << EXIT << ". Exit" << endl;

            cout << "Enter your choice: ";
            cin >> res;
        } while (res < 1 || res > 7);

        switch (res)
        {
        case 1:
            createAccount(list);
            break;

        case 2:
        {
            double amount;
            string IBAN1, IBAN2;
            int id1, id2;

            cout << "Enter the amount to transfer: ";
            cin >> amount;
            cout << "Enter the ID of the first user: ";
            cin >> id1;
            cout << "Enter the ID of the second user: ";
            cin >> id2;
            cin.ignore();
            cout << "Enter the IBAN of the account to transfer from: ";
            getline(cin, IBAN1);
            cout << "Enter the IBAN of the account to transfer to: ";
            getline(cin, IBAN2);

            transfer(list, amount, returnAcc(list, id1, IBAN1), returnAcc(list, id2, IBAN2));
            break;
        }

        case 3:
        {
            string date;
            cin.ignore();
            cout << "Enter a date to delete transactions before (dd/mm/YYYY): ";
            getline(cin, date);
            deleteTxn(list, date);
            break;
        }

        case 4:
            sort(list);
            break;

        case 5:
            writeFile(list);
            break;

        case 6:
            print(list);
            break;

        case 7:
            return;
        }
    }
}

int main()
{
    userList *list = readFile();

    consoleOutput(list);

    return 0;
}