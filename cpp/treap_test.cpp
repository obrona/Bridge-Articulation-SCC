#include <bits/stdc++.h>
#include "treap.hpp"

template <typename T, typename Compare = std::less<T>>
void validate(TreapNode<T, Compare>& t) {
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
    if (treap.root == nullptr) return;

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

