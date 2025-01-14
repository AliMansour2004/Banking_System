#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> 
#include <ctime>

using namespace std;

struct transaction {
	string date;
	double amount;
	transaction* next;
};
struct account {
	string IBAN;
	string accountName;
	double balance;
	string currency;
	double limitDepositPerDay;
	double limitWithdrawPerMonth;
	transaction* txn;
	account* next;
};
struct user {
	int userID;
	string fname;
	string lname;
	account* acc;
	user* next;
	user* prev;
};
struct userList {
	user* head;
	user* tail;
};

string getCurrentDate() {
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);

	int year = 1900 + ltm.tm_year;
	int month = 1 + ltm.tm_mon;
	int day = ltm.tm_mday;

	return (day < 10 ? "0" : "") + to_string(day) + "/" + (month < 10 ? "0" : "") + to_string(month) + "/" + to_string(year);
}

void displayData(userList* list) {
	user* currentUser = list->head;
	while (currentUser != NULL) {
		cout << "User ID: " << currentUser->userID << endl;
		cout << "Name: " << currentUser->fname << " " << currentUser->lname << endl;

		account* currentAccount = currentUser->acc;
		while (currentAccount != NULL) {
			cout << "  Account IBAN: " << currentAccount->IBAN << endl;
			cout << "  Account Name: " << currentAccount->accountName << endl;


			cout << "  Balance: " << fixed << setprecision(0) << currentAccount->balance << " " << currentAccount->currency << endl;
			cout << "  Limit Deposit Per Day: " << currentAccount->limitDepositPerDay << " " << currentAccount->currency << endl;
			cout << "  Limit Withdraw Per Month: " << currentAccount->limitWithdrawPerMonth << " " << currentAccount->currency << endl;

			transaction* currentTxn = currentAccount->txn;
			while (currentTxn != NULL) {
				cout << "    Transaction Date: " << currentTxn->date << endl;
				cout << "    Amount: " << currentTxn->amount << " " << currentAccount->currency << endl;
				currentTxn = currentTxn->next;
			}

			currentAccount = currentAccount->next;
		}

		currentUser = currentUser->next;
	}
}

user* createUser(int id, string& fname, string& lname) {
	user* newUser = new user;
	newUser->userID = id;
	newUser->fname = fname;
	newUser->lname = lname;
	newUser->acc = NULL;
	newUser->next = NULL;
	newUser->prev = NULL;
	return newUser;
}
account* createAccount(string& IBAN, string& accountName, double balance, string& currency, double limitDepositPerDay, double limitWithdrawPerMonth) {

	account* newAcc = new account;
	newAcc->IBAN = IBAN;
	newAcc->accountName = accountName;
	newAcc->balance = balance;
	newAcc->currency = currency;
	newAcc->limitDepositPerDay = limitDepositPerDay;
	newAcc->limitWithdrawPerMonth = limitWithdrawPerMonth;
	newAcc->txn = NULL;
	newAcc->next = NULL;
	return newAcc;
}
transaction* createTransaction(string& date, double amount) {
	transaction* newTxn = new transaction;
	newTxn->date = date;
	newTxn->amount = amount;
	newTxn->next = NULL;
	return newTxn;
}
void parse(userList* list) {
	ifstream file("txn_user.txt");


	string line;
	user* currentUser = NULL;
	account* currentAccount = NULL;

	while (getline(file, line)) {
		if (line.empty())
			continue;

		if (line[0] == '-') {

			int id = stoi(line.substr(1, line.find(',')));
			size_t firstComma = line.find(',', 1);
			size_t secondComma = line.find(',', firstComma + 1);

			string fname = line.substr(firstComma + 1, secondComma - firstComma - 1);
			string lname = line.substr(secondComma + 1);

			currentUser = createUser(id, fname, lname);


			if (list->head == NULL) {
				list->head = list->tail = currentUser;
			}
			else {
				list->tail->next = currentUser;
				currentUser->prev = list->tail;
				list->tail = currentUser;
			}
		}
		else if (line[0] == '#') {
			size_t firstComma = line.find(',', 1);
			size_t secondComma = line.find(',', firstComma + 1);
			size_t thirdComma = line.find(',', secondComma + 1);
			size_t fourthComma = line.find(',', thirdComma + 1);
			size_t fifthComma = line.find(',', fourthComma + 1);

			string IBAN = line.substr(1, firstComma - 1);
			string accountName = currentUser->fname + " " + currentUser->lname;

			string balanceStr = line.substr(secondComma + 1, thirdComma - secondComma - 1);
			string currency;
			double balance = 0.0;

			if (balanceStr.find('$') != string::npos) {
				currency = "$";
				balanceStr.erase(remove(balanceStr.begin(), balanceStr.end(), '$'), balanceStr.end());
			}
			else if (balanceStr.find("€") != string::npos || balanceStr.find("\xE2\x82\xAC") != string::npos || balanceStr.find("\x80") != string::npos) {
				//bta3mel check (awwal case)
				//check bl UTF-8 byte (tene case)
				//check bl Extended ASCII representation of the euro symbol under Windows-1252 encoding (telet case)
				currency = "€";
				balanceStr.erase(remove(balanceStr.begin(), balanceStr.end(), '€'), balanceStr.end());
			}
			else if (balanceStr.find("L.L") != string::npos) {
				currency = "L.L";
				balanceStr.erase(balanceStr.find("L.L"), 3);
			}
			
			balance = stod(balanceStr);

			string limitDepositStr = line.substr(thirdComma + 1, fourthComma - thirdComma - 1);

			limitDepositStr.erase(remove(limitDepositStr.begin(), limitDepositStr.end(), currency[0]), limitDepositStr.end());
			if (currency == "L.L") {
				size_t pos = limitDepositStr.find(currency);
				if (pos != string::npos) {
					limitDepositStr.erase(pos, currency.size());
				}
			}

			double limitDeposit = stod(limitDepositStr);

			string limitWithdrawStr = line.substr(fourthComma + 1, fifthComma - fourthComma - 1);

			limitWithdrawStr.erase(remove(limitWithdrawStr.begin(), limitWithdrawStr.end(), currency[0]), limitWithdrawStr.end());
			if (currency == "L.L") {
				size_t pos = limitWithdrawStr.find(currency);
				if (pos != string::npos) {
					limitWithdrawStr.erase(pos, currency.size());
				}
			}

			double limitWithdraw = stod(limitWithdrawStr);

			currentAccount = createAccount(IBAN, accountName, balance, currency, limitDeposit, limitWithdraw);

			if (currentUser->acc == NULL) {
				currentUser->acc = currentAccount;
			}
			else {
				account* tmp = currentUser->acc;
				while (tmp->next != NULL) {
					tmp = tmp->next;
				}
				tmp->next = currentAccount;
			}
		}
		else if (line[0] == '*') {

			size_t commaPos = line.find(',', 1);
			string date = line.substr(1, commaPos - 1);
			double amount = stod(line.substr(commaPos + 1));

			transaction* newTxn = createTransaction(date, amount);

			if (currentAccount->txn == NULL) {
				currentAccount->txn = newTxn;
			}
			else {
				transaction* tempTxn = currentAccount->txn;
				while (tempTxn->next != NULL) {
					tempTxn = tempTxn->next;
				}
				tempTxn->next = newTxn;
			}
		}
	}
	file.close();
}



void swap(transaction* a, transaction* b) {
	string tempDate = a->date;
	double tempAmount = a->amount;

	a->date = b->date;
	a->amount = b->amount;

	b->date = tempDate;
	b->amount = tempAmount;
}
transaction* partition(transaction* low, transaction* high) {
	string pivot = high->date;
	transaction* i = low;

	for (transaction* j = low; j != high; j = j->next) {
		if (j->date < pivot) {
			swap(i, j);
			i = i->next;
		}
	}
	swap(i, high);
	return i;
}
void quickSort(transaction* low, transaction* high) {
	if (low != NULL && high != NULL && low != high) {
		transaction* pi = partition(low, high);

		transaction* temp = low;
		while (temp != NULL && temp->next != pi) {
			temp = temp->next;
		}

		quickSort(low, temp);
		quickSort(pi->next, high);
	}
}
transaction* getTail(transaction* head) {
	if (head == NULL)
		return NULL;

	while (head->next != NULL) {
		head = head->next;
	}
	return head;
}
transaction* sortTransactions(transaction* head) {
	if (head == NULL || head->next == NULL) //ya3ne iza b2alba 0 aw 1 nodes 
		return head;

	transaction* tail = getTail(head);
	quickSort(head, tail);
	return head;
}
void displayTransactions(transaction* head) {
	transaction* current = head;
	while (current != NULL) {
		cout << "    Transaction Date: " << current->date << "\n";
		cout << "    Amount: " << current->amount << "\n";
		current = current->next;
	}
}



user* findUser(userList* list, string& fname, string& lname) {
	user* current = list->head;
	while (current != NULL) {
		if (current->fname == fname && current->lname == lname) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}
user* createNewUser(userList* list, string& fname, string& lname) {
	int newID = (list->tail == NULL) ? 1 : list->tail->userID + 1;

	user* newUser = createUser(newID, fname, lname);

	if (list->head == NULL) {
		list->head = list->tail = newUser;
	}
	else {
		list->tail->next = newUser;
		newUser->prev = list->tail;
		list->tail = newUser;
	}
	cout << "New user created with ID: " << newID << "\n";
	return newUser;
}
account* createNewAccount(string& fname, string& lname) {
	string IBAN, currency;
	double balance, limitDeposit, limitWithdraw;


	cout << "Enter IBAN: ";
	getline(cin, IBAN);

	cout << "Enter balance: ";
	cin >> balance;

	int currencyChoice;
	do {
		cout << "Select currency:\n";
		cout << "1. $\n";
		cout << "2. €\n";
		cout << "3. L.L\n";
		cout << "Enter your choice (1-3): ";
		cin >> currencyChoice;

		switch (currencyChoice) {
		case 1:
			currency = "$";
			break;
		case 2:
			currency = "€";
			break;
		case 3:
			currency = "L.L";
			break;
		default:
			cout << "Invalid, choose correctly.\n";
			break;
		}
	} while (currencyChoice < 1 || currencyChoice > 3);

	cout << "Enter daily deposit limit: ";
	cin >> limitDeposit;

	cout << "Enter monthly withdrawal limit: ";
	cin >> limitWithdraw;

	string accountName = fname + " " + lname;

	return createAccount(IBAN, accountName, balance, currency, limitDeposit, limitWithdraw);
}
void addAccountToUser(userList* list, user* targetUser, account* newAccount) {

	account* tempAcc = targetUser->acc;
	while (tempAcc != NULL) {
		if (tempAcc->IBAN == newAccount->IBAN) {
			cout << "An account with IBAN " << newAccount->IBAN << " already exists for this user.\n";
			return;
		}
		tempAcc = tempAcc->next;
	}

	if (targetUser->acc == NULL) {
		targetUser->acc = newAccount;
	}
	else {
		account* lastAcc = targetUser->acc;
		while (lastAcc->next != NULL) {
			lastAcc = lastAcc->next;
		}
		lastAcc->next = newAccount;
	}

	cout << "Account with IBAN " << newAccount->IBAN << " successfully added to user "<< targetUser->fname << " " << targetUser->lname << ".\n";
}
void manageAccountForUser(userList* list) {
	string fname, lname;

	cout << "Enter user's first name: ";
	cin.ignore();
	getline(cin, fname);
	cout << "Enter user's last name: ";
	getline(cin, lname);

	user* targetUser = findUser(list, fname, lname);
	if (targetUser == NULL) {
		cout << "User not found. Creating a new user...\n";
		targetUser = createNewUser(list, fname, lname);
	}
	else {
		cout << "User found: " << targetUser->fname << " " << targetUser->lname << "\n";
	}
	account* newAccount = createNewAccount(targetUser->fname, targetUser->lname);

	addAccountToUser(list, targetUser, newAccount); 
	cout << "Account successfully added!\n";
}



double convertCurrency(double amount, string& fromCurrency, string& toCurrency) {
	if (fromCurrency == toCurrency) {
		return amount;
	}

	if (fromCurrency == "$") {
		if (toCurrency == "L.L")
			return amount * 89000;

		if (toCurrency == "€")
			return amount * 0.9;
	}
	else if (fromCurrency == "L.L") {
		if (toCurrency == "$")
			return amount / 89000;

		if (toCurrency == "€")
			return amount / 95000;
	}
	else if (fromCurrency == "€") {
		if (toCurrency == "$")
			return amount / 0.9;

		if (toCurrency == "L.L")
			return amount * 95000;
	}

	return amount;
}
transaction* addTransaction(transaction* txnList, string& date, double amount) {
	transaction* newTxn = new transaction;
	newTxn->date = date;
	newTxn->amount = amount;
	newTxn->next = NULL;

	if (txnList == NULL) {
		return newTxn;
	}
	else {
		transaction* temp = txnList;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newTxn;
	}

	return txnList;
}
void transferAmount(userList* list, double amount, account* fromAccount, account* toAccount) {
	do {
		if (fromAccount->balance < amount) {
			cout << "Insufficient balance in the source account.\n";
			cout << "Your current balance is: " << fromAccount->balance << " " << fromAccount->currency << endl;
			cout << "Enter a new amount to transfer: ";
			cin >> amount;
		}
	} while (fromAccount->balance < amount);

	do {
		if (amount > fromAccount->limitDepositPerDay) {
			cout << "Transfer amount exceeds the daily withdrawal limit ("<< fromAccount->limitDepositPerDay << " " << fromAccount->currency << ").\n";
			cout << "Enter a new amount to transfer: ";
			cin >> amount;
		}
	} while (amount > fromAccount->limitDepositPerDay);

	double convertedAmount = convertCurrency(amount, fromAccount->currency, toAccount->currency);

	do {
		if (convertedAmount > toAccount->limitWithdrawPerMonth) {
			cout << "Transfer amount exceeds the monthly deposit limit ("<< toAccount->limitWithdrawPerMonth << " " << toAccount->currency << ").\n";
			cout << "Enter a new amount to transfer: ";
			cin >> amount;
			convertedAmount = convertCurrency(amount, fromAccount->currency, toAccount->currency);
		}
	} while (convertedAmount > toAccount->limitWithdrawPerMonth);

	string currentDate = getCurrentDate();
	fromAccount->balance -= amount;
	fromAccount->txn = addTransaction(fromAccount->txn, currentDate, -amount);
	toAccount->balance += convertedAmount;
	toAccount->txn = addTransaction(toAccount->txn, currentDate, convertedAmount);

	cout << "Transfer successful!\n";
	cout << "Transferred " << amount << " " << fromAccount->currency << " (Converted: " << convertedAmount << " " << toAccount->currency << ")\n";
	cout << "Transaction Date: " << currentDate << "\n";
}
account* chooseAccount(user* targetUser) {
	if (targetUser->acc == NULL) {
		cout << "This user has no accounts.\n";
		return NULL;
	}

	account* currentAccount = targetUser->acc;
	int accountIndex = 1;


	cout << "Select an account from the list below:\n";
	while (currentAccount != NULL) {
		cout << accountIndex << ". IBAN: " << currentAccount->IBAN << ", Account Name: " << currentAccount->accountName << ", Balance: " << fixed << setprecision(0) << currentAccount->balance << " " << currentAccount->currency << "\n";
		currentAccount = currentAccount->next;
		accountIndex++;
	}

	int choice = 0;
	do {
		cout << "Enter the account number to select: ";
		cin >> choice;

		if (choice < 1 || choice >= accountIndex) {
			cout << "Invalid choice. Please try again.\n";
		}
	} while (choice < 1 || choice >= accountIndex);


	currentAccount = targetUser->acc;
	for (int i = 1; i < choice; ++i) {
		currentAccount = currentAccount->next;
	}

	return currentAccount;
}



string formatDate(int day, int month, int year) {
	string formattedDate = "";

	if (day < 10) formattedDate += "0";
	formattedDate += to_string(day) + "/";

	if (month < 10) formattedDate += "0";
	formattedDate += to_string(month) + "/";

	formattedDate += to_string(year);
	return formattedDate;
}
bool isValidDate(int day, int month, int year) {
	time_t now = time(0);
	struct tm current;

	localtime_s(&current, &now);

	int currentYear = 1900 + current.tm_year;
	int currentMonth = 1 + current.tm_mon;
	int currentDay = current.tm_mday;

	if (year > currentYear || (year == currentYear && month > currentMonth) || (year == currentYear && month == currentMonth && day >= currentDay)) {
		cout << "The entered date must be before the current date.\n";
		return false;
	}

	return true;
}
void deleteTransactionsBeforeDate(userList* list, int day, int month, int year) {
	string inputDate = formatDate(day, month, year);

	user* currentUser = list->head;

	while (currentUser != NULL) {
		account* currentAccount = currentUser->acc;

		while (currentAccount != NULL) {
			transaction* currentTransaction = currentAccount->txn;
			transaction* prevTransaction = NULL;

			while (currentTransaction != NULL) {
				if (currentTransaction->date < inputDate) {
					if (prevTransaction == NULL) {
						currentAccount->txn = currentTransaction->next;
						delete currentTransaction;
						currentTransaction = currentAccount->txn;
					}
					else {
						prevTransaction->next = currentTransaction->next;
						delete currentTransaction;
						currentTransaction = prevTransaction->next;
					}
				}
				else {
					prevTransaction = currentTransaction;
					currentTransaction = currentTransaction->next;
				}
			}
			currentAccount = currentAccount->next;
		}
		currentUser = currentUser->next;
	}

	cout << "Transactions before " << inputDate << " have been deleted successfully.\n";
}



void writeToFile(userList* list) {
	ofstream outFile("updated.txt");

	user* currentUser = list->head;

	while (currentUser != NULL) {
		outFile << "-" << currentUser->userID << "," << currentUser->fname << "," << currentUser->lname << endl;

		account* currentAccount = currentUser->acc;

		while (currentAccount != NULL) {
			outFile << "#" << currentAccount->IBAN << "," << currentAccount->accountName << "," << fixed << setprecision(0) << currentAccount->balance << currentAccount->currency << "," << currentAccount->limitDepositPerDay << currentAccount->currency << "," << currentAccount->limitWithdrawPerMonth << currentAccount->currency << endl;

			transaction* currentTxn = currentAccount->txn;

			while (currentTxn != NULL) {
				outFile << "*" << currentTxn->date << "," << fixed << setprecision(0) << currentTxn->amount << currentAccount->currency << endl;
				currentTxn = currentTxn->next;
			}
			currentAccount = currentAccount->next;
		}
		currentUser = currentUser->next;
	}
	outFile.close();
	cout << "Data has been successfully written to updated.txt." << endl;
}

transaction* getMiddle(transaction* head, transaction* last) {
	if (head == NULL)
		return NULL;

	int count = 0;
	transaction* temp = head;
	while (temp != last->next) {
		count++;
		temp = temp->next;
	}

	int middle = count / 2;
	transaction* middleNode = head;

	for (int i = 0; i < middle; i++) {
		middleNode = middleNode->next;
	}

	return middleNode;
}
bool binarySearchTransaction(transaction* head, string& targetDate) {
	transaction* current = head;
	transaction* last = NULL; 

	while (current != NULL) {
		last = current;
		current = current->next;
	}

	while (current != last) {
		transaction* mid = getMiddle(head, last);

		if (mid == NULL)
			return false;

		if (mid->date == targetDate) {
			cout << "Transaction Found: \n";
			cout << "  Date: " << mid->date << "\n";
			cout << "  Amount: " << mid->amount << "\n";
			return true;
		}
		else if (mid->date < targetDate) {
			current = mid->next;
		}
		else {
			last = mid;
		}
	}

	return false;
}
void searchTransactionByUser(userList* list) {
	string fname, lname, targetDate;
	int day, month, year;

	cout << "Enter user's first name: ";
	cin.ignore();
	getline(cin, fname);
	cout << "Enter user's last name: ";
	getline(cin, lname);

	user* targetUser = findUser(list, fname, lname);

	cout << "Enter the transaction date:\n";
	cout << "Day (1-31): ";
	cin >> day;
	cout << "Month (1-12): ";
	cin >> month;
	cout << "Year: ";
	cin >> year;


	targetDate = formatDate(day, month, year);


	account* currentAccount = NULL;

	if (targetUser != nullptr) {
		currentAccount = targetUser->acc;

	}
	else {
		cout << "User not found, unable to access account information." << endl;
	}
	while (currentAccount != NULL) {
		cout << "Searching in account: " << currentAccount->IBAN << "\n";


		currentAccount->txn = sortTransactions(currentAccount->txn);

		bool found = binarySearchTransaction(currentAccount->txn, targetDate);
		if (found == true) {
			return;
		}

		currentAccount = currentAccount->next;
	}

	cout << "No transaction found for the specified date.\n";
}

void menu() {
	cout << "\n===^ Main Menu ^===\n";
	cout << "\t---\n";
	cout << "1. Add a new account.\n";
	cout << "2. List users.\n";
	cout << "3. Make a transaction.\n";
	cout << "4. Sort accounts by date.\n";
	cout << "5. Delete transactions.\n";
	cout << "6. Binary Search a transaction of a user by date.\n";
	cout << "0. Close the program.\n";
	cout << "Choose an option: ";
}
void input(userList* list) {
	parse(list);

	int choice;
	bool running = true;

	while (running == true) {
		menu();
		cin >> choice;

		switch (choice) {
		case 1:
			cout << "Adding a new account...\n";
			manageAccountForUser(list);
			break;
		case 2:
			cout << "Listing all users...\n";
			displayData(list);
			break;
		case 3: {
			cout << "Making a transaction...\n";

			string sourceFName, sourceLName, targetFName, targetLName;
			double amount;
			user* sourceUser = NULL;
			user* targetUser = NULL;
			account* fromAccount = NULL;
			account* toAccount = NULL;

			do {
				cout << "Enter source user's first name: ";
				cin.ignore();
				getline(cin, sourceFName);
				cout << "Enter source user's last name: ";
				getline(cin, sourceLName);

				sourceUser = findUser(list, sourceFName, sourceLName);
				if (sourceUser == NULL)
					cout << "Source user not found. Please try again.\n";

			} while (sourceUser == NULL);

			do {
				cout << "Available accounts for " << sourceUser->fname << " " << sourceUser->lname << ":\n";
				fromAccount = chooseAccount(sourceUser);
				if (fromAccount == NULL)
					cout << "No valid source account chosen. Please try again.\n";

			} while (fromAccount == NULL);

			do {
				cout << "Enter target user's first name: ";
				cin.ignore();
				getline(cin, targetFName);
				cout << "Enter target user's last name: ";
				getline(cin, targetLName);

				targetUser = findUser(list, targetFName, targetLName);
				if (targetUser == NULL)
					cout << "Target user not found. Please try again.\n";

			} while (targetUser == NULL);

			do {
				cout << "Available accounts for " << targetUser->fname << " " << targetUser->lname << ":\n";
				toAccount = chooseAccount(targetUser);
				if (toAccount == NULL)
					cout << "No valid target account chosen. Please try again.\n";

			} while (toAccount == NULL);

			do {
				cout << "Enter the amount to transfer: ";
				cin >> amount;

				if (amount <= 0)
					cout << "Amount must be greater than zero. Please try again.\n";

			} while (amount <= 0);

			transferAmount(list, amount, fromAccount, toAccount);
			break;
		}
		case 4: {
			cout << "Sorting transactions by date for all accounts...\n";

			user* currentUser = list->head;
			while (currentUser != NULL) {
				account* currentAccount = currentUser->acc;

				while (currentAccount != NULL) {
					cout << "Before sorting - Account IBAN: " << currentAccount->IBAN << "\n";
					displayTransactions(currentAccount->txn);

					if (currentAccount->txn != NULL) {
						currentAccount->txn = sortTransactions(currentAccount->txn);
					}

					cout << "After sorting - Account IBAN: " << currentAccount->IBAN << "\n";
					displayTransactions(currentAccount->txn);
					cout << "-------------------------\n";

					currentAccount = currentAccount->next;
				}

				currentUser = currentUser->next;
			}

			cout << "Transactions sorted successfully!\n";
			break;
		}

		case 5: {
			cout << "Deleting transactions before a specific date...\n";

			int day, month, year;
			bool validDate = false;

			do {
				cout << "Enter the day: ";
				cin >> day;

				cout << "Enter the month: ";
				cin >> month;

				cout << "Enter the year: ";
				cin >> year;

				validDate = isValidDate(day, month, year);
				if (!validDate) {
					cout << "Please enter a valid date before the current date.\n";
				}

			} while (!validDate);

			deleteTransactionsBeforeDate(list, day, month, year);
			break;
		}
		case 6: {
			cout << "Searching for a transaction by user...\n";
			searchTransactionByUser(list);
			break;
		}
		case 0:
			cout << "Closing the program. Goodbye!\n";
			running = false;
			break;
		default:
			cout << "Invalid choice. Please try again.\n";
			break;
		}
	}

	writeToFile(list);
}


int main() {
	userList* users = new userList;
	users->head = NULL;
	users->tail = NULL;
	cout << "Welcome to our banking System!\n";
	input(users);
	return 0;
}


