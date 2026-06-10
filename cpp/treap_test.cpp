#include <bits/stdc++.h>
#include <ranges>
#include "treap.hpp"

template <TreapNodeLike Node>
int validate_node(Node* t, const typename Node::Comparator& cmp) {
    if (t == nullptr) return 0;

    if (t->l != nullptr) assert(t->l->prior <= t->prior);
    if (t->r != nullptr) assert(t->r->prior <= t->prior);

    int left_size = validate_node(t->l, cmp);
    int right_size = validate_node(t->r, cmp);

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
    const vector<typename Node::Value>& expected,
    const typename Node::Comparator& cmp = typename Node::Comparator()
) {
    assert(validate_node(t, cmp) == static_cast<int>(expected.size()));
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

struct StatefulCompare {
    bool descending = false;

    bool operator()(int a, int b) const {
        return descending ? a > b : a < b;
    }
};

void test_stateful_comparator() {
    StatefulCompare cmp{.descending = true};
    vector<int> expected = {9, 7, 5, 3, 1};
    TreapNode<int, StatefulCompare>* root = nullptr;

    for (int value : {1, 7, 3, 9, 5}) insert(root, value, cmp);
    validate(root, expected, cmp);
    assert(find_by_rank(root, 0) == 9);
    assert(rank_of_key(root, 5, cmp) == 2);
    assert(get_cnt(root, 7, cmp) == 1);

    erase(root, 7, cmp);
    expected.erase(expected.begin() + 1);
    validate(root, expected, cmp);
    clean(root);
}

struct SumRange {
    static long long map(int value, int count) {
        return 1LL * value * count;
    }

    static long long update(const long long& sum, const long long& delta, int size) {
        return sum + delta * size;
    }

    static long long propogate(const long long& current, const long long& next) {
        return current + next;
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

void test_lazy_aggregate_update() {
    using SumNode = TreapNode<int, less<int>, long long, SumRange>;
    SumNode* root = nullptr;

    for (int value : {1, 3, 5}) insert(root, value);
    propogate(root, 10);
    propogate(root, 2);
    assert(root->res == 45);

    auto [left, right] = split(root, 5);
    assert(left->res == 28);
    assert(right->res == 17);

    root = merge(left, right);
    insert(root, 7);
    assert(root->res == 52);
    clean(root);
}

void test_range_query() {
    using SumNode = TreapNode<int, less<int>, long long, SumRange>;
    SumNode* root = nullptr;

    for (int value : {5, 1, 5, 3, 1, 7}) insert(root, value);
    assert(range_query(root, 0, 5) == 22);
    assert(range_query(root, 1, 4) == 14);
    assert(range_query(root, 0, 1) == 2);
    assert(range_query(root, 2, 3) == 8);
    assert(range_query(root, 3, 5) == 17);

    propogate(root, 10);
    assert(range_query(root, 2, 5) == 60);
    assert(range_query(root, 2, 5) == 60);
    clean(root);
}

struct ConcatenateRange {
    static string map(int value, int count) {
        string result;
        while (count-- > 0) result += to_string(value);
        return result;
    }

    static string update(const string& value, const string&, int) {
        return value;
    }

    static string propogate(const string& current, const string&) {
        return current;
    }

    static string reduce(const string& left, const string& right) {
        return left + right;
    }
};

void test_range_query_order() {
    using StringNode = TreapNode<int, less<int>, string, ConcatenateRange>;
    StringNode* root = nullptr;

    for (int value : {4, 1, 3, 2}) insert(root, value);
    assert(range_query(root, 0, 3) == "1234");
    assert(range_query(root, 1, 3) == "234");
    assert(range_query(root, 0, 2) == "123");
    clean(root);
}

int main() {
    test_simple_insert();
    test_insert_delete();
    test_unite();
    test_rank();
    test_stateful_comparator();
    test_aggregate();
    test_lazy_aggregate_update();
    test_range_query();
    test_range_query_order();
}
