#include<iostream>
#include <vector>
using namespace std;
class Node {
public:
	int data;
	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;

	explicit Node(int x) {
		data = x;
		left = nullptr;
		right = nullptr;
	}
};
void inOrder(const Node* node, vector<int>& res) {
	if (node == nullptr)
		return;
	inOrder(node->left.get(), res);
	res.push_back(node->data);
	inOrder(node->right.get(), res);
}
int main() {
	auto root = std::make_unique<Node>(1);
	root->left = std::make_unique<Node>(2);
	root->right = std::make_unique<Node>(3);
	root->left->left = std::make_unique<Node>(4);
	root->left->right = std::make_unique<Node>(5);
	root->right->right = std::make_unique<Node>(6);

	vector<int> res;
	inOrder(root.get(), res);

	for (auto node : res)
		cout << node << " ";

	return 0;
}