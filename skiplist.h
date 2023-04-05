/*
注意：
	使用模板时，要将模板的声明和定义都放置在同一个.h文件中，（迟后联编/动态联编）
	当实例化一个模板时，编译器要看到模板确切的定义，而不仅仅是声明部分。
	例如：STL标准库中头文件都包含模板定义。
*/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include<memory>

#define STORE_FILE "store/dumpFile"

std::mutex mtx, mtx1; // 临界区互斥锁
std::string delimiter = ":"; // 定义一个分隔符


// 节点类
template<typename K, typename V>
class Node {
public:
    Node() {}

    Node(K k, V v, int);

    ~Node();

    K get_key() const;  // const 不允许更改类内成员

    V get_value() const;

    void set_value(V);

    Node<K, V> **forward; // 保存指向下一个不同级别节点的指针的线性数组

    int node_level;
	
private:
    K key;
    V value;
};

// 节点类构造函数
// level是该节点在跳表中有多少层，创建节点时进行随机分配
template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    this->forward = new Node<K, V>*[level + 1]; // 数组大小，树高level + 1
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
};

// 节点类析构函数
template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
};

// 获取 key
template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
};

// 获取 value
template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
};

// 设置 value
template<typename K, typename V>
void Node<K, V>::set_value(V value) {
    this->value = value;
}

// 跳表类定义
template<typename K, typename V>
class Skiplist {
public:
    Skiplist(int);
    ~Skiplist();

    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    int update_element(K, V, bool);
    void dump_file();
    void load_file();
    int size();
	void clear_list();

private:
    void get_key_value_from_string(const std::string &str, std::shared_ptr<int> key, std::string *value);
    bool is_valid_string(const std::string& str);

private:
    int _max_level; 		// 跳表最大层数
    int _skip_list_level;  	// 当前层数

    Node<K, V>* _header; 	// 头节点

    std::ofstream _file_writer;
    std::ifstream _file_reader;

    int _element_count;     // 跳表当前元素个数
};

// 跳表类构造函数
template<typename K, typename V>
Skiplist<K, V>::Skiplist(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // 创建跳表的头节点初始化 key 和 value 为空值
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, max_level);
};

// 创建节点
template<typename K, typename V>
Node<K, V>* Skiplist<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V>* n = new Node<K, V>(k, v, level);
    return n;
}

// 插入节点
// update 数组存放插入位置的上一个节点
template<typename K, typename V>
int Skiplist<K, V>::insert_element(const K key, const V value) {
    mtx.lock();

    Node<K, V> *current = this->_header;

    // 创建并初始化 update 数组，update[i]是第 i 层中 key 最后一个比插入 key 小的节点
    Node<K, V> *update[_max_level + 1];
	memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // 从最高层开始搜索，更新 update
    for(int i = _skip_list_level; i >= 0; --i) {
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    // 从第0层开始，current->forward[0]为应该插入的位置
    current = current->forward[0];

    // 该 key 已存在，解锁后直接返回
    if(current != nullptr && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // current 空，表示到达了该层的末尾，插入；不为空则在 update[0] 和 current 之间插入
    if(current == nullptr || current->get_key() != key) {
        // 得到一个随机层数
        int random_level = get_random_level();

        // 如果随机层数比当前的层数高
        if(random_level > _skip_list_level) {
            // 新建一层，该节点在该层，也就是最高层的前一个是 _header
            for(int i = _skip_list_level + 1; i < random_level + 1; ++i) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // 创建节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);

        // 插入节点
        for(int i = 0; i < random_level; ++i) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }

        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count++;   // 跳表当前元素个数加一
    }

    mtx.unlock();
    return 0;
}

// 搜索节点
// 从顶层的 _header 开始，从上而下，从左到右，依次遍历每层的节点 key，直到第0层
template<typename K, typename V>
bool Skiplist<K, V>::search_element(K key) {
    std::cout << "search_element------------" << std::endl;

    Node<K, V> *current = _header;

    // 从最高层开始，直到第0层
    for(int i = _skip_list_level; i >= 0; --i) {
		// 如果当前层 next 不为空，且值小于 target，继续向右查找， p 指向 p 的 next
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }
	
	// 到第0层
    current = current->forward[0];

    if(current != nullptr && current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// 删除元素
// 先找到每层要删除的前一节点，若某层中该 key 存在，更新前一节点的指针指向
template<typename K, typename V>
void Skiplist<K, V>::delete_element(K key) {
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    // 从最高层开始搜索
    for(int i = _skip_list_level; i >= 0; --i) {
        // 找到当前层最后一个小于 key 的节点
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        // 记录层级轨迹
        update[i] = current;
    }

    // 到第0层
    current = current->forward[0];

    // 存在该节点
    if(current != nullptr && current->get_key() == key) {
        // 最底层，删除每级的当前节点
        for(int i = 0; i <= _skip_list_level; ++i) {
            //如果该层没有该节点，跳过
            if(update[i]->forward[i] != current)
                break;

            // 有该节点，让该节点的前一个节点指向该节点的后一个节点进行删除该节点
            update[i]->forward[i] = current->forward[i];
        }

        // 删除没有元素的层级
        while(_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level--;
        }

        std::cout << "Successfully deleted key " << key << std::endl;
        _element_count--;
    }else{
        std::cout << "The key is not existed: " << key << std::endl;
    }
	
    mtx.unlock();
    return ;
}

// 更新修改 value
// 1. 如果当前键存在，更新值
// 2. 如果当前键不存在，通过 flag 参数是否创建该键 (默认false)
//   2.1 flag = true ：创建 key value
//   2.2 flag = false : 返回键不存在
template<typename K, typename V>
int Skiplist<K, V>::update_element(const K key, const V value, bool flag) {
    // 先 search
    mtx1.lock(); 
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));  
    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];  
        }
        update[i] = current; 
    }
    current = current->forward[0];
	
    // 1. 插入元素已经存在
    if (current != nullptr && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        std::cout << "old value : " << current->get_value() << " --> "; // 打印 old value
        current->set_value(value);  // 重新设置 value, 并打印输出。
        std::cout << "new value : " << current->get_value() << std::endl;
        mtx1.unlock();
        return 1;  // 更新成功
    }
    // 2. 如果插入的元素不存在
    //  2.1 flag = true,允许更新创建操作,则使用 insert_element 添加
    if (flag) {
        Skiplist<K, V>::insert_element(key, value);
        mtx1.unlock();
        return 0;  // key 不存在，但是成功创建
    }
    //  2.1 flag = false, 不允许更新创建操作, 打印提示信息
    else {
        std::cout << key << " is not exist, please check your input !\n";
        mtx1.unlock();
        return -1; // key 不存在，不允许创建
    } 
}

// 展示跳表
template<typename K, typename V>
void Skiplist<K, V>::display_list() {
	if (_element_count == 0) {
        std::cout << " List is Null" << std::endl;
        return;
    }
	// 自顶向下展示跳表
    for(int i = 0; i < _skip_list_level; ++i) {
        // 指向头节点的第 i 层
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";

        while(node != nullptr) {
            std::cout << node->get_key() << ": " << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// 将内存中的数据转储到文件
template<typename K, typename V>
void Skiplist<K, V>::dump_file() {
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];   // 第0层

    while(node != nullptr) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush(); // 刷新
    _file_writer.close(); // 关闭

    return ;
}

// 从磁盘加载数据
template <typename K, typename V>
void Skiplist<K, V>::load_file()
{
    _file_reader.open(STORE_FILE);
    std::string line;

    // 智能指针
    std::shared_ptr<int> key = std::make_shared<int>();

    std::string *value = new std::string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (value->empty())
        {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key: " << key << " ,value: " << *value << std::endl;
    }

    delete value;
    _file_reader.close();
}

// 字符串中获取key、value
template <typename K, typename V>
void Skiplist<K, V>::get_key_value_from_string(const std::string &str, std::shared_ptr<int> key, std::string *value)
{

    if (!is_valid_string(str)) {
        return;
    }
    *key = stoi(str.substr(0, str.find(delimiter)));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

// 判断文件中字符串是否合法
template<typename K, typename V>
bool Skiplist<K, V>::is_valid_string(const std::string& str) {
    if(str.empty()) {
        return false;
    }
    if(str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// 跳表大小
template<typename K, typename V>
int Skiplist<K, V>::size() {
    return _element_count;
}

// 析构
template<typename K, typename V>
Skiplist<K, V>::~Skiplist() {
    if(_file_reader.is_open()) {
        _file_reader.close();
    }
    if(_file_writer.is_open()) {
        _file_writer.close();
    }
	
	//删除所有的节点信息
    Node<K, V> *current = _header->forward[0];
    while (current) {
        Node<K, V> *tmp = current->forward[0];
        delete current;
        current = tmp;
    }

    delete _header;
}

// 获得随机层数
template<typename K, typename V>
int Skiplist<K, V>::get_random_level() {
    int k = 1;
    while (rand() % 2) {
        k++; // 每+1层机会减半
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};

// 清空跳表
template<typename K, typename V>
void Skiplist<K, V>::clear_list(){
	Node<K, V> *current = _header->forward[0];

	while (current != nullptr){
		Node<K, V> *tmp = current->forward[0];
		delete current;
		current = tmp;
	}
	
	//若不清空，_header->forward[0]会保存非法值，插入时调用_header->forward[0]->get_key出错
	memset(_header->forward, 0, sizeof(Node<K, V> *) * (_skip_list_level + 1));
	_skip_list_level = 0;
	_element_count = 0;
}
