#include <iostream>
using namespace std;

int main(){
	cout << std::hex << std::uppercase;
	
	cout << "BEGIN LIST ALL VALUES" << endl;
	cout << "---------------------" << endl;
	int sum = 0;
	for (int i = 26; i < 226; i++){
			cout << i << "  ";
			sum = sum + i;
	}
	cout << endl;
	cout << "SUM: " << sum << endl << endl;
	
	cout << "BEGIN LIST VALUES DIVISIBLE BY 3" << endl;
	cout << "--------------------------------" << endl;
	sum = 0;
	for (int i = 26; i < 226; i++)
		if ((i % 3) == 0){
			cout << i << "  ";
			sum = sum + i;
		}
	cout << endl;
	cout << "SUM: " << sum << endl << endl;
	
	cout << "BEGIN LIST VALUES DIVISIBLE BY 7" << endl;
	cout << "--------------------------------" << endl;
	sum = 0;
	for (int i = 26; i < 226; i++)
		if ((i % 7) == 0){
			cout << i << "  ";
			sum = sum + i;
		}
	cout << endl;
	cout << "SUM: " << sum << endl << endl;
	
	cout << "BEGIN LIST VALUES DIVISIBLE BY 3 AND 7" << endl;
	cout << "--------------------------------------" << endl;
	sum = 0;
	for (int i = 26; i < 226; i++)
		if ((i % 7) == 0 and (i % 3) == 0){
			cout << i << "  ";
			sum = sum + i;
		}
	cout << endl;
	cout << "SUM: " << sum << endl << endl;
	
	cout << "BEGIN LIST VALUES DIVISIBLE NEITHER 3 OR 7" << endl;
	cout << "------------------------------------------" << endl;
	sum = 0;
	for (int i = 26; i < 226; i++)
		if ((i % 7) != 0 and (i % 3) != 0){
			cout << i << "  ";
			sum = sum + i;
		}
	cout << endl;
	cout << "SUM: " << sum << endl << endl;
}