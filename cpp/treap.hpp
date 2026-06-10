#include <bits/stdc++.h>
using namespace std;

template <typename T, typename Compare = std::less<T>>
struct TreapNode {
    inline static mt19937 rng{};
    
    T val;
    int cnt = 1;
    int sz = 1;
    int prior;
    TreapNode<T, Compare> *l = nullptr, *r = nullptr;

    TreapNode(T val): val(val), prior(rng()) {}
};

template <typename T, typename Compare = std::less<T>>
int get_sz(TreapNode<T, Compare> *t) {
    return t == nullptr ? 0 : t->sz;
}

template <typename T, typename Compare = std::less<T>>
void combine(TreapNode<T, Compare>* t, TreapNode<T, Compare>* l, TreapNode<T, Compare>* r) {
    t->l = l;
    t->r = r;
    int lsz = (l == nullptr) ? 0 : l->sz;
    int rsz = (r == nullptr) ? 0 : r->sz;
    t->sz = t->cnt + lsz + rsz;
}

template <typename T, typename Compare = std::less<T>>
array<TreapNode<T, Compare>*, 2> split(TreapNode<T> *t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return {nullptr, nullptr};
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
template<typename T, typename Compare = std::less<T>>
array<TreapNode<T>*, 3> split_equal(TreapNode<T> *t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return {nullptr, nullptr, nullptr};

    if (!cmp(t->val, val) && !cmp(val, t->val)) {
        auto l = t->l, r = t->r;
        combine<T, Compare>(t, nullptr, nullptr);
        return {l, t, r};
    }

    if (cmp(t->val, val)) {
        auto [l, e, r] = split_equal(t->r, val, cmp);
        combine<T, Compare>(t, t->l, l);
        return {t, e, r};
    } else {
        auto [l, e, r] = split_equal(t->l, val, cmp);
        combine<T, Compare>(t, r, t->r);
        return {l, e, t};
    }
}

template <typename T, typename Compare = std::less<T>>
TreapNode<T, Compare>* merge(TreapNode<T, Compare> *l, TreapNode<T, Compare> *r, const Compare& cmp = Compare()) {
    if (l == nullptr) return r;
    if (r == nullptr) return l;

    // if l->val and r->val are the same, then l cannot have any right children and r cannot have any left children.
    if (!cmp(l->val, r->val) && !cmp(r->val, l->val)) {
        l->prior = max(l->prior, r->prior);
        l->cnt += r->cnt;
        combine<T, Compare>(l, l->l, r->r);
        delete r;
        return l;
    }

    if (l->prior >= r->prior) {
        combine<T, Compare>(l, l->l, merge(l->r, r));
        return l;
    } else {
        combine<T, Compare>(r, merge(l, r->l), r->r);
        return r;
    }
}


template <typename T, typename Compare = std::less<T>>
void insert(TreapNode<T, Compare>*& t, T val, const Compare& cmp = Compare()) {
    auto [l, r] = split<T, Compare>(t, val);
    t = merge(l, merge(new TreapNode(val), r));
}

template <typename T, typename Compare = std::less<T>>
void erase(TreapNode<T, Compare>*& t, T val, const Compare& cmp = Compare()) {
    auto [l, e, r] = split_equal<T, Compare>(t, val);
    delete e;
    t = merge(l, r);
}

template<typename T, typename Compare = std::less<T>>
TreapNode<T, Compare>* unite(TreapNode<T, Compare> *t1, TreapNode<T, Compare> *t2) {
    if (t1 == nullptr) return t2;
    if (t2 == nullptr) return t1;

    if (t1->prior < t2->prior) swap(t1, t2);
    
    auto [l, e, r] = split_equal(t2, t1->val);
    if (e != nullptr) {
        t1->cnt += e->cnt;
        delete e;
    }
    combine<T, Compare>(t1, unite(t1->l, l), unite(t1->r, r));
    return t1;
}

// Rank is 0 indexed. Requires t != nullptr and 0 <= rank < t->sz.
template<typename T, typename Compare = std::less<T>>
T find_by_rank(TreapNode<T, Compare> *t, int rank) {
    int go_left = get_sz<T, Compare>(t->l);
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
template<typename T, typename Compare = std::less<T>>
int rank_of_key(TreapNode<T, Compare> *t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return 0;
    if (cmp(t->val, val)) {
        return get_sz<T, Compare>(t->l) + t->cnt + rank_of_key(t->r, val, cmp);
    } else if (cmp(val, t->val)) {
        return rank_of_key(t->l, val,cmp);
    } else {
        return get_sz<T, Compare>(t->l);
    }

}

template<typename T, typename Compare = std::less<T>>
int get_cnt(TreapNode<T, Compare>* t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return 0;
    if (!cmp(t->val, val) && !cmp(val, t->val)) return t->cnt;
    return cmp(t->val, val) ? get_cnt(t->r, val) : get_cnt(t->l, val);
}

template<typename T, typename Compare = std::less<T>>
void clean(TreapNode<T, Compare>* t) {
    if (t == nullptr) return;
    clean(t->l);
    clean(t->r);
    delete t;
}