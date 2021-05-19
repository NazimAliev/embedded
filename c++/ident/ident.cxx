/*
 Nazim Aliev
 nazim.ru@gmail.com
*/

#include "ident.h"

using namespace std;

// default constructor
Ident::Ident()
{
	if(!m_ul.try_lock())
	{
		cout << "Mutex is busy" << endl;
		return;
	}
}

Ident::~Ident()
{
	m_ul.unlock();
}

shared_mutex Ident::sm;

// parse string as vector as vector of identificator numbers 
// not checking 'wrong' symbols. User is smart enough!
void Ident::set(string str)
{
	m_ident = str;
	m_v.clear();
	// no need '-' human-friendly delimiters
	str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
	// map identificator to number and store in vector
	for(unsigned int i=0; i<str.size(); i+=2)
	{	
		int x = 0;
		x += (str[i]-'A')*10;
		x += (str[i+1]-'0');
		m_v.push_back(x);
	}
}

string Ident::inc()
{
	// run increment until wrong characters will be incremented to right
	string s;
	// no good idea have a risk fall into infinity
	// let's fix it in the next release	
	for(;;)	
	{
		s = _inc();
		if(check(s))
			break;
	} 
	return s;
}

// increment worker
// map identificator to number, increment number taking account carry,
//map number to identificator 
string Ident::_inc()
{
	string str = "";
	int flag = 1;
	// reverse iteration from end of number vector
	for (auto it = m_v.rbegin(); it != m_v.rend(); ++it)
	{
		// process "carry" conditions
		auto x = *it;
		if(flag == 1)
		{
			if(x == 259)
			{
				x = 0;
				flag = 1;
			}
			else
			{
				x += 1;
				flag = 0;
			}
		}
		// map number to identificator string
		char ch0 = '0' + x % 10;
		str += ch0;
		char ch1 = 'A' + x / 10;
		str += ch1;
		// let's be as human
		str += '-';
	}
	// delete last '-'
	str.pop_back();
	reverse(begin(str), end(str));
	// clear vector and set m_ident to incremented value
	set(str);
	return str;
}

// check identificator contains wrong characters
bool Ident::check(string str)
{
	string wrong ("DFGJMQV0");
	bool res = true;
	for(auto ch : str)
	{
		if (wrong.find(ch) != std::string::npos)
		{
			res = false;
			break;
		}
	}
	return res;
}
