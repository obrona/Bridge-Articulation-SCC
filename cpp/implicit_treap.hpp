#include <bits/stdc++.h>
using namespace std;

// implicit treap whose keys are the 0-indexed positions of an array.
// the array on object of type T.
// implicit treap lets us 
// 1. insert an element at any position
// 2. erase an element at any position
// 3. range update
// 4. range query

// invariant for split and merge.
// the root node aggregate is absolutely correct, all lazy updates have been applied.



template <typename T>
struct DefaultRangePolicy {
    // the update function is the same for self and prop.
    // e.g range min query range add update.
    static T update(T t, T) {
        return t;
    }

    static T update_agg(T t, T, int) {
        return t;
    }

    static T reduce(T t, T) {
        return t;
    }
};

template <typename T, typename RangePolicy = DefaultRangePolicy<T>>
struct TreapNode {
    using Value = T;
    using Range = RangePolicy;

    static inline mt19937 rng{};

    T val;
    int prior;
    int sz = 1;

    T agg;
    optional<T> prop; // value to propogate downwards.

    TreapNode *l = nullptr, *r = nullptr;

    TreapNode(T val): val(val), prior(rng()), agg(val) {};
};

template <typename NodeType>
concept TreapNodeLike = requires(NodeType t, typename NodeType::Value val, typename NodeType::Value& v) {
    { t.val } -> same_as<typename NodeType::Value&>;
    { t.prior } -> same_as<int&>;
    { t.sz } -> same_as<int&>;
    { t.agg } -> same_as<typename NodeType::Value&>;
    { t.prop } -> same_as<optional<typename NodeType::Value>&>;

    { NodeType::Range::update(val, val) } -> same_as<typename NodeType::Value>;
    { NodeType::Range::update_agg(val, val, 1) } -> same_as<typename NodeType::Value>;
    { NodeType::Range::reduce(val, val) } -> same_as<typename NodeType::Value>;
};

template <TreapNodeLike Node>
int get_sz(Node *t) { return t == nullptr ? 0 : t->sz; };

template<TreapNodeLike Node>
void update(Node *t, const typename Node::Value& p) {
    if (t == nullptr) return;

    t->val = Node::Range::update(t->val, p);
    t->agg = Node::Range::update_agg(t->agg, p, t->sz);
    t->prop = (t->prop) ? Node::Range::update(t->prop.value(), p) : p;
}

template <TreapNodeLike Node>
void push(Node *t) {
    if (t == nullptr || !t->prop) return;

    update(t->l, t->prop.value());
    update(t->r, t->prop.value());
    t->prop.reset();
}

template <TreapNodeLike Node>
void combine(Node *t, Node *l, Node *r) {
    t->sz = 1 + get_sz(l) + get_sz(r);
    t->agg = t->val;
    if (l) {
        t->agg = Node::Range::reduce(t->agg, l->agg);
    } 
    if (r) {
        t->agg = Node::Range::reduce(t->agg, r->agg);
    }
    t->l = l;
    t->r = r;
}

// return  < idx, >= idx
template <TreapNodeLike Node>
array<Node*, 2> split(Node *t, int idx) {
    if (t == nullptr) return {nullptr, nullptr};

    push(t);
    int left_sz = get_sz(t->l);
    if (idx == left_sz) {
        auto l = t->l;
        combine(t, nullptr, t->r);
        return {l, t};
    } else if (idx < left_sz) {
        auto [l, r] = split(t->l, idx);
        combine(t, r, t->r);
        return {l, t};
    } else {
        auto [l, r] = split(t->r, idx - left_sz - 1);
        combine(t, t->l, l);
        return {t, r};
    }
}

template <TreapNodeLike Node>
Node* merge(Node *l, Node *r) {
    if (l == nullptr) return r;
    if (r == nullptr) return l;

    if (l->prior >= r->prior) {
        push(l);
        combine(l, l->l, merge(l->r, r));
        return l;
    } else {
        push(r);
        combine(r, merge(l, r->l), r->r);
        return r;
    }
}

template <TreapNodeLike Node>
void insert(Node *&t, int idx, typename Node::Value val) {
    push(t);
    auto [l, r] = split(t, idx);
    t = merge(l, merge(new Node(val), r));
}

template <TreapNodeLike Node>
void erase(Node *&t, int idx) {
    push(t);
    auto [l, r] = split(t, idx);
    auto [l2, r2] = split(r, 1);
    delete l2;
    t = merge(l, r2);
}

// update the range [s, e]
template <TreapNodeLike Node>
void range_update(Node *t, int s, int e, typename Node::Value val) {
    push(t);

    if (s == 0 && e == get_sz(t) - 1) {
        update(t, val);
        return;
    }

    int my_key = get_sz(t->l);
    if (e < my_key) {
        range_update(t->l, s, e, val);
        combine(t, t->l, t->r);
    } else if (s > my_key) {
        range_update(t->r, s - my_key - 1, e - my_key - 1, val);
        combine(t, t->l, t->r);   
    } else {
        if (s < my_key) range_update(t->l, s, my_key - 1, val);
        if (e > my_key) range_update(t->r, 0, e - my_key - 1, val);
        t->val = Node::Range::update(t->val, val);
        combine(t, t->l, t->r);
    }
} 

template <TreapNodeLike Node>
typename Node::Value range_query(Node *t, int s, int e) {
    push(t);

    if (s == 0 && e == get_sz(t) - 1) {
        return t->agg;
    }

    int my_key = get_sz(t->l);
    if (e < my_key) {
        return range_query(t->l, s, e);
    } else if (s > my_key) {
        return range_query(t->r, s - my_key - 1, e - my_key - 1);
    } else {
        typename Node::Value res = t->val;
        if (s < my_key) res = Node::Range::reduce(range_query(t->l, s, my_key - 1), res);
        if (e > my_key) res = Node::Range::reduce(res, range_query(t->r, 0, e - my_key - 1));
        return res;
    }
}

template <TreapNodeLike Node>
void clean(Node *root) {
    if (root == nullptr) return;
    clean(root->l);
    clean(root->r);
    delete root;
}
