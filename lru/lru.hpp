#ifndef SJTU_LRU_HPP
#define SJTU_LRU_HPP

#include "utility.hpp"
#include "exceptions.hpp"
#include "class-integer.hpp"
#include "class-matrix.hpp"
#include <vector>
#include <memory>
#include <cstddef>

class Hash {
public:
	unsigned int operator () (Integer lhs) const {
		int val = lhs.val;
		return std::hash<int>()(val);
	}
};
class Equal {
public:
	bool operator () (const Integer &lhs, const Integer &rhs) const {
		return lhs.val == rhs.val;
	}
};

namespace sjtu {
template <typename T>
class double_list {
private:
    struct Node {
        std::shared_ptr<T> data;
        Node* pre;
        Node* nxt;
        Node(const T& val)
            : data(std::make_shared<T>(val)), pre(nullptr), nxt(nullptr) {}
    };

    Node* head;
    Node* tail;
    std::size_t size_;

public:
    double_list() : head(nullptr), tail(nullptr), size_(0) {}

    double_list(const double_list<T>& other) {
        head = tail = nullptr;
        Node* cur = other.head;
        while (cur) {
            insert_tail(*cur->data);
            cur = cur->nxt;
        }
        size_ = other.size_;
    }

    ~double_list() {
        clear();
    }

    class iterator {
    public:
        Node* current;
        iterator() : current(nullptr) {}
        iterator(Node* node) : current(node) {}
        iterator(const iterator& other) : current(other.current) {}
        iterator operator++(int) {
            iterator temp = *this;
            if (!current) {throw std::out_of_range("Invalid iterator");}
            current = current->nxt;
            return temp;
        }
        iterator& operator++() {
            if (!current) {throw std::out_of_range("Invalid iterator");}
            current = current->nxt;
            return *this;
        }
        iterator operator--(int) {
            iterator temp = *this;
            if (!current || !current->pre) {throw std::out_of_range("Invalid iterator");}
            current = current->pre;
            return temp;
        }
        iterator& operator--() {
            if (!current || !current->pre) {throw std::out_of_range("Invalid iterator");}
            current = current->pre;
            return *this;
        }
        T& operator*() const {
            if (!current) {throw std::out_of_range("Invalid iterator");}
            return *current->data;
        }
        T* operator->() const noexcept {
            if (!current) {return nullptr;}
            return current->data.get();
        }

        bool operator==(const iterator& rhs) const {return current == rhs.current;}
        bool operator!=(const iterator& rhs) const {return current != rhs.current;}
    };
    iterator begin() const {return iterator(head);}
	iterator get_tail() const{return iterator(tail);}
    iterator end() const { return iterator(nullptr); }

    iterator erase(iterator pos) {
        if (!pos.current) { return end();}
        Node* node_to_delete = pos.current;
        if (node_to_delete->pre) {
            node_to_delete->pre->nxt = node_to_delete->nxt;
        } else {
            head = node_to_delete->nxt;
        }
        if (node_to_delete->nxt) {
            node_to_delete->nxt->pre = node_to_delete->pre;
        } else {
            tail = node_to_delete->pre;
        }
        delete node_to_delete;
        size_--;
        return iterator(node_to_delete->nxt);
    }

    iterator insert(iterator pos, const T& value) {
        Node* new_node = new Node(value);
        if (!pos.current) {
            throw std::out_of_range("Invalid iterator");
        }
        Node* current = pos.current;
        if (current->pre) {
            current->pre->nxt = new_node;
            new_node->pre = current->pre;
        } else {
            head = new_node;
        }
        new_node->nxt = current;
        current->pre = new_node;
        size_++;
        return iterator(new_node);
    }

    void insert_head(const T& value) {
        Node* new_node = new Node(value);
        if (!head) {
            head = tail = new_node;
        } else {
            head->pre = new_node;
            new_node->nxt = head;
            head = new_node;
        }
        size_++;
    }
    void insert_tail(const T& value) {
        Node* new_node = new Node(value);
        if (!tail) {
            head = tail = new_node;
        } else {
            tail->nxt = new_node;
            new_node->pre = tail;
            tail = new_node;
        }
        size_++;
    }

    void delete_head() {
        if (!head) {
            return;
        }
        Node* node_to_delete = head;
        head = head->nxt;
        if (head) {
            head->pre = nullptr;
        } else {
            tail = nullptr;
        }
        delete node_to_delete;
        size_--;
    }
    void delete_tail() {
        if (!tail) {
            return;
        }
        Node* node_to_delete = tail;
        tail = tail->pre;
        if (tail) {
            tail->nxt = nullptr;
        } else {
            head = nullptr;
        }
        delete node_to_delete;
        size_--;
    }
    bool empty() const {
        return size_ == 0;
    }
    std::size_t size() const {
        return size_;
    }
    void clear() {
        while (head) {
            Node* node_to_delete = head;
            head = head->nxt;
            delete node_to_delete;
        }
        size_ = 0;
        head = tail = nullptr;
    }
};

template < class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class hashmap{
public:
	using value_type = pair<Key, T>;
private:
    std::vector<std::vector<value_type>> hash_table;
    size_t size;
    Hash hash;
    Equal equal;
public:
	hashmap(): size(0), hash_table(16) {}
	hashmap(const hashmap &other): size(other.size), hash_table(other.hash_table), hash(other.hash), equal(other.equal) {}
	~hashmap(){}

	hashmap & operator=(const hashmap &other){
		if (this != &other) {
            hash_table = other.hash_table;
            size = other.size;
            hash = other.hash;
            equal = other.equal;
        }
        return *this;
	}

	class iterator{
	private:
        std::vector<std::vector<value_type>>* iterlist;
        size_t bucket_idx;
        size_t element_idx;
		bool is_end;
	public:
		iterator(std::vector<std::vector<value_type>>* ptr, size_t bidx, size_t eidx, bool is_end = false)
            : iterlist(ptr), bucket_idx(bidx), element_idx(eidx), is_end(is_end) {}
		iterator(const iterator &t): iterlist(t.iterlist), bucket_idx(t.bucket_idx), element_idx(t.element_idx), is_end(t.is_end) {}
		~iterator(){}

		value_type &operator*() const {
			if(this->is_end) throw std::out_of_range("invalid");
			return (*iterlist)[bucket_idx][element_idx];
		}
		value_type *operator->() const noexcept {
			return &((*iterlist)[bucket_idx][element_idx]);
		}
		bool operator==(const iterator &rhs) const {
			if(is_end && rhs.is_end) return true;
			return (iterlist==rhs.iterlist)&&(bucket_idx==rhs.bucket_idx)&&(element_idx==rhs.element_idx);
    	}
		bool operator!=(const iterator &rhs) const {
			return !(*this==rhs);
		}
	};

	void clear(){
		for (int i=0;i<hash_table.size();i++) {
            hash_table[i].clear();
        }
        size = 0;
	}

	void expand(){
		size_t new_size = hash_table.size() * 16;
        std::vector<std::vector<value_type>> new_hash_table(new_size);
        for (size_t i = 0; i < hash_table.size(); i++) {
            for (const auto& item : hash_table[i]) {
                size_t hash_val = hash(item.first) % new_size;
                new_hash_table[hash_val].push_back(item);
            }
        }
        hash_table = std::move(new_hash_table);
	}

	iterator end() {
		return iterator(&hash_table, hash_table.size(), 0, true);
	}
	iterator find(const Key &key) {
		size_t hash_val = hash(key)%hash_table.size();
		for(int i=0;i<hash_table[hash_val].size();i++)
			if(equal(hash_table[hash_val][i].first, key))
				return iterator(&hash_table, hash_val, i, false);
		return end();
	}
    pair<iterator, bool> insert(const value_type& value_pair) {
        if (size >= hash_table.size() * 2) expand();
        size_t hash_val = hash(value_pair.first) % hash_table.size();
        for (size_t i = 0; i < hash_table[hash_val].size(); i++)
            if (equal(hash_table[hash_val][i].first, value_pair.first)){
				hash_table[hash_val][i].second=value_pair.second;
                return pair(iterator(&hash_table, hash_val, i), false);
			}
        hash_table[hash_val].push_back(value_pair);
        ++size;
        return pair(iterator(&hash_table, hash_val, hash_table[hash_val].size() - 1, false), true);
    }
	bool remove(const Key &key){
		size_t hash_val = hash(key)%hash_table.size();
		for(int i=0;i<hash_table[hash_val].size();i++)
			if(equal(hash_table[hash_val][i].first, key)){
				hash_table[hash_val].erase(hash_table[hash_val].begin()+i);
				size--;
				return true;
			}
		return false;
	}
};

template<class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class linked_hashmap {
public:
	typedef pair<Key, T> value_type;
private:
	double_list<value_type> linked_hash_table;
	hashmap<Key, typename double_list<value_type>::iterator, Hash, Equal> hash_map;
public:
	class const_iterator;
	class iterator{
	public:
		typename double_list<value_type>::iterator it;
		iterator(){}
		iterator(typename double_list<value_type>::iterator it) : it(it) {}
		iterator(const iterator &other): it(other.it) {}
		~iterator(){}
		iterator operator++(int) {
			iterator tmp=*this;
			it++;
			return tmp;
		} 
		iterator &operator++() {
			it++;
			return *this;
		}
		iterator operator--(int) {
			iterator tmp=*this;
			it--;
			return tmp;
		} 
		iterator &operator--() {
			it--;
			return *this;
		}
		value_type &operator*() const {	return *it;	} 
		value_type *operator->() const noexcept {return &(*it);	}
		bool operator==(const iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const iterator &rhs) const {return it!=rhs.it;}
		bool operator==(const const_iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const const_iterator &rhs) const {return it!=rhs.it;}
	};
	class const_iterator {
	public:
		typename double_list<value_type>::iterator it;
		const_iterator() {}
		const_iterator(typename double_list<value_type>::iterator it) : it(it) {}
		const_iterator(const iterator &other): it(other.it){}
		const_iterator operator++(int) {
			const_iterator tmp=*this;
			it++;
			return tmp;
		}
		const_iterator &operator++() {
			it++;
			return *this;
		}
		const_iterator operator--(int) {
			const_iterator tmp=*this;
			it--;
			return tmp;
		}
		const_iterator &operator--() {
			it--;
			return *this;
		}
		const value_type &operator*() const {return *it;}
		const value_type *operator->() const noexcept {	return &(*it);} 
		bool operator==(const iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const iterator &rhs) const {return it!=rhs.it;}
		bool operator==(const const_iterator &rhs) const {return it==rhs.it;}
		bool operator!=(const const_iterator &rhs) const {return it!=rhs.it;}
	};
	linked_hashmap() {}
	linked_hashmap(const linked_hashmap &other): hash_map(other.hash_map){
		for(auto it=other.linked_hash_table.begin();it!=other.linked_hash_table.end();it++)
			linked_hash_table.insert_tail(*it);
	}
	~linked_hashmap() {
		linked_hash_table.clear();
		hash_map.clear();
	}
	linked_hashmap & operator=(const linked_hashmap &other) {
		if(this != &other){
			hash_map = other.hash_map;
			linked_hash_table.clear();
			for(auto it=other.linked_hash_table.begin();it!=other.linked_hash_table.end();it++)
				linked_hash_table.insert_tail(*it);
		}
		return *this;
	}
	T & at(const Key &key) {
        auto it = hash_map.find(key);
        if (it == hash_map.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	const T & at(const Key &key) const {
    	auto it = hash_map.find(key);
        if (it == hash_map.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	T & operator[](const Key &key) {
        auto it = hash_map.find(key);
        if (it == hash_map.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	const T & operator[](const Key &key) const {
        auto it = hash_map.find(key);
        if (it == hash_map.end()) throw std::out_of_range("Key not found");
        return it->second->second;
	}
	iterator begin() {	return iterator(linked_hash_table.begin());	}
	const_iterator cbegin() const {	return const_iterator(linked_hash_table.begin());}
	iterator end() {return iterator(linked_hash_table.end());}
	const_iterator cend() const {return const_iterator(linked_hash_table.end());}
	bool empty() const {return linked_hash_table.empty();	}
    void clear(){
		hash_map.clear();
		linked_hash_table.clear();
	}
	size_t size() const {return linked_hash_table.size();}
	pair<iterator, bool> insert(const value_type &value) {
		auto it = hash_map.find(value.first);
		if(it!=hash_map.end()){
			linked_hash_table.erase(it->second);
			linked_hash_table.insert_tail(value);
			auto new_pos = linked_hash_table.get_tail();
        	it->second = new_pos;
    	    return {iterator(new_pos), false};
		}
		else{
			linked_hash_table.insert_tail(value);
			auto lit = linked_hash_table.get_tail();
        	hash_map.insert({value.first, lit});
        	return {iterator(lit), true};
		}
	}
	void remove(iterator pos) {
		if(pos==end()) throw std::out_of_range("Key not found");
		hash_map.remove(pos->first);
		linked_hash_table.erase(pos.it);
	}
	size_t count(const Key &key) {
		auto it = hash_map.find(key);
		if(it==hash_map.end())
			return 0;
		return 1;
	}
	iterator find(const Key &key) {
		auto it = hash_map.find(key);
		if(it==hash_map.end()) return end();
		return iterator(it->second);
	}
};

class lru{
    using lmap = sjtu::linked_hashmap<Integer,Matrix<int>,Hash,Equal>;
    using value_type = sjtu::pair<const Integer, Matrix<int> >;

	size_t capacity;
	lmap hash_map;
public:
    lru(size_t size): capacity(size){}
    ~lru(){}

    void save(const value_type &v) {
		if (hash_map.count(v.first)) {
    		hash_map.remove(hash_map.find(v.first));
    	}
    	if (hash_map.size() >= capacity) {
        	auto oldest = hash_map.begin();
        	hash_map.remove(oldest);
    	}
    	hash_map.insert(v);
    }
    Matrix<int>* get(const Integer &v) {
		auto it=hash_map.find(v);
		if(it!=hash_map.end()) {
			auto value = it->second;
			hash_map.remove(it);
			hash_map.insert({v,value});
			return &(hash_map.find(v)->second);
		}
		return nullptr;
    }
    void print(){
		for (auto it = hash_map.begin(); it != hash_map.end(); ++it) {
        	std::cout << it->first.val << " " << it->second << std::endl;
    	}
    }
};
};

#endif