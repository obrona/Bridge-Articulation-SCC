#include <bits/stdc++.h>
#include "implicit_treap.hpp"

using namespace std;

struct RangeAddMin {
    static long long update(long long value, long long delta) {
        return value + delta;
    }

    static long long update_agg(long long aggregate, long long delta, int) {
        return aggregate + delta;
    }

    static long long reduce(long long left, long long right) {
        return min(left, right);
    }
};

struct RangeAddMax {
    static long long update(long long value, long long delta) {
        return value + delta;
    }

    static long long update_agg(long long aggregate, long long delta, int) {
        return aggregate + delta;
    }

    static long long reduce(long long left, long long right) {
        return max(left, right);
    }
};

template <TreapNodeLike Node>
Node* build(const vector<long long>& values) {
    Node *root = nullptr;
    for (long long value : values) {
        root = merge(root, new Node(value));
    }
    return root;
}

template <typename Policy>
long long expected_query(
    const vector<long long>& values,
    int left,
    int right
) {
    long long result = values[left];
    for (int i = left + 1; i <= right; ++i) {
        result = Policy::reduce(result, values[i]);
    }
    return result;
}

template <typename Policy>
void test_fixed_cases() {
    using Node = TreapNode<long long, Policy>;

    vector<long long> values = {5, -2, 8, 1, 7, -4, 3};
    Node *root = build<Node>(values);

    auto check = [&](int left, int right) {
        assert(range_query(root, left, right) == expected_query<Policy>(values, left, right));
    };

    check(0, 6);
    check(0, 0);
    check(6, 6);
    check(2, 5);

    range_update(root, 1, 4, 6);
    for (int i = 1; i <= 4; ++i) values[i] += 6;
    check(0, 6);
    check(1, 4);
    check(0, 2);

    range_update(root, 0, 6, -3);
    for (long long& value : values) value -= 3;
    check(0, 6);
    check(3, 6);

    range_update(root, 3, 3, 100);
    values[3] += 100;
    check(3, 3);
    check(2, 4);

    range_update(root, 2, 6, -20);
    for (int i = 2; i <= 6; ++i) values[i] -= 20;
    check(0, 6);
    check(2, 6);

    clean(root);
}

template <typename Policy>
void test_randomized() {
    using Node = TreapNode<long long, Policy>;

    random_device rd;
    mt19937 generator{rd()};
    uniform_int_distribution<long long> initial_value(-1000, 1000);
    uniform_int_distribution<long long> delta_value(-100, 100);

    constexpr int size = 100000;
    constexpr int operations = 200;

    vector<long long> values(size);
    ranges::generate(values, [&] { return initial_value(generator); });
    Node *root = build<Node>(values);

    for (int op = 0; op < operations; ++op) {
        int left = static_cast<int>(generator() % size);
        int right = static_cast<int>(generator() % size);
        if (left > right) swap(left, right);

        if (generator() % 2 == 0) {
            long long delta = delta_value(generator);
            range_update(root, left, right, delta);
            for (int i = left; i <= right; ++i) values[i] += delta;
        } else {
            assert(range_query(root, left, right) == expected_query<Policy>(values, left, right));
        }
    }

    assert(
        range_query(root, 0, size - 1)
        == expected_query<Policy>(values, 0, size - 1)
    );
    clean(root);
}

template <typename Policy>
void test_interleaved_operations() {
    using Node = TreapNode<long long, Policy>;

    vector<long long> values = {4, -1, 7, 3};
    Node *root = build<Node>(values);

    auto check_all_ranges = [&] {
        assert(get_sz(root) == values.size());
        for (int left = 0; left < values.size(); ++left) {
            for (int right = left; right < values.size(); ++right) {
                assert(range_query(root, left, right) == expected_query<Policy>(values, left, right));
            }
        }
    };

    check_all_ranges();

    range_update(root, 1, 3, 5);
    for (int i = 1; i <= 3; ++i) values[i] += 5;
    check_all_ranges();

    insert(root, 2, -6);
    values.insert(values.begin() + 2, -6);
    check_all_ranges();

    range_update(root, 0, 2, 3);
    for (int i = 0; i <= 2; ++i) values[i] += 3;
    check_all_ranges();

    erase(root, 1);
    values.erase(values.begin() + 1);
    check_all_ranges();

    insert(root, static_cast<int>(values.size()), -10);
    values.push_back(-10);
    check_all_ranges();

    range_update(root, 2, 4, -4);
    for (int i = 2; i <= 4; ++i) values[i] -= 4;
    check_all_ranges();

    erase(root, 0);
    values.erase(values.begin());
    check_all_ranges();

    range_update(root, 0, static_cast<int>(values.size()) - 1, 2);
    for (long long& value : values) value += 2;
    check_all_ranges();

    insert(root, 0, 20);
    values.insert(values.begin(), 20);
    check_all_ranges();

    clean(root);
}

void test_large_size() {
    using Node = TreapNode<long long, RangeAddMax>;

    vector<long long> values(100000, 0);
    Node *root = build<Node>(values);
    
    for (int op = 0; op < 100000; op += 10) {
        range_update(root, op, min(op + 9, 99999), 1);
    }

    assert(range_query(root, 0, 0) == 1);

    for (int i = 10; i < 100000; i++) {
        erase(root, 10);
    }
    assert(get_sz(root) == 10);

    for (int i = 0; i < 3; i++) {
        insert(root, 1, i);
    }
    assert(get_sz(root) == 13);
    assert(range_query(root, 1, 3) == 2);

    clean(root);
}

int main() {
    test_fixed_cases<RangeAddMin>();
    test_randomized<RangeAddMin>();
    test_interleaved_operations<RangeAddMin>();

    test_fixed_cases<RangeAddMax>();
    test_randomized<RangeAddMax>();
    test_interleaved_operations<RangeAddMax>();

    test_large_size();
}
