#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;


struct HuffmanNode {
    unsigned char byte;
    unsigned long long frequency;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(unsigned char b, unsigned long long freq)
        : byte(b), frequency(freq), left(nullptr), right(nullptr) {
    }

    HuffmanNode(unsigned long long freq, HuffmanNode* l, HuffmanNode* r)
        : byte(0), frequency(freq), left(l), right(r) {
    }


    bool operator>(const HuffmanNode& other) const {
        return frequency > other.frequency;
    }

    bool isLeaf() const {
        return !left && !right;
    }
};

class MinHeap {
private:
    vector<HuffmanNode*> heap;

    int parent(int i) { return (i - 1) / 2; }
    int leftChild(int i) { return 2 * i + 1; }
    int rightChild(int i) { return 2 * i + 2; }

    void heapifyUp(int i) {
        while (i > 0 && heap[parent(i)]->frequency > heap[i]->frequency) {
            swap(heap[i], heap[parent(i)]);
            i = parent(i);
        }
    }

    void heapifyDown(int i) {
        int smallest = i;
        int left = leftChild(i);
        int right = rightChild(i);

        if (left < heap.size() && heap[left]->frequency < heap[smallest]->frequency) {
            smallest = left;
        }

        if (right < heap.size() && heap[right]->frequency < heap[smallest]->frequency) {
            smallest = right;
        }

        if (smallest != i) {
            swap(heap[i], heap[smallest]);
            heapifyDown(smallest);
        }
    }

public:
    MinHeap() = default;

    MinHeap(const vector<HuffmanNode*>& nodes) {
        heap = nodes;
        for (int i = (heap.size() / 2) - 1; i >= 0; --i)  heapifyDown(i);
    }

    ~MinHeap() {
        clear();
    }

    int size() const {
        return heap.size();
    }


    bool empty() const {
        return heap.empty();
    }

    HuffmanNode* peek() const {
        if (empty()) return nullptr;
        return heap[0];
    }

    HuffmanNode* extractMin() {
        if (empty())  return nullptr;
        HuffmanNode* root = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        heapifyDown(0);
        return root;
    }

    void insert(HuffmanNode* node) {
        heap.push_back(node);
        heapifyUp(heap.size() - 1);
    }

    void buildHeap(const vector<HuffmanNode*>& nodes) {
        heap = nodes;
        for (int i = (heap.size() / 2) - 1; i >= 0; --i) 
            heapifyDown(i);
    }

    void clear() {
        heap.clear();
    }

    void printHeap() const {
        cout << "Мин-куча [" << heap.size() << " элементов]: ";
        for (const auto& node : heap) {
            cout << node->frequency << " ";
        }
        cout << endl;
    }
};


void buildHuffmanCodes(HuffmanNode* node, string code,
    unordered_map<unsigned char, string>& codes,
    unordered_map<unsigned char, int>& codeLengths) {
    if (!node) return;

    if (node->isLeaf()) {
        codes[node->byte] = code;
        codeLengths[node->byte] = code.length();
        return;
    }

    buildHuffmanCodes(node->left, code + "0", codes, codeLengths);
    buildHuffmanCodes(node->right, code + "1", codes, codeLengths);
}

HuffmanNode* buildHuffmanTree(MinHeap& minHeap) {
    if (minHeap.empty())  return nullptr;
    
    if (minHeap.size() == 1) {
        HuffmanNode* onlyNode = minHeap.extractMin();
        HuffmanNode* parent = new HuffmanNode(onlyNode->frequency, onlyNode, nullptr);
        return parent;
    }

    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.extractMin();
        HuffmanNode* right = minHeap.extractMin();
        unsigned long long sumFreq = left->frequency + right->frequency;
        HuffmanNode* parent = new HuffmanNode(sumFreq, left, right);
        minHeap.insert(parent);
    }

    return minHeap.extractMin();
}

void deleteTree(HuffmanNode* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

bool readFileAndCountFrequencies(const string& filename,
    vector<unsigned long long>& frequencies,
    unsigned long long& totalBytes) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        cerr << "Ошибка: не удалось открыть файл " << filename << endl;
        return false;
    }


    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);
    unsigned char buffer[4096];
    totalBytes = 0;
    frequencies.assign(256, 0);

    while (file) {
        file.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
        streamsize bytesRead = file.gcount();

        for (streamsize i = 0; i < bytesRead; ++i) {
            frequencies[buffer[i]]++;
            totalBytes++;
        }
    }

    file.close();
    return true;
}

void printHuffmanTable(const vector<unsigned long long>& frequencies,
    const unordered_map<unsigned char, string>& codes,
    const unordered_map<unsigned char, int>& codeLengths,
    unsigned long long totalBytes) {
    
    cout << left << setw(13) << "Байт" 
         << setw(12) << "Частота" 
         << setw(20) << "Вероятность" 
         << setw(20) << "Код Хаффмана" 
         << setw(10) << "Длина" << endl;

    vector<pair<unsigned long long, unsigned char>> sortedBytes;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            sortedBytes.push_back({ frequencies[i], static_cast<unsigned char>(i) });
        }
    }

    sort(sortedBytes.rbegin(), sortedBytes.rend());

    int count = 0;
    for (const auto& pair : sortedBytes) {
        if (count++ >= 20) break;

        unsigned long long freq = pair.first;
        unsigned char byte = pair.second;
        string code = codes.at(byte);
        int length = codeLengths.at(byte);
        double probability = static_cast<double>(freq) / totalBytes;

        cout << "0x" << hex << setw(2) << setfill('0') 
             << static_cast<int>(byte) << dec << setfill(' ');
        cout << " (";
        if (byte >= 32 && byte <= 126) {
            cout << static_cast<char>(byte);
        } else cout << '.';
        cout << ")  ";
        cout << setw(11) << right << freq;
        cout << setw(16) << right << fixed << setprecision(6) << probability;
        cout << setw(20) << right << code;
        cout << setw(10) << right << length << endl;
    }
    
}
void printCompressionStats(unsigned long long totalBytes,
    unsigned long long totalBits,
    const vector<unsigned long long>& frequencies) {
    unsigned long long fixedBits = totalBytes * 8; // 8 бит на байт без сжатия
    double compressionRatio = static_cast<double>(totalBits) / fixedBits * 100;
    double avgCodeLength = static_cast<double>(totalBits) / totalBytes;

    cout << "Размер исходного файла: " << fixedBits << " бит ("
        << totalBytes << " байт)" << endl;
    cout << "Размер после сжатия Хаффмана: " << totalBits << " бит ("
        << (totalBits + 7) / 8 << " байт)" << endl;

    double entropy = 0.0;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            double p = static_cast<double>(frequencies[i]) / totalBytes;
            entropy -= p * log2(p);
        }
    }

    cout << "Энтропия Шеннона (бит/символ): " << entropy << endl;
    cout << "Средняя длина кода Хаффмана: " << avgCodeLength << " бит/символ" << endl;
    cout << "Разница: " << (avgCodeLength - entropy) << " бит/символ" << endl;

}
int main(int argc, char* argv[]) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    string filename = argv[1];
    vector<unsigned long long> frequencies;
    unsigned long long totalBytes = 0;

    if (!readFileAndCountFrequencies(filename, frequencies, totalBytes)) {
        return 1;
    }

    unsigned long long uniqueBytes = 0;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            uniqueBytes++;
        }
    }

    cout << "Размер файла: " << totalBytes << " байт" << endl;
    cout << "Уникальных байтов: " << uniqueBytes << " из 256" << endl;

    vector<HuffmanNode*> initialNodes;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            initialNodes.push_back(new HuffmanNode(static_cast<unsigned char>(i), frequencies[i]));
        }
    }

    cout << "Создано " << initialNodes.size() << " узлов для уникальных байтов" << endl;

    MinHeap minHeap(initialNodes);
    cout << "Мин-куча построена. Размер кучи: " << minHeap.size() << endl;

    HuffmanNode* minNode = minHeap.peek();
    if (minNode) {
        cout << "Минимальный элемент в куче: байт 0x" << hex
            << static_cast<int>(minNode->byte) << dec
            << ", частота = " << minNode->frequency << endl;
    }


    HuffmanNode* root = buildHuffmanTree(minHeap);
    unordered_map<unsigned char, string> huffmanCodes;
    unordered_map<unsigned char, int> codeLengths;
    buildHuffmanCodes(root, "", huffmanCodes, codeLengths);
    printHuffmanTable(frequencies, huffmanCodes, codeLengths, totalBytes);
    unsigned long long totalBits = 0;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            unsigned char byte = static_cast<unsigned char>(i);
            totalBits += frequencies[i] * codeLengths[byte];
        }
    }

    printCompressionStats(totalBytes, totalBits, frequencies);
    vector<unsigned char> exampleBytes = { 0x00, 0xFF, 'A', ' ', 0x0A };
    for (auto byte : exampleBytes) {
        if (frequencies[byte] > 0) {
            cout << "Байт 0x" << hex << setw(2) << setfill('0')
                << static_cast<int>(byte) << dec << " ('"
                << (byte >= 32 && byte <= 126 ? static_cast<char>(byte) : '.')
                << "'): " << huffmanCodes[byte] << endl;
        }
    }

    deleteTree(root);
    return 0;
}