#include <iostream>
#include <string.h>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"


// 菜单
void print()
{
    std::cout << std::endl;
	std::cout << "********************* A tiny KV storage based on Skiplist *********************" << std::endl;
	std::cout << "*******************************************************************************" << std::endl;
    std::cout << "*******************************************************************************" << std::endl;
    std::cout << "***** 1.insert     2.search     3.delete      4.display_list  5.list_size *****" << std::endl;
	std::cout << "***** 6.dump_file  7.load_file  8.clear_list  9.update_value  10.exit     *****" << std::endl;
    std::cout << "*******************************************************************************" << std::endl;
	std::cout << "*******************************************************************************" << std::endl;
    std::cout << "Please enter the number to start the operation:  ";
}

bool keyIsValid(const std::string &str)
{
    if (str.size() > 5)
        return false;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

int main()
{
	std::cout << "Please enter the max_level of the skip list" << std::endl;
	
	int max_level;
	std::cin >> max_level;
	Skiplist<int, std::string> Skiplist(max_level);

    print();
	
    std::string flagstr = "";
    int flag = 0;
    while (std::cin >> flagstr)
    {
        if (flagstr == "exit" || flagstr == "EXIT" )
        {
            break;
        }
        if (!keyIsValid(flagstr))
        {
            std::cout << "The enter is invalid,  please enter again:" << std::endl;
            continue;
        }
		
        flag = stoi(flagstr);
        std::cout << std::endl;
        switch (flag)
        {
        case 1:
        {
            int key;
            std::string keystr;
            std::string value;
            std::cout << "Please enter key:  ";

            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            std::cout << "Please enter value:  ";
            std::cin >> value;
            Skiplist.insert_element(key, value);
			break;
        }
        case 2:
        {
            int key;
            std::string keystr;
            std::cout << "Please enter search_key:  ";
            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            Skiplist.search_element(key);
            break;
        }
        case 3:
        {
            int key;
            std::string keystr;
            std::cout << "Please enter delete_key:  ";
            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            Skiplist.delete_element(key);
            break;
        }
        case 4:
        {
            Skiplist.display_list();
            break;
        }
        case 5:
        {
            std::cout << "Skiplist size:" << Skiplist.size() << std::endl;
            break;
        }
        case 6:
        {
            Skiplist.dump_file();
            break;
        }
        case 7:
        {
            Skiplist.load_file();
            break;
        }
        case 8:
        {
            Skiplist.clear_list();
            break;
        }
	case 9:
        {
	    	int key;
            std::string keystr;
            std::string value;
	    	std::string flagstr;
			bool flag = false;
            std::cout << "Please enter key:  ";

            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            std::cout << "Please enter value:  ";
            std::cin >> value;
			std::cout << "Please enter flag:  ";
            std::cin >> flagstr;
			if (flagstr == "true") {
				flag = true;
			} else if(flagstr == "false") {
				flag = false;
			}
            Skiplist.update_element(key, value, flag);
            break;
        }
	case 10:
        {
            return 0;
        }
	case 999:
        {
            return 0;
        }
    default:
            std::cout << "your input is not right,  please input again" << std::endl;
            break;
        }
	
        print();
    }


    return 0;
}
