#include <bits/stdc++.h>
#include <ranges>
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

void test_simple_insert() {
    TreapNode<int>* root = nullptr;
    for(int i = 0; i < 100000; i++) insert(root, 0);
    assert(root->cnt == 100000);
    assert(root->sz == 100000);

    clean(root);
}

void test_insert_delete() {
    auto nums = random_nums(100000);
    vector<int> expected(100000);
    iota(expected.begin(), expected.end(), 0);
    
    TreapNode<int>* root = nullptr;
    for (int x : nums) insert(root, x);
    validate(root, expected);

    for (int x : std::views::iota(0, 100)) erase(root, x);
    expected = vector(expected.begin() + 100, expected.end());
    validate(root, expected);
    assert(root->sz = 99900);

    clean(root);
}

void test_unite() {
    auto nums = random_nums(100000);
    vector<int> expected(100000);
    iota(expected.begin(), expected.end(), 0);

    TreapNode<int> *t1 = nullptr, *t2 = nullptr;
    for (int x : nums) {
        insert(t1, x);
        insert(t2, x);
    }

    auto t3 = unite<int>(t1, t2);
    for (int x : nums) {
        assert(get_cnt(t3, x) == 2);
    }
    validate(t3, expected);

    clean(t3);
}

void test_rank() {
    auto nums = random_nums(100000);
    vector<int> expected(100000);
    iota(expected.begin(), expected.end(), 0);
    
    TreapNode<int>* root = nullptr;
    for (int x : nums) insert(root, x);
    
    for (int i = 0; i < 100; i++) {
        assert(find_by_rank<int>(root, i) == i);
    }
    for (int i = 5000; i < 6000; i++) {
        assert(rank_of_key<int>(root, i) == i);
    }

    clean(root);
}

void test_rank_2() {
    vector<int> values = {
        12, -4, 7, 12, 0, 25, -4, 7, 7, 18,
        3, 12, 30, 0, -10, 18, 5, 25, 12, 3
    };
    vector<int> expected = values;
    sort(expected.begin(), expected.end());

    TreapNode<int>* root = nullptr;
    for (int value : values) insert(root, value);

    assert(root->sz == static_cast<int>(expected.size()));

    for (int rank = 0; rank < static_cast<int>(expected.size()); rank++) {
        assert(find_by_rank<int>(root, rank) == expected[rank]);
    }

    for (int key = -12; key <= 32; key++) {
        int expected_rank = lower_bound(expected.begin(), expected.end(), key)
                          - expected.begin();
        assert(rank_of_key<int>(root, key) == expected_rank);
    }

    clean(root);
}

int main() {
    test_simple_insert();
    test_insert_delete();
    test_unite();
    test_rank();
    test_rank_2();
}
