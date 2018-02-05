/**
Purpose: Compression and Decompression using Huffman Algorithm
Aaraj Habib
*/

#include <iostream>
#include <functional>
#include <queue>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
using namespace std;


unsigned char buffer;
int bitCount = 0;
queue<char> output;

struct Node
{
	char symbol;
	int freq;
	Node *left, *right;

	Node(char symbol = NULL, int freq = 0) {
		left = right = NULL;
		this->symbol = symbol;
		this->freq = freq;
	}
};

struct compare
{
	bool operator()(Node* left, Node* right) {
		return (left->freq > right->freq);
	}
};

void bitWriter(bool bit, ofstream &out) {
	if (bitCount > 7) {
		out << buffer;
		bitCount = 0;
	}
	buffer = buffer << 1;
	if (bit) {
		buffer++;
	}
	bitCount++;
}

void dec2bit(unsigned char symbol, ofstream &out)
{
	unsigned char m = 0;
	unsigned char mask = 128;
	for (int i = 0; i < 8; i++){
		m = symbol & mask;
		if (m > 0)
			bitWriter(1, out);
		else
			bitWriter(0, out);
		mask = mask >> 1;
	}
}

void bitReader(unsigned int symbol) {
	stack<char> temp;
	for (int i = 0; i < 8; ++i) {
		temp.push(symbol % 2 + '0');
		symbol >>= 1;
	}
	for (int i = 0; i < 8; i++) {
		output.push(temp.top());
		temp.pop();
	}
}
char getBit()
{
	if (!output.size())
		return 0;
	char ret = output.front();
	output.pop();
	return ret;
}
char getByte()
{
	unsigned char ret = 0;
	unsigned char temp = 0;
	for (int i = 0; i < 8; i++) {
		temp = getBit();
		ret = ret << 1;
		if (temp != '0') {
			ret++;
		}
	}
	return ret;
}

void StoreTree(Node* node,ofstream &out)
{
	if (node->left==0 && node->right==0)
	{
		bitWriter(1, out);
		dec2bit(node->symbol, out);
	}
	else{
		bitWriter(0, out);
		if(node->left!=NULL)
		StoreTree(node->left, out);
		if (node->right != NULL)
		StoreTree(node->right, out);
	}
}

void getCode(char symbol, Node *root, string &code)
{
	if (root->symbol == symbol && code.back() != 'E')
		code.push_back('E');
		
	if (root->left != NULL ) {
		code.push_back('0');

		getCode(symbol, root->left, code);
		if (code.back() == 'E') {
			return;
		}
		else
				code.pop_back();
	}
	if (root->right != NULL ) {
		code.push_back('1');
		getCode(symbol, root->right, code);
		if (code.back() == 'E')
			return;
		else code.pop_back();
	}
	return;
}

void Compression(string filename)
{
	char in;
	ifstream input(filename, ios::binary | ios::in);
	int freqTable[256] = { 0 };
	freqTable[1]++;


	while (!input.eof()) {
		input.read(&in, 1);
		freqTable[in]++;
	}

	priority_queue<Node*, vector<Node*>, compare> minHeap;
	for (int i = 0; i<256; i++){
		if (freqTable[i]) {
			minHeap.push(new Node(i, freqTable[i]));
		}
	}

	while (minHeap.size() != 1) {
		Node *left, *right, *alpha;
		left = minHeap.top();
		minHeap.pop();

		right = minHeap.top();
		minHeap.pop();

		alpha = new Node;
		alpha->freq= left->freq + right->freq;
		alpha->left = left;
		alpha->right = right;
		minHeap.push(alpha);
	}
	filename.erase(filename.end() - 3,filename.end());
	filename += "mcf";
	ofstream out(filename);
	StoreTree(minHeap.top(), out);
	while (bitCount != 8) {
		buffer = buffer << 1;
		bitCount++;
	}
	out << buffer;
	bitCount = 0;
	out << ' ';
	input.clear();              
	input.seekg(0, std::ios::beg);
	char inp;
	string code;
	while(input.get(inp))
	{
		getCode(inp, minHeap.top(), code);
		code.pop_back();
		for (int i = 0; i < code.length(); i++){
			bitWriter(code[i] -= '0', out);
		}
		code.clear();
	}
	getCode(1, minHeap.top(), code); //TO ADD A PSEUDO EOF
	while (bitCount != 8) {
		buffer = buffer << 1;
		bitCount++;
	}
	out << buffer;

}

Node* treeReader() {
	Node* root = new Node;
	char bit;
	if (output.size() != 0)
		bit = getBit();
	else
		return NULL;
	if (bit == '0') {
		root->left = treeReader();
		root->right = treeReader();
	}
	else if (bit == '1') {
		root->symbol=getByte();
		root->left = root->right = NULL;
	}
	return root;
}
char deCode(Node* root,ifstream &inp) {
	if (root->symbol != NULL) {
		return root->symbol;
	}
	char bit;
	while (output.size() == 0 && inp.get(bit)) {
		bitReader(bit);
	}
	bit = getBit();
	if (bit == '0' && root->left != NULL) {
		return deCode(root->left,inp);
	}
	else if (bit == '1' && root->right != NULL) {
		return deCode(root->right,inp);
	}
}
void Decompression(string filename) {
	ifstream input(filename, ios::binary | ios::in);
	char sym;
	
	while (input.get(sym) && sym != ' ') {
		bitReader(sym);
	}
	Node* root = treeReader();
	while (!output.empty())
		output.pop();
	filename.erase(filename.end() - 3, filename.end());
	filename += "txtss";
	ofstream out(filename);
	while (input.get(sym)) {
		bitReader(sym);
		out << deCode(root, input);
	}
	char temp;
	while (output.size() > 0) {
		temp= deCode(root, input);
		if (temp == 0)
			break;
		out << temp;
	}
}

int main()
{
	string input;
	getline(cin, input, ' ');
	while (input != "exit") {
		if (input == "compress") {
			getline(cin, input);
			Compression(input);
			cout << "Compressed successfully." << endl;
		}

		else if (input == "decompress") {
			getline(cin, input);
			Decompression(input);
			cout << "Decompressed Successfully" << endl;
		}
		getline(cin, input, ' ');
	}
}
