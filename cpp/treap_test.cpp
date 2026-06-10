#include <bits/stdc++.h>
#include <ranges>
#include "treap.hpp"

template <TreapNodeLike Node>
int validate_node(Node* t) {
    if (t == nullptr) return 0;

    if (t->l != nullptr) assert(t->l->prior <= t->prior);
    if (t->r != nullptr) assert(t->r->prior <= t->prior);

    int left_size = validate_node(t->l);
    int right_size = validate_node(t->r);

    typename Node::Comparator cmp;
    if (t->l != nullptr) assert(cmp(t->l->val, t->val));
    if (t->r != nullptr) assert(cmp(t->val, t->r->val));
    assert(t->sz == left_size + t->cnt + right_size);
    return t->sz;
}

template <TreapNodeLike Node>
vector<typename Node::Value> values_of(Node* t) {
    if (t == nullptr) return {};

    vector<typename Node::Value> values;
    function<void(Node*)> dfs = [&](Node* node) {
        if (node == nullptr) return;
        dfs(node->l);
        values.insert(values.end(), node->cnt, node->val);
        dfs(node->r);
    };
    dfs(t);
    return values;
}

template <TreapNodeLike Node>
void validate(
    Node* t,
    const vector<typename Node::Value>& expected
) {
    assert(validate_node(t) == static_cast<int>(expected.size()));
    assert(values_of(t) == expected);
}

vector<int> random_nums(int len) {
    vector<int> out(len);
    iota(out.begin(), out.end(), 0);
    shuffle(out.begin(), out.end(), mt19937{});
    return out;
}

void test_simple_insert() {
    TreapNode<int>* root = nullptr;
    for (int i = 0; i < 100000; i++) insert(root, 0);

    assert(root->cnt == 100000);
    assert(root->sz == 100000);
    assert(get_cnt(root, 0) == 100000);
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
    expected.erase(expected.begin(), expected.begin() + 100);
    validate(root, expected);
    assert(root->sz == 99900);
    clean(root);
}

void test_unite() {
    auto nums = random_nums(100000);
    vector<int> expected;
    expected.reserve(200000);
    for (int x = 0; x < 100000; x++) {
        expected.push_back(x);
        expected.push_back(x);
    }

    TreapNode<int> *t1 = nullptr, *t2 = nullptr;
    for (int x : nums) {
        insert(t1, x);
        insert(t2, x);
    }

    auto* t3 = unite(t1, t2);
    for (int x : nums) assert(get_cnt(t3, x) == 2);
    validate(t3, expected);
    clean(t3);
}

void test_rank() {
    vector<int> values = {
        12, -4, 7, 12, 0, 25, -4, 7, 7, 18,
        3, 12, 30, 0, -10, 18, 5, 25, 12, 3
    };
    vector<int> expected = values;
    sort(expected.begin(), expected.end());

    TreapNode<int>* root = nullptr;
    for (int value : values) insert(root, value);
    validate(root, expected);

    for (int rank = 0; rank < static_cast<int>(expected.size()); rank++) {
        assert(find_by_rank(root, rank) == expected[rank]);
    }
    for (int key = -12; key <= 32; key++) {
        int expected_rank = lower_bound(expected.begin(), expected.end(), key)
                          - expected.begin();
        assert(rank_of_key(root, key) == expected_rank);
    }
    clean(root);
}

struct DescendingCompare {
    bool operator()(int a, int b) const {
        return a > b;
    }
};

void test_stateless_comparator() {
    vector<int> expected = {9, 7, 5, 3, 1};
    TreapNode<int, DescendingCompare>* root = nullptr;

    for (int value : {1, 7, 3, 9, 5}) insert(root, value);
    validate(root, expected);
    assert(find_by_rank(root, 0) == 9);
    assert(rank_of_key(root, 5) == 2);
    assert(get_cnt(root, 7) == 1);

    erase(root, 7);
    expected.erase(expected.begin() + 1);
    validate(root, expected);
    clean(root);
}

struct SumRange {
    static long long map(int value, int count) {
        return 1LL * value * count;
    }

    static long long reduce(const long long& left, const long long& right) {
        return left + right;
    }
};

void test_aggregate() {
    using SumNode = TreapNode<int, less<int>, long long, SumRange>;
    SumNode* root = nullptr;

    for (int value : {5, 1, 5, 3, 1}) insert(root, value);
    assert(root->res == 15);

    erase(root, 3);
    assert(root->res == 12);

    SumNode* other = nullptr;
    for (int value : {2, 5}) insert(other, value);
    root = unite(root, other);
    assert(root->res == 19);
    assert(root->sz == 6);
    clean(root);
}

void test_range_query() {
    using SumNode = TreapNode<int, less<int>, long long, SumRange>;
    SumNode* root = nullptr;

    insert(root, 5);
    insert(root, 1);
    assert(range_query(root, 0, 1) == 6);

    insert(root, 5);
    insert(root, 3);
    assert(range_query(root, 1, 2) == 8);

    erase(root, 5);
    assert(range_query(root, 0, 1) == 4);

    insert(root, 7);
    assert(range_query(root, 1, 2) == 10);

    insert(root, 1);
    assert(range_query(root, 0, 3) == 12);
    assert(range_query(root, 0, 1) == 2);

    erase(root, 3);
    assert(range_query(root, 1, 2) == 8);

    insert(root, 5);
    assert(range_query(root, 1, 3) == 13);
    clean(root);
}

struct ConcatenateRange {
    static string map(int value, int count) {
        string result;
        while (count-- > 0) result += to_string(value);
        return result;
    }

    static string reduce(const string& left, const string& right) {
        return left + right;
    }
};

void test_range_query_order() {
    using StringNode = TreapNode<int, less<int>, string, ConcatenateRange>;
    StringNode* root = nullptr;

    insert(root, 4);
    insert(root, 1);
    assert(range_query(root, 0, 1) == "14");

    insert(root, 3);
    insert(root, 2);
    assert(range_query(root, 0, 3) == "1234");

    erase(root, 3);
    assert(range_query(root, 1, 2) == "24");

    insert(root, 3);
    assert(range_query(root, 1, 3) == "234");

    erase(root, 1);
    assert(range_query(root, 0, 2) == "234");
    clean(root);
}

int main() {
    test_simple_insert();
    test_insert_delete();
    test_unite();
    test_rank();
    test_stateless_comparator();
    test_aggregate();
    test_range_query();
    test_range_query_order();
}
