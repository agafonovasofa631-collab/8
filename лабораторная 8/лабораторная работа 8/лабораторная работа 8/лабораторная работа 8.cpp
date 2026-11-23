#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <iomanip>
#include <fstream>
using namespace std;
using namespace chrono;


// Часть 1. Двоичный поиск (итеративный)
// Условие: массив отсортирован по возрастанию
// Сложность: O(log n)
int binarySearchIter(const vector<int>& arr, int target) {
    int left = 0;
    int right = (int)arr.size() - 1;
    while (left <= right) {
        // Защита от переполнения: left + (right - left) / 2
        int mid = left + (right - left) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1; // не найдено
}


// Часть 2. Хеш таблица с методом цепочек
// Ключ: int, Значение: int
class HashTableChaining {
private:
    struct Node {
        int key;
        int value;
        Node* next;
        Node(int k, int v) : key(k), value(v), next(nullptr) {}
    };

    vector<Node*> table;  // массив бакетов (головы списков)
    int size;             // количество бакетов
    int count;            // количество элементов

    // Хеш-функция: key % size
    int hash(int key) const {
        int h = key % size;
        return h < 0 ? h + size : h;
    }

public:
    HashTableChaining(int s = 10) : size(s), count(0) {
        table.assign(size, nullptr);
    }

    ~HashTableChaining() {
        for (int i = 0; i < size; i++) {
            Node* cur = table[i];
            while (cur) {
                Node* tmp = cur;
                cur = cur->next;
                delete tmp;
            }
        }
    }

    // Вставка / обновление
    void insert(int key, int value) {
        int idx = hash(key);
        Node* cur = table[idx];
        while (cur) {
            if (cur->key == key) {     // ключ уже есть — обновляем
                cur->value = value;
                return;
            }
            cur = cur->next;
        }
        // добавляем в начало списка
        Node* node = new Node(key, value);
        node->next = table[idx];
        table[idx] = node;
        count++;
    }

    // Поиск: возвращает true и кладёт значение в result
    bool find(int key, int& result) const {
        int idx = hash(key);
        Node* cur = table[idx];
        while (cur) {
            if (cur->key == key) {
                result = cur->value;
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    // Удаление
    bool remove(int key) {
        int idx = hash(key);
        Node* cur = table[idx];
        Node* prev = nullptr;
        while (cur) {
            if (cur->key == key) {
                if (prev) prev->next = cur->next;
                else      table[idx] = cur->next;
                delete cur;
                count--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    int getSize() const { return count; }
    int getBuckets() const { return size; }

    // Коэффициент загрузки
    double loadFactor() const { return (double)count / size; }

    void print() const {
        for (int i = 0; i < size; i++) {
            cout << i << ": ";
            Node* cur = table[i];
            while (cur) {
                cout << "(" << cur->key << ", " << cur->value << ") ";
                cur = cur->next;
            }
            cout << "\n";
        }
    }
};


// Часть 3. Сравнительный анализ

// Замер времени двоичного поиска: Q запросов по отсортированному массиву
long long timeBinarySearch(const vector<int>& arr,
    const vector<int>& queries) {
    auto start = high_resolution_clock::now();
    volatile long long sink = 0; // чтобы компилятор не выкинул вызовы
    for (int q : queries) {
        sink += binarySearchIter(arr, q);
    }
    auto end = high_resolution_clock::now();
    (void)sink;
    return duration_cast<microseconds>(end - start).count();
}

// Замер времени поиска в хеш-таблице
long long timeHashSearch(const HashTableChaining& ht,
    const vector<int>& queries) {
    auto start = high_resolution_clock::now();
    volatile long long sink = 0;
    for (int q : queries) {
        int v;
        if (ht.find(q, v)) sink += v;
    }
    auto end = high_resolution_clock::now();
    (void)sink;
    return duration_cast<microseconds>(end - start).count();
}

int main() {
    setlocale(LC_ALL, "rus");
    // Демонстрация работы 
    cout << " Демонстрация хеш-таблицы (int -> int) \n";
    HashTableChaining demo(5);
    demo.insert(10, 100);
    demo.insert(20, 200);
    demo.insert(30, 300);
    demo.insert(15, 150);
    demo.insert(25, 250);  // коллизия с 10, 15, 20, 25, 30 по mod 5
    demo.print();

    int val;
    if (demo.find(20, val))
        cout << "find(20) = " << val << "\n";
    demo.remove(20);
    cout << "После удаления 20:\n";
    demo.print();
    cout << "Коэффициент загрузки: " << demo.loadFactor() << "\n\n";

    cout << " Демонстрация двоичного поиска \n";
    vector<int> sample = { 1, 3, 5, 7, 9, 11, 13 };
    cout << "Массив: ";
    for (int x : sample) cout << x << " ";
    cout << "\nПоиск 7 -> индекс " << binarySearchIter(sample, 7) << "\n";
    cout << "Поиск 8 -> индекс " << binarySearchIter(sample, 8) << "\n\n";

    // Сравнительный анализ 
    cout << " Сравнение производительности \n";
    cout << "N = 1000, 10000, 100000; запросов: 10000\n\n";

    cout << left
        << setw(10) << "N"
        << setw(20) << "Binary, мкс"
        << setw(20) << "Hash, мкс"
        << setw(15) << "Buckets"
        << setw(15) << "LoadFactor"
        << "\n";
    cout << string(80, '-') << "\n";

  
    const int QUERIES = 10000;

    mt19937 rng(42);
    ofstream csv("results_variant1.csv");
    csv << "N,binary_us,hash_us,buckets,load_factor\n";

    for (int N : {1000, 10000, 100000}) {
        // 1) Отсортированный массив
        vector<int> arr(N);
        for (int i = 0; i < N; i++) arr[i] = i;

        // 2) Хеш-таблица: size ~ N (чтобы loadFactor ~ 1)
        //    Берём простое число, чуть больше N
        int buckets = (int)(N * 1.3);
        // Простая корректировка до простого
        auto isPrime = [](int x) {
            if (x < 2) return false;
            for (int d = 2; 1LL * d * d <= x; d++)
                if (x % d == 0) return false;
            return true;
        };
        while (!isPrime(buckets)) buckets++;

        HashTableChaining ht(buckets);
        for (int i = 0; i < N; i++) ht.insert(i, i);

        // 3) Запросы: часть — существующие ключи, часть — отсутствующие
        vector<int> queries(QUERIES);
        uniform_int_distribution<int> dist(0, N - 1);
        uniform_int_distribution<int> distMiss(N, 2 * N);
        for (int i = 0; i < QUERIES; i++) {
            queries[i] = (i % 2 == 0) ? dist(rng) : distMiss(rng);
        }

        // 4) Замеры (несколько прогонов, берём минимум для стабильности)
        long long tBin = LLONG_MAX, tHash = LLONG_MAX;
        for (int rep = 0; rep < 5; rep++) {
            tBin = min(tBin, timeBinarySearch(arr, queries));
            tHash = min(tHash, timeHashSearch(ht, queries));
        }

        cout << left
            << setw(10) << N
            << setw(20) << tBin
            << setw(20) << tHash
            << setw(15) << buckets
            << setw(15) << fixed << setprecision(3) << ht.loadFactor()
            << "\n";

        csv << N << "," << tBin << "," << tHash << ","
            << buckets << "," << ht.loadFactor() << "\n";
    }
    csv.close();

    return 0;
}