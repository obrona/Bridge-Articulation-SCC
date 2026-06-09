#include <bits/stdc++.h>
#include "treap.hpp"

template <typename T, typename Compare = std::less<T>>
void validate(TreapNode<T, Compare>* t) {
    if (t == nullptr) return;

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

    dfs(t);

    for (int i = 0; i < inorder.size() - 1; i++) {
        assert(cmp(inorder[i], inorder[i + 1]));
    }
}

template <typename T, typename Compare = std::less<T>>
void validate(TreapNode<T>* t, const vector<T>& expected) {
    if (t == nullptr) return;

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

    dfs(t);

    assert(inorder == expected);
}

vector<int> random_nums(int len) {
    vector<int> out(len);
    iota(out.begin(), out.end(), 0);
    shuffle(out.begin(), out.end(), mt19937{});
    return out;
}

void test1() {
    TreapNode<int>* root = nullptr;
    for(int i = 0; i < 100000; i++) insert(root, 0);
    assert(root->cnt == 100000);
}

void test2() {
    auto nums = random_nums(100000);
    vector<int> expected(100000);
    iota(expected.begin(), expected.end(), 0);
    
    TreapNode<int>* root = nullptr;
    for (int x : nums) insert(root, x);
    validate(root, expected);
}
int main() {
    test1();
    test2();
}

