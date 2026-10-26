// ============================================================================
//  经典数据结构实现 - 老师傅带徒弟系列
// ============================================================================
//
//  徒儿啊，今天咱们来聊聊数据结构。
//  俗话说得好："程序 = 数据结构 + 算法"，算法你已经学了不少，
//  这数据结构也得跟上才行。数据结构就是组织和存储数据的方式，
//  选对了数据结构，算法才能如虎添翼。
//
//  今天为师就带你实现几个最经典的数据结构，一个一个来，不着急。
//
//  编译命令：g++ -std=c++17 -O2 data_structures.cpp -o data_structs
//
// ============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stdexcept>
#include <sstream>
#include <functional>
#include <memory>

using namespace std;

// ============================================================================
//  【一】动态数组（Vector）
// ============================================================================
//
//  【原理说明】
//  徒儿，普通数组大小是固定的，用起来不方便对吧？
//  动态数组就是能"自动扩容"的数组。底层还是用的普通数组，
//  但当元素装不下的时候，就申请一块更大的内存（一般是原来的2倍），
//  把老数据拷过去，再把老内存释放掉。这样用户用起来就感觉不到大小限制了。
//
//  【核心操作】
//  - push_back：在末尾添加元素    平均O(1)，扩容时O(n)
//  - pop_back：删除末尾元素       O(1)
//  - 随机访问：用下标直接读/写    O(1)
//  - 中间插入/删除：要挪动元素    O(n)
//
//  【复杂度分析】
//  - 访问：O(1)
//  - 尾部插入删除：平均O(1)（摊还分析）
//  - 任意位置插入删除：O(n)
//  - 空间：O(n)
//
//  老师傅说：C++的std::vector就是干这个的，日常开发用得最多。
//  特点就是快——随机访问快，尾部操作快，缓存友好。
//  但中间插删就慢了，得挪动后面所有元素。
//
// ============================================================================

template <typename T>
class MyVector {
private:
    T* data;          //  底层数组指针
    int capacity;     //  当前容量（能装多少个）
    int size;         //  当前元素个数

    //  扩容函数：当装不下了就调用
    void expand() {
        //  新容量取原来的2倍（如果原来是0就取1）
        int newCapacity = (capacity == 0) ? 1 : capacity * 2;
        //  用 unique_ptr 临时托管新数组：如果元素复制赋值抛异常，
        //  新数组会自动释放，不会泄漏（基本的异常安全）。
        unique_ptr<T[]> newData(new T[newCapacity]);
        //  把老数据拷过去
        for (int i = 0; i < size; i++) {
            newData[i] = data[i];
        }
        //  释放老内存，接管新内存
        delete[] data;
        data = newData.release();
        capacity = newCapacity;
    }

public:
    //  构造函数
    MyVector() : data(nullptr), capacity(0), size(0) {}

    //  析构函数
    ~MyVector() {
        delete[] data;
    }

    //  禁用拷贝构造和赋值（简化实现，避免浅拷贝问题）
    MyVector(const MyVector&) = delete;
    MyVector& operator=(const MyVector&) = delete;

    //  尾插
    void push_back(const T& val) {
        //  满了就扩容
        if (size == capacity) {
            expand();
        }
        data[size++] = val;
    }

    //  尾删
    void pop_back() {
        if (size == 0) {
            throw out_of_range("Vector是空的，没法pop！");
        }
        //  注意：这里 new T[] 已经构造了全部 capacity 个元素，
        //  所以只缩小 size 就是合法的，不能手动再调一次析构函数，
        //  否则 delete[] 时会重复析构。
        //  这是简化实现：被弹出元素的资源要等整个数组销毁或扩容时
        //  才释放；标准 vector 用 allocator 区分“已构造区间”，
        //  可以立刻析构被删除的元素。
        size--;
    }

    //  随机访问（可读可写）
    T& operator[](int index) {
        if (index < 0 || index >= size) {
            throw out_of_range("下标越界了！");
        }
        return data[index];
    }

    //  const版本的随机访问（只读）
    const T& operator[](int index) const {
        if (index < 0 || index >= size) {
            throw out_of_range("下标越界了！");
        }
        return data[index];
    }

    //  获取元素个数
    int getSize() const { return size; }

    //  获取容量
    int getCapacity() const { return capacity; }

    //  判空
    bool empty() const { return size == 0; }

    //  打印一下看看
    void print(const string& name = "") const {
        if (!name.empty()) cout << name << ": ";
        cout << "[";
        for (int i = 0; i < size; i++) {
            if (i > 0) cout << ", ";
            cout << data[i];
        }
        cout << "]  (size=" << size << ", cap=" << capacity << ")" << endl;
    }
};

// ============================================================================
//  【二】单链表（Singly Linked List）
// ============================================================================
//
//  【原理说明】
//  链表和数组不一样，它的元素在内存里不是连续放的，而是靠"指针"串起来的。
//  每个节点存两样东西：数据 + 下一个节点的地址。
//  要找某个元素，得从头节点一个一个往后找，像串糖葫芦一样。
//
//  【核心操作】
//  - 头插法：在最前面加一个节点         O(1)
//  - 尾插法：在最后面加一个节点         O(1)（有尾指针时）
//  - 查找：按值找节点                    O(n)
//  - 删除：删除指定值的节点              O(n)（得先找到它）
//  - 反转：把整个链表反过来              O(n)
//
//  【复杂度分析】
//  - 访问：O(n)（得从头找）
//  - 已知位置的插入删除：O(1)（只需要改指针）
//  - 查找：O(n)
//  - 空间：O(n)
//
//  老师傅说：链表的优势就是插删快（只要知道位置），不用挪动元素。
//  但缺点也明显——不能随机访问，想找第k个得从头数。
//  而且每个节点都要存指针，空间开销比数组大。
//
// ============================================================================

//  链表节点
template <typename T>
struct ListNode {
    T val;
    ListNode* next;
    ListNode(const T& v) : val(v), next(nullptr) {}
};

template <typename T>
class MyLinkedList {
private:
    ListNode<T>* head;  //  头节点指针
    int size;           //  元素个数

public:
    //  构造函数
    MyLinkedList() : head(nullptr), size(0) {}

    //  析构函数：把所有节点释放掉
    ~MyLinkedList() {
        while (head) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
        }
    }

    //  禁用拷贝构造和赋值
    MyLinkedList(const MyLinkedList&) = delete;
    MyLinkedList& operator=(const MyLinkedList&) = delete;

    //  头插法：新节点插在最前面
    void push_front(const T& val) {
        ListNode<T>* newNode = new ListNode<T>(val);
        newNode->next = head;  //  新节点的next指向原来的头
        head = newNode;        //  头指针更新为新节点
        size++;
    }

    //  尾插法：新节点插在最后面
    void push_back(const T& val) {
        ListNode<T>* newNode = new ListNode<T>(val);
        if (head == nullptr) {
            //  空链表，新节点就是头
            head = newNode;
        } else {
            //  找到最后一个节点
            ListNode<T>* cur = head;
            while (cur->next) {
                cur = cur->next;
            }
            cur->next = newNode;
        }
        size++;
        //  徒儿，这里你注意到没？尾插要遍历到最后，是O(n)的
        //  所以实际工程中如果经常尾插，会加一个tail指针，变成O(1)
    }

    //  查找：找到返回true
    bool find(const T& val) const {
        ListNode<T>* cur = head;
        while (cur) {
            if (cur->val == val) return true;
            cur = cur->next;
        }
        return false;
    }

    //  删除：删除第一个值为val的节点
    bool remove(const T& val) {
        //  空链表直接返回
        if (!head) return false;

        //  头节点就是要删的
        if (head->val == val) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
            size--;
            return true;
        }

        //  找要删除节点的前一个节点
        ListNode<T>* prev = head;
        while (prev->next && prev->next->val != val) {
            prev = prev->next;
        }

        //  找到了
        if (prev->next) {
            ListNode<T>* temp = prev->next;
            prev->next = temp->next;  //  跳过要删的节点
            delete temp;
            size--;
            return true;
        }
        return false;  //  没找到
    }

    //  反转链表：经典面试题！
    //  思路：三个指针prev、cur、next，逐个翻转指针方向
    void reverse() {
        ListNode<T>* prev = nullptr;
        ListNode<T>* cur = head;
        while (cur) {
            ListNode<T>* next = cur->next;  //  先把下一个存起来
            cur->next = prev;               //  当前节点指向前一个
            prev = cur;                     //  prev往前走
            cur = next;                     //  cur往前走
        }
        head = prev;  //  最后prev就是新的头
    }

    //  获取大小
    int getSize() const { return size; }

    //  打印
    void print(const string& name = "") const {
        if (!name.empty()) cout << name << ": ";
        ListNode<T>* cur = head;
        while (cur) {
            cout << cur->val;
            if (cur->next) cout << " -> ";
            cur = cur->next;
        }
        cout << " (size=" << size << ")" << endl;
    }
};

// ============================================================================
//  【三】栈（Stack）
// ============================================================================
//
//  【原理说明】
//  栈就像一摞盘子，后放的先拿——这叫"后进先出"（LIFO, Last In First Out）。
//  你只能在栈顶操作：push压入栈顶，pop弹出栈顶，top看栈顶元素。
//
//  栈可以用数组实现，也可以用链表实现。
//  - 数组实现：用一个数组+一个top指针，push和pop都在尾部，O(1)
//  - 链表实现：用头插法模拟push，头删法模拟pop，也是O(1)
//
//  【复杂度分析】
//  - push/pop/top：O(1)
//  - 空间：O(n)
//
//  老师傅说：栈看着简单，但应用可广了！
//  函数调用栈、括号匹配、表达式求值、深度优先搜索(DFS)……
//  都是栈的经典应用。
//
// ============================================================================

//  用动态数组实现栈（简单高效，推荐）
template <typename T>
class MyStack {
private:
    vector<T> data;  //  底层用vector，省得自己写扩容了

public:
    //  压栈：放到尾部
    void push(const T& val) {
        data.push_back(val);
    }

    //  弹栈：移除尾部
    void pop() {
        if (data.empty()) {
            throw out_of_range("栈是空的，没法pop！");
        }
        data.pop_back();
    }

    //  看栈顶
    T& top() {
        if (data.empty()) {
            throw out_of_range("栈是空的，没有top！");
        }
        return data.back();
    }

    const T& top() const {
        if (data.empty()) {
            throw out_of_range("栈是空的，没有top！");
        }
        return data.back();
    }

    //  判空
    bool empty() const {
        return data.empty();
    }

    //  大小
    int size() const {
        return data.size();
    }
};

// ============================================================================
//  【四】队列（Queue）
// ============================================================================
//
//  【原理说明】
//  队列就像排队买票，先来的先上——这叫"先进先出"（FIFO, First In First Out）。
//  队尾入队（enqueue），队头出队（dequeue）。
//
//  普通数组实现队列有个问题：出队了前面空出来的位置浪费了。
//  所以有了"循环队列"——把数组看成一个环，用两个指针front和rear，
//  front指向队头，rear指向队尾的下一个位置。
//  这样出队后空出来的位置还能继续用，空间不浪费。
//
//  【循环队列的关键】
//  - 判空：front == rear
//  - 判满：(rear + 1) % capacity == front（故意留一个空位区分空和满）
//  - 元素个数：(rear - front + capacity) % capacity
//
//  【复杂度分析】
//  - enqueue/dequeue/front：O(1)
//  - 空间：O(n)
//
//  老师傅说：队列也是基础中的基础，广度优先搜索(BFS)、消息队列、
//  任务调度……到处都有它的身影。循环队列是数组实现队列的标准姿势，
//  面试常考，一定要会手写！
//
// ============================================================================

template <typename T>
class MyCircularQueue {
private:
    T* data;          //  底层数组
    int capacity;     //  容量（实际能存 capacity-1 个元素，留一个空位）
    int front;        //  队头下标
    int rear;         //  队尾的下一个位置下标

public:
    //  构造函数：指定队列大小
    //  注意：因为要留一个空位，所以实际分配 k+1 个空间
    explicit MyCircularQueue(int k) {
        capacity = k + 1;
        data = new T[capacity];
        front = 0;
        rear = 0;
    }

    //  析构函数
    ~MyCircularQueue() {
        delete[] data;
    }

    //  拥有裸指针，默认浅拷贝会导致 double free；本示例不实现深拷贝，
    //  直接禁用拷贝。
    MyCircularQueue(const MyCircularQueue&) = delete;
    MyCircularQueue& operator=(const MyCircularQueue&) = delete;

    //  入队：加到队尾
    bool enqueue(const T& val) {
        if (isFull()) return false;  //  满了就加不进去
        data[rear] = val;
        rear = (rear + 1) % capacity;  //  rear后移，到末尾就绕回来
        return true;
    }

    //  出队：移除队头
    bool dequeue() {
        if (isEmpty()) return false;  //  空的出啥队
        front = (front + 1) % capacity;  //  front后移就行
        return true;
    }

    //  看队头
    T Front() const {
        if (isEmpty()) throw out_of_range("队列是空的！");
        return data[front];
    }

    //  看队尾
    T Rear() const {
        if (isEmpty()) throw out_of_range("队列是空的！");
        //  rear的前一个就是队尾，注意绕圈
        return data[(rear - 1 + capacity) % capacity];
    }

    //  判空
    bool isEmpty() const {
        return front == rear;
    }

    //  判满
    bool isFull() const {
        return (rear + 1) % capacity == front;
    }

    //  元素个数
    int size() const {
        return (rear - front + capacity) % capacity;
    }
};

// ============================================================================
//  【五】二叉搜索树（Binary Search Tree, BST）
// ============================================================================
//
//  【原理说明】
//  二叉搜索树是一种特殊的二叉树，它满足：
//  对于任意一个节点，它左子树所有节点的值都小于它，
//  右子树所有节点的值都大于它。
//
//  这个性质很重要——它让我们查找一个元素的时候，
//  每次都能排除一半的节点，就像二分查找一样！
//
//  【核心操作】
//  - 插入：从根开始比大小，小了往左，大了往右，找到空位就插  平均O(log n)
//  - 查找：同上，找到了就返回                                  平均O(log n)
//  - 删除：比较麻烦，分三种情况：                                平均O(log n)
//      1. 叶子节点：直接删
//      2. 只有一个孩子：让孩子顶替它
//      3. 两个孩子都有：找右子树的最小值（或左子树的最大值）顶替它
//  - 遍历：
//      前序（根左右）：复制树、前缀表达式
//      中序（左根右）：BST中序遍历结果就是有序的！
//      后序（左右根）：删除树、后缀表达式
//      层序（按层来）：广度优先，要用队列辅助
//
//  【复杂度分析】
//  - 平均：查找/插入/删除都是 O(log n) （树是平衡的时候）
//  - 最坏：O(n) （退化成链表了，比如按顺序插入1,2,3,4...）
//  - 空间：O(n)
//
//  老师傅说：BST的想法很美好，但有个致命问题——容易不平衡。
//  所以才有了AVL树、红黑树这些自平衡的BST。
//  C++的std::map和std::set底层就是红黑树哦！
//
// ============================================================================

//  BST节点
template <typename T>
struct BSTNode {
    T val;
    BSTNode* left;
    BSTNode* right;
    BSTNode(const T& v) : val(v), left(nullptr), right(nullptr) {}
};

template <typename T>
class MyBST {
private:
    BSTNode<T>* root;  //  根节点

    //  递归插入辅助函数
    BSTNode<T>* insertHelper(BSTNode<T>* node, const T& val) {
        if (!node) return new BSTNode<T>(val);  //  找到空位了，新建节点
        if (val < node->val) {
            node->left = insertHelper(node->left, val);   //  小了往左
        } else if (val > node->val) {
            node->right = insertHelper(node->right, val); //  大了往右
        }
        //  相等的话不插入（BST一般不允许重复值）
        return node;
    }

    //  递归查找辅助函数
    bool findHelper(BSTNode<T>* node, const T& val) const {
        if (!node) return false;          //  空了，没找到
        if (val == node->val) return true; //  找到了
        if (val < node->val) return findHelper(node->left, val);
        return findHelper(node->right, val);
    }

    //  找最小值节点（用于删除操作）
    BSTNode<T>* findMin(BSTNode<T>* node) const {
        while (node && node->left) {
            node = node->left;  //  一直往左走，最左边的就是最小的
        }
        return node;
    }

    //  递归删除辅助函数
    BSTNode<T>* removeHelper(BSTNode<T>* node, const T& val) {
        if (!node) return nullptr;  //  没找到

        if (val < node->val) {
            node->left = removeHelper(node->left, val);
        } else if (val > node->val) {
            node->right = removeHelper(node->right, val);
        } else {
            //  找到了！开始删
            //  情况1：叶子节点 或 只有一个孩子
            if (!node->left) {
                BSTNode<T>* temp = node->right;
                delete node;
                return temp;
            }
            if (!node->right) {
                BSTNode<T>* temp = node->left;
                delete node;
                return temp;
            }
            //  情况2：两个孩子都有
            //  找右子树的最小值来顶替
            BSTNode<T>* minNode = findMin(node->right);
            node->val = minNode->val;
            //  然后把那个最小值节点删掉
            node->right = removeHelper(node->right, minNode->val);
        }
        return node;
    }

    //  前序遍历：根左右
    void preorderHelper(BSTNode<T>* node, vector<T>& result) const {
        if (!node) return;
        result.push_back(node->val);          //  根
        preorderHelper(node->left, result);   //  左
        preorderHelper(node->right, result);  //  右
    }

    //  中序遍历：左根右
    void inorderHelper(BSTNode<T>* node, vector<T>& result) const {
        if (!node) return;
        inorderHelper(node->left, result);    //  左
        result.push_back(node->val);          //  根
        inorderHelper(node->right, result);   //  右
    }

    //  后序遍历：左右根
    void postorderHelper(BSTNode<T>* node, vector<T>& result) const {
        if (!node) return;
        postorderHelper(node->left, result);   //  左
        postorderHelper(node->right, result);  //  右
        result.push_back(node->val);           //  根
    }

    //  销毁整棵树（后序）
    void destroyTree(BSTNode<T>* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

public:
    MyBST() : root(nullptr) {}

    ~MyBST() {
        destroyTree(root);
    }

    //  禁用拷贝
    MyBST(const MyBST&) = delete;
    MyBST& operator=(const MyBST&) = delete;

    //  插入
    void insert(const T& val) {
        root = insertHelper(root, val);
    }

    //  查找
    bool find(const T& val) const {
        return findHelper(root, val);
    }

    //  删除
    void remove(const T& val) {
        root = removeHelper(root, val);
    }

    //  前序遍历
    vector<T> preorder() const {
        vector<T> result;
        preorderHelper(root, result);
        return result;
    }

    //  中序遍历（BST的中序是有序的！）
    vector<T> inorder() const {
        vector<T> result;
        inorderHelper(root, result);
        return result;
    }

    //  后序遍历
    vector<T> postorder() const {
        vector<T> result;
        postorderHelper(root, result);
        return result;
    }

    //  层序遍历（广度优先），用队列实现
    vector<T> levelOrder() const {
        vector<T> result;
        if (!root) return result;

        queue<BSTNode<T>*> q;
        q.push(root);

        while (!q.empty()) {
            BSTNode<T>* node = q.front();
            q.pop();
            result.push_back(node->val);
            //  左右孩子入队
            if (node->left) q.push(node->left);
            if (node->right) q.push(node->right);
        }
        return result;
    }
};

// ============================================================================
//  【六】哈希表（Hash Table）
// ============================================================================
//
//  【原理说明】
//  哈希表也叫散列表，是一种"键值对"存储结构。
//  它的核心思想是：用一个哈希函数把key转换成数组下标，
//  然后直接用下标访问——这样查找、插入、删除理论上都是O(1)！
//
//  但是有个问题：不同的key可能算出同一个下标，这叫"哈希冲突"。
//  解决冲突的方法有很多，咱们实现最常用的一种——链地址法：
//  数组的每个位置存一个链表，冲突的元素都挂在同一个链表上。
//
//  【核心概念】
//  - 哈希函数：把key映射成数组下标的函数。好的哈希函数要分布均匀。
//  - 装填因子（load factor）：元素个数 / 表长。这个值越大，冲突越多。
//    一般超过0.75就要扩容了（扩容后重新哈希，叫rehash）。
//
//  【复杂度分析】
//  - 平均：插入/删除/查找都是 O(1)
//  - 最坏：O(n) （所有元素都冲突，退化成链表）
//  - 空间：O(n)
//
//  老师傅说：哈希表可是神器！用好了能把很多O(n)的操作变成O(1)。
//  C++的std::unordered_map和std::unordered_set底层就是哈希表。
//  但要注意，哈希表是无序的，而且有一定的空间开销。
//
// ============================================================================

template <typename K, typename V>
class MyHashTable {
private:
    //  每个链表节点存键值对
    struct HashNode {
        K key;
        V value;
        HashNode(const K& k, const V& v) : key(k), value(v) {}
    };

    vector<vector<HashNode>> table;  //  数组+链表（链地址法）
    int count;                       //  元素个数
    double loadFactorThreshold;      //  装填因子阈值

    //  哈希函数：把key转成下标
    //  这里用简单的取模法，实际工程中哈希函数要复杂得多
    int hash(const K& key) const {
        //  C++的std::hash可以给很多类型算哈希值
        std::hash<K> hasher;
        return static_cast<int>(hasher(key) % table.size());
    }

    //  扩容+重新哈希
    void rehash() {
        int newSize = table.size() * 2;  //  扩容到2倍
        vector<vector<HashNode>> newTable(newSize);

        //  把老表里的元素重新哈希到新表
        std::hash<K> hasher;
        for (auto& bucket : table) {
            for (auto& node : bucket) {
                int idx = hasher(node.key) % newSize;
                newTable[idx].push_back(HashNode(node.key, node.value));
            }
        }
        //  换表
        table.swap(newTable);
    }

public:
    //  构造函数：初始大小设为16，装填因子阈值0.75
    MyHashTable() : count(0), loadFactorThreshold(0.75) {
        table.resize(16);
    }

    //  插入或更新
    void put(const K& key, const V& value) {
        int idx = hash(key);

        //  先看看key是不是已经存在了，存在就更新
        for (auto& node : table[idx]) {
            if (node.key == key) {
                node.value = value;
                return;
            }
        }

        //  不存在，新增
        table[idx].emplace_back(key, value);
        count++;

        //  检查装填因子，超了就扩容
        double loadFactor = static_cast<double>(count) / table.size();
        if (loadFactor > loadFactorThreshold) {
            rehash();
        }
    }

    //  查找：找到返回value，找不到抛异常
    V get(const K& key) const {
        int idx = hash(key);
        for (auto& node : table[idx]) {
            if (node.key == key) {
                return node.value;
            }
        }
        throw out_of_range("Key不存在！");
    }

    //  检查key是否存在
    bool contains(const K& key) const {
        int idx = hash(key);
        for (auto& node : table[idx]) {
            if (node.key == key) return true;
        }
        return false;
    }

    //  删除
    bool remove(const K& key) {
        int idx = hash(key);
        auto& bucket = table[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                count--;
                return true;
            }
        }
        return false;
    }

    //  元素个数
    int size() const { return count; }

    //  表的大小（桶的数量）
    int bucketCount() const { return table.size(); }

    //  打印看看分布情况
    void printStats() const {
        cout << "哈希表状态：元素" << count << "个，桶" << table.size() << "个" << endl;
        int maxBucketSize = 0;
        int usedBuckets = 0;
        for (auto& bucket : table) {
            if (!bucket.empty()) usedBuckets++;
            if (bucket.size() > maxBucketSize) maxBucketSize = bucket.size();
        }
        cout << "已使用桶：" << usedBuckets << "，最大桶深：" << maxBucketSize << endl;
        cout << "装填因子：" << static_cast<double>(count) / table.size() << endl;
    }
};

// ============================================================================
//  【七】堆（Heap）
// ============================================================================
//
//  【原理说明】
//  堆是一种特殊的完全二叉树，分为大顶堆和小顶堆：
//  - 大顶堆：每个节点的值 >= 它的孩子节点，堆顶是最大值
//  - 小顶堆：每个节点的值 <= 它的孩子节点，堆顶是最小值
//
//  虽然堆是二叉树的结构，但实际存储用的是数组！
//  因为完全二叉树可以很方便地用数组下标来表示父子关系：
//  - 父节点 i 的左孩子：2*i + 1
//  - 父节点 i 的右孩子：2*i + 2
//  - 子节点 i 的父节点：(i-1) / 2
//
//  【核心操作】
//  - 插入：加到数组末尾，然后"上浮"（和父节点比，大了就往上换） O(log n)
//  - 删除堆顶：把最后一个元素放到堆顶，然后"下沉"（和孩子比，小了就往下换） O(log n)
//  - 建堆：从最后一个非叶子节点开始，从后往前做下沉操作        O(n)
//  - 堆排序：不断把堆顶和末尾交换，然后调整                    O(n log n)
//
//  【复杂度分析】
//  - 插入：O(log n)
//  - 删除堆顶：O(log n)
//  - 建堆：O(n)
//  - 空间：O(n)
//
//  老师傅说：堆最经典的应用就是"优先级队列"和"Top K问题"。
//  比如找100万个数里最大的10个，用个大小为10的小顶堆就搞定了。
//  C++的std::priority_queue底层就是堆哦。
//
// ============================================================================

class MyMaxHeap {
private:
    vector<int> data;  //  底层用数组存储

    //  上浮操作：index位置的元素往上浮
    void siftUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;  //  父节点下标
            if (data[index] > data[parent]) {
                //  比父节点大，交换
                swap(data[index], data[parent]);
                index = parent;  //  继续往上看
            } else {
                break;  //  不比父节点大了，就停
            }
        }
    }

    //  下沉操作：index位置的元素往下沉
    void siftDown(int index) {
        int n = data.size();
        while (true) {
            int left = 2 * index + 1;   //  左孩子
            int right = 2 * index + 2;  //  右孩子
            int largest = index;        //  假设当前最大

            //  找三个里面最大的
            if (left < n && data[left] > data[largest]) largest = left;
            if (right < n && data[right] > data[largest]) largest = right;

            if (largest != index) {
                //  最大的不是自己，和最大的孩子交换
                swap(data[index], data[largest]);
                index = largest;  //  继续往下看
            } else {
                break;  //  自己就是最大的，停
            }
        }
    }

public:
    MyMaxHeap() = default;

    //  用已有数组建堆（heapify）
    explicit MyMaxHeap(const vector<int>& arr) : data(arr) {
        //  从最后一个非叶子节点开始，从后往前下沉
        int n = data.size();
        for (int i = n / 2 - 1; i >= 0; i--) {
            siftDown(i);
        }
    }

    //  插入元素
    void push(int val) {
        data.push_back(val);       //  加到末尾
        siftUp(data.size() - 1);   //  上浮
    }

    //  删除堆顶（最大值）
    int pop() {
        if (data.empty()) {
            throw out_of_range("堆是空的！");
        }
        int maxVal = data[0];          //  堆顶就是最大值
        data[0] = data.back();         //  最后一个元素放堆顶
        data.pop_back();               //  删掉最后一个
        if (!data.empty()) siftDown(0); //  下沉调整
        return maxVal;
    }

    //  看堆顶
    int top() const {
        if (data.empty()) {
            throw out_of_range("堆是空的！");
        }
        return data[0];
    }

    //  判空
    bool empty() const {
        return data.empty();
    }

    //  大小
    int size() const {
        return data.size();
    }

    //  打印
    void print() const {
        cout << "堆：[";
        for (size_t i = 0; i < data.size(); i++) {
            if (i > 0) cout << ", ";
            cout << data[i];
        }
        cout << "]" << endl;
    }
};

// ============================================================================
//  测试函数：挨个演示各个数据结构
// ============================================================================

void testVector() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【一】动态数组（Vector）演示                    ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    MyVector<int> vec;
    cout << "  初始状态：size=" << vec.getSize() << ", cap=" << vec.getCapacity() << endl;

    cout << "\n  push_back 5个元素：" << endl;
    for (int i = 1; i <= 5; i++) {
        vec.push_back(i * 10);
        vec.print("    第" + to_string(i) + "次");
    }

    cout << "\n  随机访问 vec[2] = " << vec[2] << endl;
    vec[2] = 999;
    cout << "  修改后 vec[2] = " << vec[2] << endl;
    vec.print("    修改后");

    cout << "\n  pop_back 两次：" << endl;
    vec.pop_back();
    vec.print("    第1次pop");
    vec.pop_back();
    vec.print("    第2次pop");
}

void testLinkedList() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【二】单链表（Linked List）演示                 ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    MyLinkedList<int> list;

    cout << "  头插法插入 30, 20, 10：" << endl;
    list.push_front(30);
    list.push_front(20);
    list.push_front(10);
    list.print("    链表");

    cout << "\n  尾插法插入 40, 50：" << endl;
    list.push_back(40);
    list.push_back(50);
    list.print("    链表");

    cout << "\n  查找30：" << (list.find(30) ? "找到了" : "没找到") << endl;
    cout << "  查找99：" << (list.find(99) ? "找到了" : "没找到") << endl;

    cout << "\n  删除30：" << (list.remove(30) ? "成功" : "失败") << endl;
    list.print("    删除后");

    cout << "\n  反转链表：" << endl;
    list.reverse();
    list.print("    反转后");
}

void testStack() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【三】栈（Stack）演示                           ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    MyStack<int> stk;
    cout << "  压入 10, 20, 30：" << endl;
    stk.push(10);
    stk.push(20);
    stk.push(30);
    cout << "    栈顶：" << stk.top() << "，大小：" << stk.size() << endl;

    cout << "\n  弹出两个：" << endl;
    stk.pop();
    cout << "    弹一个后栈顶：" << stk.top() << endl;
    stk.pop();
    cout << "    弹两个后栈顶：" << stk.top() << endl;

    cout << "\n  栈空吗？" << (stk.empty() ? "空了" : "还没空") << endl;
    stk.pop();
    cout << "  再弹一个后空吗？" << (stk.empty() ? "空了" : "还没空") << endl;

    cout << "\n  ★ 小应用：括号匹配检测" << endl;
    string expr = "((a+b)*(c-d))";
    MyStack<char> parenStack;
    bool valid = true;
    for (char c : expr) {
        if (c == '(') parenStack.push(c);
        else if (c == ')') {
            if (parenStack.empty()) { valid = false; break; }
            parenStack.pop();
        }
    }
    if (valid && parenStack.empty()) {
        cout << "    \"" << expr << "\" 括号匹配正确！" << endl;
    } else {
        cout << "    \"" << expr << "\" 括号不匹配！" << endl;
    }
}

void testQueue() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【四】循环队列（Circular Queue）演示            ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    MyCircularQueue<int> q(5);  //  容量5，实际能存5个
    cout << "  创建容量为5的循环队列" << endl;
    cout << "  空吗？" << (q.isEmpty() ? "是" : "否") << endl;

    cout << "\n  入队 10, 20, 30, 40, 50：" << endl;
    for (int i = 1; i <= 5; i++) {
        q.enqueue(i * 10);
        cout << "    入队" << i*10 << "后，队头=" << q.Front() << "，队尾=" << q.Rear()
             << "，大小=" << q.size() << "，满？" << (q.isFull()?"是":"否") << endl;
    }

    cout << "\n  满了，再入队60试试：" << (q.enqueue(60) ? "成功" : "失败（满了）") << endl;

    cout << "\n  出队两个：" << endl;
    q.dequeue();
    cout << "    出队一个后，队头=" << q.Front() << "，大小=" << q.size() << endl;
    q.dequeue();
    cout << "    出队两个后，队头=" << q.Front() << "，大小=" << q.size() << endl;

    cout << "\n  再入队 60, 70（看看循环利用空间）：" << endl;
    q.enqueue(60);
    q.enqueue(70);
    cout << "    队头=" << q.Front() << "，队尾=" << q.Rear()
         << "，大小=" << q.size() << "，满？" << (q.isFull()?"是":"否") << endl;
}

void testBST() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【五】二叉搜索树（BST）演示                     ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    MyBST<int> bst;
    cout << "  插入 50, 30, 70, 20, 40, 60, 80：" << endl;
    vector<int> vals = {50, 30, 70, 20, 40, 60, 80};
    for (int v : vals) bst.insert(v);

    auto printVec = [](const string& name, const vector<int>& v) {
        cout << "    " << name << ": ";
        for (int x : v) cout << x << " ";
        cout << endl;
    };

    printVec("前序遍历", bst.preorder());
    printVec("中序遍历", bst.inorder());
    printVec("后序遍历", bst.postorder());
    printVec("层序遍历", bst.levelOrder());

    cout << "\n  查找 40：" << (bst.find(40) ? "存在" : "不存在") << endl;
    cout << "  查找 99：" << (bst.find(99) ? "存在" : "不存在") << endl;

    cout << "\n  删除叶子节点 20：" << endl;
    bst.remove(20);
    printVec("中序遍历", bst.inorder());

    cout << "\n  删除有一个孩子的节点 30：" << endl;
    bst.remove(30);
    printVec("中序遍历", bst.inorder());

    cout << "\n  删除有两个孩子的节点 50：" << endl;
    bst.remove(50);
    printVec("中序遍历", bst.inorder());
}

void testHashTable() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【六】哈希表（Hash Table）演示                  ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    MyHashTable<string, int> scores;
    cout << "  插入学生成绩：" << endl;
    scores.put("张三", 95);
    scores.put("李四", 88);
    scores.put("王五", 76);
    scores.put("赵六", 92);
    cout << "    当前元素个数：" << scores.size() << endl;
    scores.printStats();

    cout << "\n  查找张三的成绩：" << scores.get("张三") << endl;
    cout << "  查找李四的成绩：" << scores.get("李四") << endl;
    cout << "  有叫\"孙七\"的吗？" << (scores.contains("孙七") ? "有" : "没有") << endl;

    cout << "\n  修改李四的成绩为99：" << endl;
    scores.put("李四", 99);
    cout << "    修改后李四的成绩：" << scores.get("李四") << endl;
    cout << "    元素个数：" << scores.size() << "（不变，因为是更新）" << endl;

    cout << "\n  删除王五：" << (scores.remove("王五") ? "成功" : "失败") << endl;
    cout << "    删除后元素个数：" << scores.size() << endl;
    cout << "    还有王五吗？" << (scores.contains("王五") ? "有" : "没有") << endl;

    cout << "\n  多插入一些数据，看看扩容：" << endl;
    for (int i = 0; i < 20; i++) {
        scores.put("学生" + to_string(i), 60 + i);
    }
    scores.printStats();
}

void testHeap() {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              【七】堆（Heap）演示                            ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    cout << "  方式一：逐个插入建堆" << endl;
    MyMaxHeap heap1;
    vector<int> nums = {5, 3, 8, 1, 10, 7, 2};
    cout << "    插入：";
    for (int x : nums) {
        cout << x << " ";
        heap1.push(x);
    }
    cout << endl;
    heap1.print();
    cout << "    堆顶最大值：" << heap1.top() << endl;

    cout << "\n  方式二：用已有数组建堆（heapify，O(n)）" << endl;
    MyMaxHeap heap2(nums);
    heap2.print();

    cout << "\n  堆排序（依次弹出堆顶）：" << endl;
    cout << "    排序结果：";
    while (!heap2.empty()) {
        cout << heap2.pop() << " ";
    }
    cout << "（从大到小）" << endl;

    cout << "\n  ★ 小应用：Top K 问题" << endl;
    cout << "    在数组 [5,3,8,1,10,7,2,15,4,12] 中找最大的3个：" << endl;
    vector<int> bigNums = {5, 3, 8, 1, 10, 7, 2, 15, 4, 12};
    //  正确姿势：维护一个大小为 K 的“小顶堆”。
    //  堆顶是当前 K 个候选里最小的；新元素比堆顶大就替换它。
    //  时间 O(n log K)，额外空间 O(K)，不需要把整个数组建成堆。
    const int K = 3;
    priority_queue<int, vector<int>, greater<int>> minHeap;
    for (int x : bigNums) {
        if (static_cast<int>(minHeap.size()) < K) {
            minHeap.push(x);
        } else if (x > minHeap.top()) {
            minHeap.pop();
            minHeap.push(x);
        }
    }
    cout << "    最大的3个（从小到大）：";
    while (!minHeap.empty()) {
        cout << minHeap.top() << " ";
        minHeap.pop();
    }
    cout << endl;
    cout << "    说明：用大小为 K 的小顶堆，时间复杂度 O(n log K)，" << endl;
    cout << "         比“全部建堆再弹 K 次”的 O(n + K log n) 更适合海量数据。" << endl;
}

// ============================================================================
//  老师傅的话：数据结构怎么选？
// ============================================================================
//
//  徒儿啊，七个数据结构都讲完了，是不是信息量有点大？
//  别急，为师给你梳理梳理，帮你建立个知识体系。
//
//  ┌─────────────────────────────────────────────────────────────┐
//  │  一、线性结构（一对一）                                      │
//  ├─────────────────────────────────────────────────────────────┤
//  │                                                             │
//  │  【动态数组 Vector】                                         │
//  │  - 优点：随机访问O(1)，尾部操作快，缓存友好                  │
//  │  - 缺点：中间插删O(n)，扩容有开销                            │
//  │  - 适用：大多数场景，尤其是知道下标或只在尾部操作时          │
//  │  - 对应STL：std::vector                                     │
//  │                                                             │
//  │  【链表 Linked List】                                       │
//  │  - 优点：已知位置的插删O(1)，不需要连续内存                  │
//  │  - 缺点：不能随机访问，查找O(n)，每个节点有指针开销          │
//  │  - 适用：频繁在中间插删，且位置已知的场景                    │
//  │  - 对应STL：std::list（双向链表）, std::forward_list        │
//  │                                                             │
//  │  【栈 Stack】                                               │
//  │  - 特点：后进先出（LIFO）                                   │
//  │  - 适用：函数调用、括号匹配、表达式求值、DFS、撤销操作      │
//  │  - 对应STL：std::stack                                      │
//  │                                                             │
//  │  【队列 Queue】                                             │
//  │  - 特点：先进先出（FIFO）                                   │
//  │  - 适用：任务调度、消息队列、BFS、缓冲区                    │
//  │  - 对应STL：std::queue                                      │
//  │                                                             │
//  └─────────────────────────────────────────────────────────────┘
//
//  ┌─────────────────────────────────────────────────────────────┐
//  │  二、树形结构（一对多）                                      │
//  ├─────────────────────────────────────────────────────────────┤
//  │                                                             │
//  │  【二叉搜索树 BST】                                         │
//  │  - 优点：平均O(log n)的查找插入删除，中序遍历有序            │
//  │  - 缺点：最坏O(n)，不平衡的时候就废了                        │
//  │  - 进阶：AVL树、红黑树（自平衡BST）                          │
//  │  - 对应STL：std::map, std::set（底层是红黑树）              │
//  │                                                             │
//  │  【堆 Heap】                                                │
//  │  - 优点：取堆顶 O(1)（大顶堆取最大/小顶堆取最小），插入删除 O(log n) │
//  │  - 缺点：不能查找任意元素，只能管堆顶                       │
//  │  - 适用：优先级队列、Top K问题、堆排序                      │
//  │  - 对应STL：std::priority_queue                             │
//  │                                                             │
//  └─────────────────────────────────────────────────────────────┘
//
//  ┌─────────────────────────────────────────────────────────────┐
//  │  三、哈希结构                                                │
//  ├─────────────────────────────────────────────────────────────┤
//  │                                                             │
//  │  【哈希表 Hash Table】                                      │
//  │  - 优点：平均O(1)的查找插入删除，无敌快！                    │
//  │  - 缺点：无序，有空间开销，最坏O(n)（概率极低）              │
//  │  - 适用：绝大多数需要快速查找的场景                          │
//  │  - 对应STL：std::unordered_map, std::unordered_set          │
//  │                                                             │
//  └─────────────────────────────────────────────────────────────┘
//
//  ★ 选择经验 ★
//  1. 不知道用啥？先用数组（vector），大多数场景都够了
//  2. 需要快速查找？哈希表（unordered_map）先考虑
//  3. 需要有序？红黑树（map/set）
//  4. 先进先出？队列；后进先出？栈
//  5. 找最大/最小？堆
//
//  记住：没有最好的数据结构，只有最合适的数据结构！
//  要根据具体场景的操作类型和频率来选择。
//
// ============================================================================

// ============================================================================
//  小练习
// ============================================================================
//
//  徒儿啊，数据结构光看不练可不行，动手写写才记得牢。
//  来做几道题练练手：
//
//  【基础题】
//  1. 在MyVector的基础上，实现 insert(pos, val) 在指定位置插入元素。
//  2. 用栈实现一个"最小栈"：push、pop、top、getMin都是O(1)。
//     提示：用两个栈，一个存数据，一个存当前最小值。
//  3. 用两个栈实现一个队列。
//     提示：一个栈负责入队，一个栈负责出队，出队时把入队栈倒过去。
//
//  【进阶题】
//  1. 实现双向链表：每个节点有prev和next两个指针。
//     头插、尾插、删除都要O(1)（假设已经知道节点位置）。
//  2. BST的层序遍历是从上到下一层一层的，
//     那你能实现"之字形遍历"吗？第一层从左到右，第二层从右到左……
//     提示：用两个栈或者用deque。
//  3. 给一个字符串，找出第一个只出现一次的字符。要求O(n)时间。
//     提示：用哈希表统计每个字符出现的次数。
//
//  【挑战题】
//  1. 实现一个LRU缓存（最近最少使用淘汰策略）。
//     要求：get和put都是O(1)。
//     提示：哈希表 + 双向链表。哈希表存key到链表节点的映射，
//     链表按访问顺序排列，最近访问的在前面。
//  2. 实现一个小顶堆（Min Heap），并实现"堆排序"从小到大输出。
//  3. 用非递归的方式实现BST的前中后序遍历。
//     提示：用栈模拟递归过程。
//
// ============================================================================

// ============================================================================
//  主函数
// ============================================================================
int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║        老 师 傅 带 徒 弟  -  经 典 数 据 结 构              ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n";

    //  挨个演示
    testVector();
    testLinkedList();
    testStack();
    testQueue();
    testBST();
    testHashTable();
    testHeap();

    //  老师傅的话
    cout << "\n";
    cout << "╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║                  老 师 傅 的 话                              ║\n";
    cout << "╠══════════════════════════════════════════════════════════════╣\n";
    cout << "║  徒儿，数据结构是编程的内功，一定要扎实。                    ║\n";
    cout << "║  这七个只是最基础的，后面还有图、Trie树、并查集、            ║\n";
    cout << "║  线段树、树状数组……路还长着呢。                             ║\n";
    cout << "║                                                              ║\n";
    cout << "║  学习数据结构最好的方法就是——                                ║\n";
    cout << "║  第一，自己动手写一遍实现；                                  ║\n";
    cout << "║  第二，刷LeetCode，在实战中体会每种结构的优劣。              ║\n";
    cout << "║                                                              ║\n";
    cout << "║  当然了，实际工作中还是老老实实用STL，别自己造轮子。         ║\n";
    cout << "║  但原理得懂，出了问题才知道怎么调优。                        ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n";
    cout << "\n";

    return 0;
}

