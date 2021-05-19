/*
 Nazim Aliev
 nazim.ru@gmail.com
*/

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <shared_mutex>
#include <thread>

using namespace std;

class Ident
{
private:
	static shared_mutex sm;
	unique_lock<shared_mutex> m_ul {sm, defer_lock};
	
	string m_ident;	// store input data
	vector<int> m_v;	// mapped ident to numbers
	string _inc();		// internal increment function
	bool check(string ident);	// check wrong symbols

public:
	// default constructor
	Ident();
	~Ident();

	void set(string ident);	// set ident
	string inc();			// increment ident
};
