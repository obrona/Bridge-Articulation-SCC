#include <bits/stdc++.h>
#include "treap.hpp"

template <typename T, typename Compare = std::less<T>>
void validate(TreapNode<T> *root) {
    if (root == nullptr) return;

    Compare cmp;
    vector<T> inorder;

    function<void(TreapNode<T>*)> dfs = [&] (auto t) {
        if (t->l != nullptr) {
            assert(t->l->prior <= t->prior);
            dfs(t->l);
        }
        inorder.push_back(t->val);
        if (t->r != nullptr) {
            assert(t->r->prior <= t->prior);
            dfs(t->r);
        }
    };

    dfs(root);

    for (int i = 0; i < inorder.size() - 1; i++) {
        assert(cmp(inorder[i], inorder[i + 1]));
    }
}

void test1() {
    Treap<int> treap;
    for (int i = 0; i < 100000; i++) treap.insert(1);
    assert(treap.root->cnt == 100000);
}

void test2() {
    vector<int> nums(100000);
    iota(nums.begin(), nums.end(), 0);
    shuffle(nums.begin(), nums.end(), mt19937());

    Treap<int> treap;
    for (int x : nums) treap.insert(x);
    validate<int>(treap.root);

}

int main() {
    test1();
    test2();
}
