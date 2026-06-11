#include <bits/stdc++.h>
using namespace std;

// implicit treap whose keys are the 0-indexed positions of an array.
// the array on object of type T.
// implicit treap lets us 
// 1. insert an element at any position
// 2. cut a subarray and paste it elsewhere
// 3. range update
// 4. range query

mt19937 rng{};

template <typename T>
struct DefaultRangePolicy {
    // the update function is the same for self and prop.
    // e.g range min query range add update.
    static T update(T t, T) {
        return t;
    }

    static T update_agg(T t, int sz) {
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

    T val;
    int prior;
    int sz = 1;

    T agg;
    optional<T> prop; // value to propogate downwards.

    TreapNode(T val): val(val), prior(rng()), agg(val) {};
};

template <typename NodeType>
concept TreapNodeLike = requires(NodeType t, typename NodeType::Value val, typename NodeType::Value& v) {
    { t.val } -> same_as<typename NodeType::Value&>;
    { t.prior } -> same_as<int&>;
    { t.sz } -> same_as<int&>;
    { t.agg } -> same_as<typename NodeType::Value&>;
    { t.prop } -> same_as<optional<typename NodeType::Value>&>;

    { NodeType::RangePolicy::update(val, val) } -> same_as<typename NodeType::Value>;
    { NodeType::RangePolicy::update_agg(val, val, 1) } -> same_as<typename NodeType::Value>;
    { NodeType::RangePolicy::update(val, val) } -> same_as<typename NodeType::Value>;
};

template <TreapNodeLike Node>
int get_sz(Node *t) { return t == nullptr ? 0 : t->sz; };

template<TreapNodeLike Node>
void update(Node *t, const typename Node::Value& p) {
    if (t == nullptr) return;

    t->val = Node::Range::update(t->val, p);
    t->agg = Node::Range::update_agg(t->agg, p, t-sz);
    t->prop = (t->prop) ? Node::Range::update(t->prop.value(), p) : p;
}

template <TreapNodeLike Node>
void push(Node *t) {
    if (t == nullptr || !t.prop) return;

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


