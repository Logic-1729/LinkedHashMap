#include "src.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <cassert>
#include <string>

// 辅助断言函数，失败时打印信息并退出
void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << std::endl;
        exit(1);
    }
}

// ==========================================
// Test 1: LRU 严格逻辑测试 (Strict Logic)
// ==========================================
void test_lru_strict_logic() {
    std::cout << "[Running] LRU Strict Logic Test..." << std::endl;
    
    // 初始化容量为 3 的 LRU
    sjtu::lru cache(3);
    
    // 1. 填满缓存
    // 插入顺序: 1, 2, 3
    // 缓存状态 (MRU -> LRU): 3 -> 2 -> 1
    cache.save(sjtu::pair<Integer, Matrix<int>>(Integer(1), Matrix<int>(1, 1, 10)));
    cache.save(sjtu::pair<Integer, Matrix<int>>(Integer(2), Matrix<int>(1, 1, 20)));
    cache.save(sjtu::pair<Integer, Matrix<int>>(Integer(3), Matrix<int>(1, 1, 30)));
    
    // 验证基本存在性
    assert_true(cache.get(Integer(1)) != nullptr, "Key 1 should exist");
    assert_true(cache.get(Integer(2)) != nullptr, "Key 2 should exist");
    assert_true(cache.get(Integer(3)) != nullptr, "Key 3 should exist");
    
    // 2. 访问 Key 1
    // 预期行为: 1 变为最近使用
    // 缓存状态 (MRU -> LRU): 1 -> 3 -> 2
    Matrix<int>* val = cache.get(Integer(1));
    // 修复：使用 public 的 operator[] 访问元素
    assert_true(val != nullptr && (*val)[0][0] == 10, "Key 1 value incorrect");
    
    // 3. 插入 Key 4 (触发淘汰)
    // 预期行为: 淘汰最久未使用的 Key 2
    // 缓存状态 (MRU -> LRU): 4 -> 1 -> 3
    cache.save(sjtu::pair<Integer, Matrix<int>>(Integer(4), Matrix<int>(1, 1, 40)));
    
    assert_true(cache.get(Integer(2)) == nullptr, "Key 2 should have been evicted (LRU policy failed)");
    assert_true(cache.get(Integer(1)) != nullptr, "Key 1 should still exist (it was accessed recently)");
    assert_true(cache.get(Integer(3)) != nullptr, "Key 3 should still exist");
    assert_true(cache.get(Integer(4)) != nullptr, "Key 4 should exist");
    
    // 4. 更新 Key 3
    // 预期行为: 更新值，并将其提至 MRU
    // 缓存状态 (MRU -> LRU): 3 -> 4 -> 1
    cache.save(sjtu::pair<Integer, Matrix<int>>(Integer(3), Matrix<int>(1, 1, 300)));
    
    val = cache.get(Integer(3));
    // 修复：使用 public 的 operator[] 访问元素
    assert_true(val != nullptr && (*val)[0][0] == 300, "Key 3 should be updated");
    
    // 5. 再次插入 Key 5 (触发淘汰)
    // 预期行为: 淘汰 Key 1 (目前是 LRU)
    // 缓存状态 (MRU -> LRU): 5 -> 3 -> 4
    cache.save(sjtu::pair<Integer, Matrix<int>>(Integer(5), Matrix<int>(1, 1, 50)));
    
    assert_true(cache.get(Integer(1)) == nullptr, "Key 1 should have been evicted");
    assert_true(cache.get(Integer(4)) != nullptr, "Key 4 should still exist");
    assert_true(cache.get(Integer(3)) != nullptr, "Key 3 should still exist");
    
    std::cout << "[PASS] LRU Strict Logic Test" << std::endl;
}

// ==========================================
// Test 2: 哈希冲突压力测试 (Hash Collision)
// ==========================================
// 故意制造糟糕的哈希函数，所有数字都映射到 [0, 9]
struct BadHash {
    size_t operator()(const Integer& i) const {
        return i.val % 10; 
    }
};

void test_hash_collisions() {
    std::cout << "[Running] Hash Collision Test..." << std::endl;
    
    // 使用 BadHash
    using map_t = sjtu::hashmap<Integer, int, BadHash, Equal>;
    map_t map;
    
    int count = 200;
    
    // 1. 插入大量数据 (大量冲突)
    for (int i = 0; i < count; ++i) {
        map.insert(sjtu::pair<Integer, int>(Integer(i), i));
    }
    
    // 2. 验证所有数据都能找到 (find 鲁棒性)
    for (int i = 0; i < count; ++i) {
        auto it = map.find(Integer(i));
        assert_true(it != map.end(), "Key " + std::to_string(i) + " missing under high collision");
        assert_true((*it).second == i, "Value mismatch for key " + std::to_string(i));
    }
    
    // 3. 删除偶数 Key (remove 鲁棒性)
    for (int i = 0; i < count; i += 2) {
        map.remove(Integer(i));
    }
    
    // 4. 验证删除结果
    for (int i = 0; i < count; ++i) {
        auto it = map.find(Integer(i));
        if (i % 2 == 0) {
            assert_true(it == map.end(), "Key " + std::to_string(i) + " should be removed");
        } else {
            assert_true(it != map.end(), "Key " + std::to_string(i) + " should still exist");
            assert_true((*it).second == i, "Value corruption for key " + std::to_string(i));
        }
    }
    
    std::cout << "[PASS] Hash Collision Test" << std::endl;
}

// ==========================================
// Test 3: LinkedHashMap 随机化对拍 (Fuzzing)
// ==========================================
void test_linked_hashmap_fuzz() {
    std::cout << "[Running] LinkedHashMap Randomized Order Test..." << std::endl;
    
    // 修复：显式指定 Hash 和 Equal，因为 Integer 没有默认的 std::hash
    sjtu::linked_hashmap<Integer, int, Hash, Equal> lhm;
    
    // 使用 std::list 维护预期的“插入顺序”
    std::list<std::pair<int, int>> ref_list;
    // 使用 std::map 辅助快速定位 list 中的元素以便删除
    std::unordered_map<int, std::list<std::pair<int, int>>::iterator> ref_map;
    
    const int OPS = 2000;
    const int KEY_RANGE = 100;
    std::mt19937 rng(12345); // 固定种子以便复现
    
    for (int k = 0; k < OPS; ++k) {
        int op = rng() % 3; // 0: Insert, 1: Remove, 2: Validate
        int key_val = rng() % KEY_RANGE;
        
        if (op == 0) { // INSERT
            int val = rng();
            Integer key(key_val);
            
            // 如果 Key 已存在
            if (ref_map.count(key_val)) {
                // 为了测试“顺序维护”，我们策略是：先删除旧的，再插入新的，
                // 这样该元素必须被移动到链表末尾。
                
                // 修复：使用 iterator 进行删除
                auto it = lhm.find(key);
                if (it != lhm.end()) lhm.remove(it);
                
                ref_list.erase(ref_map[key_val]);
                ref_map.erase(key_val);
            }
            
            lhm.insert(sjtu::pair<Integer, int>(key, val));
            
            ref_list.push_back({key_val, val});
            ref_map[key_val] = --ref_list.end();
            
        } else if (op == 1) { // REMOVE
            Integer key(key_val);
            
            if (ref_map.count(key_val)) {
                // 修复：使用 iterator 进行删除
                auto it = lhm.find(key);
                if (it != lhm.end()) lhm.remove(it);
                
                ref_list.erase(ref_map[key_val]);
                ref_map.erase(key_val);
            } else {
                // 删除不存在的元素，不应崩溃
                // 修复：使用 iterator 进行删除
                auto it = lhm.find(key);
                if (it != lhm.end()) lhm.remove(it);
            }
            
        } else { // VALIDATE ORDER
            auto it = lhm.begin();
            auto ref_it = ref_list.begin();
            int index = 0;
            
            while (it != lhm.end() && ref_it != ref_list.end()) {
                if ((*it).first.val != ref_it->first) {
                    std::cerr << "Order Mismatch at index " << index << std::endl;
                    std::cerr << "Expected Key: " << ref_it->first << ", Actual Key: " << (*it).first.val << std::endl;
                    exit(1);
                }
                if ((*it).second != ref_it->second) {
                    std::cerr << "Value Mismatch at key " << ref_it->first << std::endl;
                    exit(1);
                }
                it++;
                ref_it++;
                index++;
            }
            
            if (it != lhm.end() || ref_it != ref_list.end()) {
                std::cerr << "Size Mismatch! LinkedHashMap size vs Reference size differs." << std::endl;
                exit(1);
            }
        }
    }
    
    std::cout << "[PASS] LinkedHashMap Randomized Order Test" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Strengthened Unit Tests for Project  " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_lru_strict_logic();
        std::cout << "----------------------------------------" << std::endl;
        test_hash_collisions();
        std::cout << "----------------------------------------" << std::endl;
        test_linked_hashmap_fuzz();
        
        std::cout << "========================================" << std::endl;
        std::cout << "   ALL TESTS PASSED :)                  " << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (const char* msg) {
        std::cerr << "Caught exception: " << msg << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Caught unknown exception!" << std::endl;
        return 1;
    }
    
    return 0;
}