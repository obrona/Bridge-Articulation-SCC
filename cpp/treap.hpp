#include <bits/stdc++.h>
using namespace std;

template <typename T, typename S>
struct DefaultRange {
    static S map(T x, int) {
        return static_cast<S>(x);
    }

    static S update(const S& s, const S&, int) {
        return s;
    }

    static S propogate(const S& s, const S&) {
        return s;
    }

    static S reduce(const S& s, const S&) {
        return s;
    }
};

template <typename T, typename Compare = std::less<T>, typename S = T, typename Range = DefaultRange<T, S>>
struct TreapNode {
    using Value = T;
    using Aggregate = S;
    using Comparator = Compare;
    using RangePolicy = Range;

    inline static mt19937 rng{};

    int prior;
    
    T val;
    int cnt = 1;
    int sz = 1;

    S res;
    S to_prop;
    bool has_prop = false;
    
    TreapNode *l = nullptr, *r = nullptr;

    TreapNode(T val): val(val), prior(rng()) {}
};

template <typename NodeType> 
concept TreapNodeLike = requires(
    NodeType t, 
    typename NodeType::Value value,
    typename NodeType::Aggregate aggregate,
    typename NodeType::Comparator cmp
) {
    { t.prior } -> same_as<int&>;
    { t.val } -> same_as<typename NodeType::Value&>;
    { t.cnt } -> same_as<int&>;
    { t.sz } -> same_as<int&>;

    { t.res } -> same_as<typename NodeType::Aggregate&>;
    { t.to_prop } -> same_as<typename NodeType::Aggregate&>;
    { t.has_prop } -> same_as<bool&>;
   
    { t.l } -> same_as<NodeType*&>;
    { t.r } -> same_as<NodeType*&>;

    { cmp(value, value) } -> convertible_to<bool>;

    { NodeType::RangePolicy::map(value, 1) } -> same_as<typename NodeType::Aggregate>;

    { NodeType::RangePolicy::update(aggregate, aggregate, 1) } -> same_as<typename NodeType::Aggregate>;

    { NodeType::RangePolicy::propogate(aggregate, aggregate) } -> same_as<typename NodeType::Aggregate>;

    { NodeType::RangePolicy::reduce(aggregate, aggregate) } -> same_as<typename NodeType::Aggregate>;



};

template <TreapNodeLike Node>
int get_sz(Node *t) {
    return t == nullptr ? 0 : t->sz;
}

template <TreapNodeLike Node>
void propogate(Node *t, const typename Node::Aggregate& to_prop) {
    if (t == nullptr) return;

    if (t->has_prop) {
        t->to_prop = Node::RangePolicy::propogate(t->to_prop, to_prop);
    } else {
        t->has_prop = true;
        t->to_prop = to_prop;
    }
}

// pull the updates into the direct children
template<TreapNodeLike Node>
void pull(Node *t) {
    if (t == nullptr || !t->has_prop) return;
    
    if (t->l != nullptr) {
        t->l->res = Node::RangePolicy::update(t->l->res, t->to_prop, t->l->sz);
        propogate(t->l, t->to_prop);
    }
    if (t->r != nullptr) {
        t->r->res = Node::RangePolicy::update(t->r->res, t->to_prop, t->r->sz);
        propogate(t->r, t->to_prop);
    }
    t->has_prop = false;
}

// l, r must have the correct agg.
template <TreapNodeLike Node>
void combine(Node *t, Node *l, Node *r) {
    t->l = l;
    t->r = r;
    
    int lsz = (l == nullptr) ? 0 : l->sz;
    int rsz = (r == nullptr) ? 0 : r->sz;
    t->sz = t->cnt + lsz + rsz;

    t->res = Node::RangePolicy::map(t->val, t->cnt);
    if (l != nullptr) {
        t->res = Node::RangePolicy::reduce(l->res, t->res);
    }
    if (r != nullptr) {
        t->res = Node::RangePolicy::reduce(t->res, r->res);
    }
    
}

template <TreapNodeLike Node>
array<Node*, 2> split(Node *t, typename Node::Value val, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    if (t == nullptr) return {nullptr, nullptr};
    
    pull(t);
    
    if (cmp(t->val, val)) {
        auto [l, r] = split(t->r, val);
        combine(t, t->l, l);
        return {t, r};
    } else {
        auto [l, r] = split(t->l, val);
        combine(t, r, t->r);
        return {l, t};
    }
}

// return less than, equal, more than.
template<TreapNodeLike Node>
array<Node*, 3> split_equal(Node *t, const typename Node::Value& val, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    if (t == nullptr) return {nullptr, nullptr, nullptr};

    pull(t);

    if (!cmp(t->val, val) && !cmp(val, t->val)) {
        auto l = t->l, r = t->r;
        combine(t, nullptr, nullptr);
        return {l, t, r};
    }

    if (cmp(t->val, val)) {
        auto [l, e, r] = split_equal(t->r, val, cmp);
        combine(t, t->l, l);
        return {t, e, r};
    } else {
        auto [l, e, r] = split_equal(t->l, val, cmp);
        combine(t, r, t->r);
        return {l, e, t};
    }
}

template <TreapNodeLike Node>
Node* merge(Node *l, Node *r, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    if (l == nullptr) return r;
    if (r == nullptr) return l;

    pull(l);
    pull(r);

    // if l->val and r->val are the same, then l cannot have any right children and r cannot have any left children.
    if (!cmp(l->val, r->val) && !cmp(r->val, l->val)) {
        l->prior = max(l->prior, r->prior);
        l->cnt += r->cnt;
        combine(l, l->l, r->r);
        delete r;
        return l;
    }

    if (l->prior >= r->prior) {
        combine(l, l->l, merge(l->r, r));
        return l;
    } else {
        combine(r, merge(l, r->l), r->r);
        return r;
    }
}


template <TreapNodeLike Node>
void insert(Node*& t, const typename Node::Value& val, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    pull(t);
    
    auto [l, r] = split<Node>(t, val);
    t = merge(l, merge(new Node(val), r));
}

template <TreapNodeLike Node>
void erase(Node*& t, const typename Node::Value& val, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    pull(t);
    
    auto [l, e, r] = split_equal(t, val);
    delete e;
    t = merge(l, r);
}

template<TreapNodeLike Node>
Node* unite(Node *t1, Node *t2) {
    pull(t1);
    pull(t2);

    if (t1 == nullptr) return t2;
    if (t2 == nullptr) return t1;

    if (t1->prior < t2->prior) swap(t1, t2);
    
    auto [l, e, r] = split_equal(t2, t1->val);
    if (e != nullptr) {
        t1->cnt += e->cnt;
        delete e;
    }
    combine<Node>(t1, unite(t1->l, l), unite(t1->r, r));
    return t1;
}

// Rank is 0 indexed. Requires t != nullptr and 0 <= rank < t->sz.
template<TreapNodeLike Node>
typename Node::Value find_by_rank(Node *t, int rank) {
    int go_left = get_sz(t->l);
    int go_right = go_left + t->cnt;

    if (rank < go_left) {
        return find_by_rank(t->l, rank);
    } else if (go_left <= rank && rank < go_right) {
        return t->val;
    } else {
        return find_by_rank(t->r, rank - go_right);
    }
} 

// returns how many elements are less than val.
// follow GNU PBDS order_of_key.
template<TreapNodeLike Node>
int rank_of_key(Node *t, const typename Node::Value& val, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    if (t == nullptr) return 0;

    if (cmp(t->val, val)) {
        return get_sz(t->l) + t->cnt + rank_of_key(t->r, val, cmp);
    } else if (cmp(val, t->val)) {
        return rank_of_key(t->l, val,cmp);
    } else {
        return get_sz(t->l);
    }

}

template<TreapNodeLike Node>
int get_cnt(Node *t, const typename Node::Value& val, const typename Node::Comparator& cmp = typename Node::Comparator()) {
    if (t == nullptr) return 0;

    if (!cmp(t->val, val) && !cmp(val, t->val)) return t->cnt;
    return cmp(t->val, val) ? get_cnt(t->r, val) : get_cnt(t->l, val);
}

template<TreapNodeLike Node>
void clean(Node *t) {
    if (t == nullptr) return;

    clean(t->l);
    clean(t->r);
    delete t;
}