# Project 1: LinkedHashmap with LRU

> SJTU CS1968-01 2025Spring 第一次大作业

## 内容概述

提供 HashMap 的接口，LinkedHashMap 的接口和 LRU（Least Recently Used）的函数接口，要求实现 HashMap 的功能、LinkedHashMap 的功能，并以此为基础实现 LRU 算法，具体的内容在下文任务一节会有介绍。

## 项目框架

### 编码

文件采用 UTF-8 编码。如果不幸添加了中文注释并使用 Dev-C++ 打开，会出现乱码。重新编码方法：

> 右击文件 → 打开方式 → 记事本 → 另存为 → 编码选择 **GB18030** → 保存
> 再用 Dev-C++ 打开即可正常显示中文。

### 内容介绍

#### 1. `class-integer.hpp`（无需修改）

整数类 `Integer`，静态变量 `counter` 记录构造 / 析构次数。
程序结束时应为 0。

示例：

```cpp
Integer a = Integer(1);
Integer *p = new Integer(2);
std::cout << a.val << " " << p->val << std::endl;
```

---

#### 2. `class-matrix.hpp`（无需修改）

矩阵类。

示例：

```cpp
Matrix<int> a = Matrix<int>(1, 2, 3);
Matrix<int> *p = new Matrix<int>(1, 2, 3);
std::cout << a << " " << *p << std::endl;
```

构造一个 `1×2`、元素全为 `3` 的矩阵。

---

#### 3. `exceptions.hpp`（无需修改）

用于 debug，也可不用。普通 `throw` 也可以。

示例：

```cpp
try {
    throw "have a try";
} catch (const char* c) {
    std::cout << c << std::endl;
}
```

---

#### 4. `lru.hpp`（todo）

包含：

* `sjtu::double_list<T>`
* `sjtu::hashmap<Key, T, Hash, Equal>`
* `sjtu::linked_hashmap<Key, T, Hash, Equal>`
* `sjtu::lru`

`linked_hashmap` 是 `hashmap` 的派生类。

模板派生类调用基类函数时需要使用 `this`。

---

#### 5. `utility.hpp`（无需修改）

包含 `pair` 类。

示例：

```cpp
sjtu::pair<Integer, Matrix<int>>
a(Integer(1), Matrix<int>(2, 2, 2));
```

### 类结构

```
lru
 └── LinkedHashMap
      └── hashmap
           └── list
```

### 代码补充细节

#### 模板中 `Hash = ...`

表示默认模板参数。
如果不显式指定，使用默认值；若指定，则使用指定类型。

对于自定义类型（如 `Integer`），需要自定义 `Hash`：

```cpp
class Hash {
public:
    unsigned int operator()(Integer lhs) const {
        return std::hash<int>()(lhs.val);
    }
};
```

---

#### Hash / Equal 的使用方式

```cpp
Hash h;
int v1 = h(key);
int v2 = Hash()(key);
```

---

#### 关于迭代器解引用空对象

解引用空指针或无效迭代器是**未定义行为**。
因此本作业要求：**必须 throw**。

---

#### 提供的测试代码

仅为 OJ 的子集，不能保证覆盖全部测试点。

---

#### LinkedHashMap 与 LRU 的区别

* LinkedHashMap：
  **仅插入时**改变顺序
* LRU：
  **插入和查询都会**改变顺序

---

#### 关于 throw

必须 `throw`，但抛出什么类型不作强制要求。

需要 `throw` 的情况：

1. 迭代器非法移动（如 `begin()--`, `end()++`）
2. 解引用空迭代器（如 `*end()`）
3. 使用 `[]` 或 `at` 时访问不存在的元素

---

### 杂项

1. `linked_hashmap` 可直接调用基类 `hashmap` 的函数
2. `list` 只有普通迭代器，无常迭代器
3. `sjtu::lru::print()` 输出格式要求如下：

```cpp
void print() {
    sjtu::linked_hashmap<Integer, Matrix<int>, Hash, Equal>::iterator it;
    for (it = mem->begin(); it != mem->end(); it++) {
        std::cout << (*it).first.val << " "
                  << (*it).second << std::endl;
    }
}
```

### 帮助

1. 时间复杂度：所有要求实现的函数为O(1),更具体地，对于扩容，由于每添加n个元素才进行一次扩容，每次扩容大概需要2n次操作，所以平均时间复杂度仍然是O(1),类似于vector的动态扩容；对于clear,也是类似，n个元素删除需要大约n次操作，但是只有存在n个元素才能进行一次clear,再次clear就不需要做n次，平均也是每插入一个元素clear一次，也就是O(1)；对于构造函数，有n个元素就要复制n个元素（深度复制），构造函数是O(n)。（构造函数也就新建一个类的时候才会调用，之后不会再调用了，实际上测试数据也不可能调用构造函数n次的hhh）
2. 对于文档里未提到的未定义行为，如一个实例erase了另一个实例的迭代器，just do nothing（没提到就说明测试代码里大概率不会出现，提到了就会出现）
3. 对于迭代器失效的情况（如insert之后hashmap扩容的情况，remove一个迭代器，这个迭代器不能继续使用的情况等）交给用户(测试代码)即可
4. 如果不特别指出用默认构造函数构造的迭代器，空迭代器就是指end()
5. linked\_hashmap继承的hashmap的模板不一定是\<Key,T,Hash,Equal>
6. 希望在提交之前都本地用valgrind测试一下,valgrind的参数如下 
~~~
   '--tool=memcheck',
    '--leak-check=full',
    '--exit-on-first-error=yes',
    f'--error-exitcode=250',
    '--quiet',
~~~
7. lru的成员函数后面的const是可以去掉的(如果你的成员变量没有使用指针)


## 任务

**作业限制**：不可使用以下头文件及类似功能头文件：

```cpp
#include <unordered_map>
#include <map>
```

---

### Subtask 1: HashMap

LinkedHashMap 在本质上是一个实现了按照插入顺序访问元素的 HashMap（哈希表），所以在介绍 LinkedHashMap 之前，我们需要先了解 HashMap。

#### 哈希表相关概念

哈希表可以只存储一些数据，检索数据是否存在，也可以存储键值对，通过键来索引到所需要的数据。这两种方式本质上是一样的，存储键值对本质也是存储一些数据（如 pair），在进行哈希的时候只对键进行哈希，把整个键值对放入哈希到的地方，就可以通过键来确定它对应的值。

哈希表存储的数据可以允许重复也可以要求不重复。本次作业存储的是键值对，要求同一个键最多只能对应一个值，新值会覆盖旧值。（所有要求会在后文汇总）

**哈希值**：就是把任意长度的输入，通过某种哈希算法，变换成某种与之对应的输出（通常是整数，用于定位）。

**哈希碰撞**：不同的输入可能会散列成相同的输出，从而不可能从散列值来唯一确定输入值。

> 在本次作业中，推荐使用 `std::hash` 来实现哈希函数。

HashMap 支持关键词对应元素的插入、查询和删除，并且这些操作的平均时间复杂度都为 **O(1)**，只会在最差情况下退化为 **O(n)**。

解决哈希碰撞有多种方法，如线性探测法、开链表法等。

#### 固定大小哈希表

有了哈希函数，我们最自然的想法就是构建一个长度为 `l` 的数组，数组的下标对应着哈希值。每当我们插入一对键值对 `(K, V)` 的时候，我们先通过哈希函数对 `K` 进行处理得到哈希值 `h`，然后将 `V` 存储到数组的第 `h` 位。这就实现了一个固定大小的哈希表。

#### 动态大小哈希表

我们需要实现的是一个封装好的数据结构供其他人（或自己）使用（如应用于 LRU），我们在开发的时候实际上并不知道数据规模的大小，所以哈希表大小的选择是一个非常关键的问题。

当哈希表大小比较小的时候，哈希碰撞概率会变高，查询复杂度会退化；当哈希表大小比较大的时候，会占用很多无用空间。

我们可以动态地改变哈希表的大小。

首先引入两个参数 `C` 和 `f`，分别是容量（Capacity）和负载因子（LoadFactor），代表着哈希表的大小，以及在某个特定容量下我们所能接受的最多元素个数占容量的比例。

我们一开始可以选择一个比较小（不建议太大）的容量，当元素个数大于 `C * f` 时，再增大 `C`，使得数据结构能够在保持良好效率的同时，不占用过多空间。具体参数可以根据自己的实现进行调整。



---

### Subtask 2: LinkedHashMap

LinkedHashMap 需要在实现 HashMap 功能的基础上，再维护插入顺序，使得我们可以按照插入顺序来访问元素。

实现方式比较简单：维护一个**双向链表**。
每次添加元素时，除了插入 HashMap，还需要插入到链表末尾。这样按照链表顺序访问元素，就可以实现按照插入顺序访问。

LinkedHashMap 的查找分为两种：

1. 返回 Key 对应的 Value
2. 返回指向 Key 的迭代器

对于以下三种操作：

* 插入元素对
* 查询 Key 对应的 Value
* 按插入顺序访问元素

都要求期望时间复杂度为 **O(1)**。

### Subtask 3: LRU Algorithm

#### LRU 相关概念

LRU（Least Recently Used，最近最少使用）是一种内存数据淘汰策略。常见场景是：当内存不足时，需要淘汰最近最少使用（被插入或被查询）的数据。

> 注：内存可以理解为一个有限长度的数组。

该算法需要一个参数 `n`，表示预设内存大小（允许存储的键值对数量）。

#### LRU 执行的操作

##### 插入（save）

可以理解为把一个键值对 `(K, V)` 放入内存：

1. 查找内存中是否存在键 `K`

   * 有：更新节点的值
   * 无：

     * 检查是否有空闲内存

       * 有：直接存入
       * 无：
         找到**最早被插入或被查询**的键值对 `(K', V')`，将其替换为 `(K, V)`，原键值对不再存在

##### 查询（get）

利用 Hash 查找内存中是否存在键 `K`：

* 未找到：返回空指针
* 找到：返回指向该对象的指针

实现关键：
**利用 LinkedHashMap 维护最早被插入或被查询的键值对，即 LRU。**

---

### 评分规则与DDL

请于1月1日前提交至ACMOJ评分计分, 数据点得分满分90分，CR得分满分10分，总共100分计。

* 30% HashMap
* 40% Linked HashMap
* 30% LRU

## Acknowledgement

感谢 2022 级蒋捷学长开发了这个大作业。

如有问题请联系本项目的发布者 `PhantomPhoenix`, 他的邮箱地址是: `logic_1729@sjtu.edu.cn`
