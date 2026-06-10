#include <bits/stdc++.h>
using namespace std;

// everytime we reach a node, res is correct.

template <typename T, typename S>
struct DefaultRange {
    static S map(T x, int) {
        return static_cast<S>(x);
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

    S self;
    S res;
    
    TreapNode *l = nullptr, *r = nullptr;

    TreapNode(T val)
        : prior(rng()),
          val(std::move(val)),
          self(RangePolicy::map(this->val, 1)),
          res(self) {}
};

template <typename NodeType> 
concept TreapNodeLike =
default_initializable<typename NodeType::Comparator>
&& requires(
    NodeType t, 
    typename NodeType::Value value,
    typename NodeType::Aggregate aggregate,
    typename NodeType::Comparator cmp
) {
    { t.prior } -> same_as<int&>;
    { t.val } -> same_as<typename NodeType::Value&>;
    { t.cnt } -> same_as<int&>;
    { t.sz } -> same_as<int&>;

    { t.self } -> same_as<typename NodeType::Aggregate&>;
    { t.res } -> same_as<typename NodeType::Aggregate&>;
   
    { t.l } -> same_as<NodeType*&>;
    { t.r } -> same_as<NodeType*&>;

    { cmp(value, value) } -> convertible_to<bool>;

    { NodeType::RangePolicy::map(value, 1) } -> same_as<typename NodeType::Aggregate>;

    { NodeType::RangePolicy::reduce(aggregate, aggregate) } -> same_as<typename NodeType::Aggregate>;
};

template <TreapNodeLike Node>
int get_sz(Node *t) {
    return t == nullptr ? 0 : t->sz;
}

// l, r must have the correct agg.
template <TreapNodeLike Node>
void combine(Node *t, Node *l, Node *r) {
    t->l = l;
    t->r = r;
    
    int lsz = (l == nullptr) ? 0 : l->sz;
    int rsz = (r == nullptr) ? 0 : r->sz;
    t->sz = t->cnt + lsz + rsz;

    t->res = t->self;
    if (l != nullptr) {
        t->res = Node::RangePolicy::reduce(l->res, t->res);
    }
    if (r != nullptr) {
        t->res = Node::RangePolicy::reduce(t->res, r->res);
    }
}

template <TreapNodeLike Node>
array<Node*, 2> split(Node *t, const typename Node::Value& val) {
    if (t == nullptr) return {nullptr, nullptr};

    typename Node::Comparator cmp;
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
array<Node*, 3> split_equal(Node *t, const typename Node::Value& val) {
    if (t == nullptr) return {nullptr, nullptr, nullptr};

    typename Node::Comparator cmp;
    if (!cmp(t->val, val) && !cmp(val, t->val)) {
        auto l = t->l, r = t->r;
        combine<Node>(t, nullptr, nullptr);
        return {l, t, r};
    }

    if (cmp(t->val, val)) {
        auto [l, e, r] = split_equal(t->r, val);
        combine(t, t->l, l);
        return {t, e, r};
    } else {
        auto [l, e, r] = split_equal(t->l, val);
        combine(t, r, t->r);
        return {l, e, t};
    }
}

template <TreapNodeLike Node>
Node* merge(Node *l, Node *r) {
    if (l == nullptr) return r;
    if (r == nullptr) return l;

    typename Node::Comparator cmp;
    // if l->val and r->val are the same, then l cannot have any right children and r cannot have any left children.
    if (!cmp(l->val, r->val) && !cmp(r->val, l->val)) {
        l->prior = max(l->prior, r->prior);
        l->cnt += r->cnt;
        l->self = Node::RangePolicy::reduce(l->self, r->self);
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
void insert(Node*& t, const typename Node::Value& val) {
    auto [l, r] = split(t, val);
    t = merge(l, merge(new Node(val), r));
}

template <TreapNodeLike Node>
void erase(Node*& t, const typename Node::Value& val) {
    auto [l, e, r] = split_equal(t, val);
    delete e;
    t = merge(l, r);
}

template<TreapNodeLike Node>
Node* unite(Node *t1, Node *t2) {
    if (t1 == nullptr) return t2;
    if (t2 == nullptr) return t1;

    if (t1->prior < t2->prior) swap(t1, t2);
    
    auto [l, e, r] = split_equal(t2, t1->val);
    if (e != nullptr) {
        t1->cnt += e->cnt;
        t1->self = Node::RangePolicy::reduce(t1->self, e->self);
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
int rank_of_key(Node *t, const typename Node::Value& val) {
    if (t == nullptr) return 0;

    typename Node::Comparator cmp;
    if (cmp(t->val, val)) {
        return get_sz(t->l) + t->cnt + rank_of_key(t->r, val);
    } else if (cmp(val, t->val)) {
        return rank_of_key(t->l, val);
    } else {
        return get_sz(t->l);
    }

}

// 0 <= l_rank <= r_rank < t->sz
// l_rank and r_rank are 0 indexed.
// get range value for [l, r]
template <TreapNodeLike Node>
typename Node::Aggregate range_query(Node *t, int l_rank, int r_rank) {
    assert(t != nullptr);
    assert(0 <= l_rank && l_rank <= r_rank && r_rank < t->sz);

    if (l_rank == 0 && r_rank == t->sz - 1) return t->res;

    int go_left = get_sz(t->l);
    int go_right = go_left + t->cnt;

    if (r_rank < go_left) {
        return range_query(t->l, l_rank, r_rank);
    } else if (l_rank >= go_right) {
        return range_query(t->r, l_rank - go_right, r_rank - go_right);
    }

    int node_count = min(go_right - 1, r_rank) - max(go_left, l_rank) + 1;
    typename Node::Aggregate curr = node_count == t->cnt
                                  ? t->self
                                  : Node::RangePolicy::map(t->val, node_count);

    if (l_rank < go_left) {
        curr = Node::RangePolicy::reduce(
            range_query(t->l, l_rank, go_left - 1),
            curr
        );
    }
    if (r_rank >= go_right) {
        curr = Node::RangePolicy::reduce(
            curr,
            range_query(t->r, 0, r_rank - go_right)
        );
    }
    return curr;
}

template<TreapNodeLike Node>
int get_cnt(Node *t, const typename Node::Value& val) {
    if (t == nullptr) return 0;

    typename Node::Comparator cmp;
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
